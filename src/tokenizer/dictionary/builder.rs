use crate::tokenizer::dictionary::model;

use crate::error::Error;
use log::{debug, info};
use postcard::to_stdvec;
use regex::Regex;
use std::{
    borrow::Borrow,
    collections::HashMap,
    fs::{self, File},
    io::{BufRead, BufReader, BufWriter, Write},
    path::{Path, PathBuf},
};
use unicode_normalization::UnicodeNormalization;

use fst::MapBuilder;
use model::*;

use super::TokenDictionary;

pub struct DictionaryBuilder {
    // Whether to normalize token surface or not in NFKC form.
    normalize: bool,
}

impl DictionaryBuilder {
    pub fn new(normalize: Option<bool>) -> DictionaryBuilder {
        let normalize_ = normalize.unwrap_or(false);

        DictionaryBuilder {
            normalize: normalize_,
        }
    }

    pub fn build(&self, input_path: &str, output_path: &str) -> Result<(), Error> {
        info!("Building token info dictionary...");
        _ = match self.build_token_infos(input_path, output_path) {
            Ok(_) => (),
            Err(e) => return Err(e),
        };

        info!("Building unknown dictionary...");
        _ = match self.build_unk_dictionary(input_path, output_path) {
            Ok(_) => (),
            Err(e) => return Err(e),
        };

        info!("Building connection cost matrix...");
        _ = match self.build_connection_cost(input_path, output_path) {
            Ok(_) => (),
            Err(e) => return Err(e),
        };

        Ok(())
    }

    // This method builds token informations from the input file and save FST and token dictionary.
    fn build_token_infos(&self, input_path: &str, output_path: &str) -> Result<(), Error> {
        // 1. List all csv files.
        let files = match fs::read_dir(input_path) {
            Ok(files) => files,
            Err(_) => return Err("Failed to read input directory".into()),
        };

        let files: Vec<PathBuf> = files
            .map(|r| r.map(|d| d.path()))
            .filter(|r| {
                r.is_ok()
                    && r.as_deref().unwrap().is_file()
                    && r.as_deref().unwrap().extension().unwrap_or_default() == "csv"
            })
            .map(|r| r.unwrap())
            .collect();

        if files.len() == 0 {
            return Err("No csv files found".into());
        }

        info!("Found {} csv files", files.len());

        // 2. Read all csvs.
        let out_fst_file_path = Path::new(output_path).join(FST_FILENAME);
        let out_fst_file = match File::create(out_fst_file_path) {
            Ok(f) => f,
            Err(_) => return Err("Failed to create output fst file".into()),
        };
        let fst_writer = BufWriter::new(out_fst_file);
        let mut fst_builder = match MapBuilder::new(fst_writer) {
            Ok(b) => b,
            Err(_) => return Err("Failed to create fst builder".into()),
        };

        let mut records: Vec<MeCabTokenCSVRecord> = Vec::new();

        for file in files {
            let path = file.as_path().to_str().unwrap();
            debug!("Reading `{}`", path);

            let mut contents = match fs::read_to_string(path) {
                Ok(contents) => contents,
                Err(_) => return Err(format!("Failed to read csv file `{}`", path).into()),
            };
            if self.normalize {
                contents = contents.nfkc().collect()
            }
            let mut csv_reader = csv::ReaderBuilder::new()
                .has_headers(false)
                .from_reader(contents.as_bytes());
            let mut records_for_file: Vec<MeCabTokenCSVRecord> = csv_reader
                .deserialize::<MeCabTokenCSVRecord>()
                .filter(|r| r.is_ok())
                .map(|r| r.unwrap())
                .collect();
            records.append(&mut records_for_file);
        }

        info!("Read {} records", records.len());

        // 3. Build FST from read rows.
        //
        // We build FST for search and TokenDictionary struct for metadata.
        info!("Building FST ...");

        let mut morphemes_by_surface: Vec<Vec<Morpheme>> = Vec::new();
        records.sort_by(|a, b| a.surface.cmp(&b.surface));

        let mut fst_index: usize = 0;
        for (index, record) in records.iter().enumerate() {
            if index == 0 || record.surface != records[index - 1].surface {
                match fst_builder.insert(record.surface.clone(), fst_index.try_into().unwrap()) {
                    Ok(_) => (),
                    Err(_) => return Err("Failed to insert into fst".into()),
                };

                morphemes_by_surface.push(Vec::new());
                fst_index += 1;
            }

            let morpheme = match Morpheme::from_csv_record(record) {
                Ok(morpheme) => morpheme,
                Err(e) => {
                    return Err(format!(
                        "cannot build morpheme `{:?}`, original error: {}",
                        record,
                        e.description()
                    )
                    .into())
                }
            };

            morphemes_by_surface[fst_index - 1].push(morpheme)
        }

        // 4. Finishing the process.
        info!("Saving Token Dictionary ...");
        match fst_builder.finish() {
            Ok(_) => (),
            Err(_) => return Err("Failed to build FST".into()),
        };

        let token_dictionary = TokenDictionary {
            morphemes: morphemes_by_surface,
        };
        let bin_token_dictionary: Vec<u8> = match to_stdvec(&token_dictionary) {
            Ok(bytes) => bytes,
            Err(_) => return Err("Failed to serialize token dictionary".into()),
        };

        let mut file = match File::create(Path::new(output_path).join(TOKEN_FILENAME)) {
            Ok(f) => f,
            Err(_) => return Err("Failed to create output file".into()),
        };

        match file.write_all(bin_token_dictionary.as_slice()) {
            Ok(_) => Ok(()),
            Err(_) => return Err("Failed to write token dictionary".into()),
        }
    }

    // This method will build unknown token dictionary using unk.def and char.def files.
    fn build_unk_dictionary(&self, input_path: &str, output_path: &str) -> Result<(), Error> {
        // 1. Build unknown token infos.
        info!("Building unknown token infos ...");
        let unk_file = match File::open(Path::new(input_path).join(MECAB_UNK_FILENAME)) {
            Ok(f) => f,
            Err(_) => return Err("Failed to open unk.def file".into()),
        };
        let unk_reader = BufReader::new(unk_file);

        let mut unk_csv_reader = csv::ReaderBuilder::new()
            .has_headers(false)
            .from_reader(unk_reader);
        let mut unk_class_morpheme_map: HashMap<CharacterClass, Morpheme> = HashMap::new();
        unk_class_morpheme_map.insert(
            CharacterClass::NGRAM,
            Morpheme {
                left_id: 1798,
                right_id: 3559,
                word_cost: 3677,
                expressions: Vec::new(),
                pos_tags: vec![POSTag::SY],
                pos_type: POSType::MORPHEME,
            },
        );

        for record in unk_csv_reader.deserialize::<MeCabUnkCSVRecord>() {
            let record = match record {
                Ok(record) => record,
                Err(_) => return Err("cannot read record while reading unk.def".into()),
            };

            let category = match CharacterClass::from_name(record.category.as_str()) {
                Some(ptags) => ptags,
                None => {
                    return Err(format!(
                        "cannot find character class `{}` while reading unk.def",
                        record.category.as_str()
                    )
                    .into())
                }
            };

            let pos_tag = match POSTag::from_name(record.pos_tags.as_str()) {
                Some(ptags) => ptags,
                None => {
                    return Err(format!(
                        "cannot find pos tag `{}` while reading unk.def",
                        record.pos_tags.as_str()
                    )
                    .into())
                }
            };
            unk_class_morpheme_map.insert(
                category,
                Morpheme {
                    left_id: record.left_id,
                    right_id: record.right_id,
                    word_cost: record.word_cost,
                    expressions: Vec::new(),
                    pos_tags: vec![pos_tag],
                    pos_type: POSType::MORPHEME,
                },
            );
        }

        // 2. Build character class infos.
        info!("Building character class infos ...");
        let mut invoke_map: HashMap<CharacterClass, CategoryDefinition> = HashMap::new();
        let mut code_to_category_map: HashMap<i32, CharacterClass> = HashMap::new();

        let char_file = match File::open(Path::new(input_path).join(MECAB_CHAR_FILENAME)) {
            Ok(f) => f,
            Err(_) => return Err("Failed to open char.def file".into()),
        };
        let char_reader = BufReader::new(char_file);

        let pat_for_remove_space = Regex::new(r"\s+").unwrap();
        let pat_for_remove_comment = Regex::new(r"\s*#.*").unwrap();
        for line in char_reader.lines() {
            let line = match line {
                Ok(line) => line,
                Err(_) => return Err("Failed to read char.def file".into()),
            };

            let line = pat_for_remove_space.replace_all(line.as_str(), " ");
            let line = pat_for_remove_comment.replace_all(line.borrow(), "");
            let line = line.trim();

            if line.starts_with("#") || line.len() == 0 {
                continue;
            }

            if line.starts_with("0x") {
                // char code definition
                let splits = line.split(" ").collect::<Vec<&str>>();
                if splits.len() < 2 {
                    return Err(format!(
                        "malformed char.def file, line: `{}`, splits: `{:?}`",
                        line, splits
                    )
                    .into());
                }
                let char_cls = match CharacterClass::from_name(splits[1]) {
                    Some(cls) => cls,
                    None => {
                        return Err(format!(
                            "cannot find character class `{}` while reading char.def",
                            splits[1]
                        )
                        .into())
                    }
                };

                if splits[0].contains("..") {
                    // range
                    let range_splits = splits[0]
                        .split("..")
                        .map(|x| x.trim_start_matches("0x"))
                        .collect::<Vec<&str>>();
                    if range_splits.len() != 2 {
                        return Err(format!(
                            "malformed char.def file, line: `{}`, splits: `{:?}`",
                            line, splits
                        )
                        .into());
                    }

                    let start = match i32::from_str_radix(range_splits[0], 16) {
                        Ok(start) => start,
                        Err(e) => {
                            return Err(format!(
                                "malformed start range: `{}`, err: `{}`",
                                range_splits[0], e
                            )
                            .into())
                        }
                    };

                    let end = match i32::from_str_radix(range_splits[1], 16) {
                        Ok(end) => end,
                        Err(e) => {
                            return Err(format!(
                                "malformed end range: `{}`, err: `{}`",
                                range_splits[1], e
                            )
                            .into())
                        }
                    };

                    for code in start..end + 1 {
                        code_to_category_map.insert(code, char_cls);
                    }
                } else {
                    // single point
                    let code = match i32::from_str_radix(splits[0].trim_start_matches("0x"), 16) {
                        Ok(code) => code,
                        Err(e) => {
                            return Err(format!(
                                "Cannot read char code `{}` while reading char.def, err: {}",
                                splits[0], e
                            )
                            .into())
                        }
                    };

                    code_to_category_map.insert(code, char_cls);
                }
            } else {
                // char category definition
                let splits = line.split(" ").collect::<Vec<&str>>();
                if splits.len() < 4 {
                    return Err(format!("malformed char.def file, line: `{}`", line).into());
                }

                let char_cls = match CharacterClass::from_name(splits[0]) {
                    Some(cls) => cls,
                    None => {
                        return Err(format!(
                            "cannot find character class `{}` while reading char.def",
                            splits[0]
                        )
                        .into())
                    }
                };

                let invoke = match splits[1].parse::<u8>() {
                    Ok(invoke) => invoke,
                    Err(_) => {
                        return Err(format!(
                            "cannot read invoke while reading char.def, line: `{}`",
                            line
                        )
                        .into())
                    }
                };

                let group = match splits[2].parse::<u8>() {
                    Ok(group) => group,
                    Err(_) => {
                        return Err(format!(
                            "cannot read group while reading char.def, line: `{}`",
                            line
                        )
                        .into())
                    }
                };

                let length = match splits[3].parse::<u8>() {
                    Ok(length) => length,
                    Err(_) => {
                        return Err(format!(
                            "cannot read length while reading char.def, line: `{}`",
                            line
                        )
                        .into())
                    }
                };

                invoke_map.insert(
                    char_cls,
                    CategoryDefinition {
                        invoke: invoke,
                        group: group,
                        length: length,
                    },
                );
            }
        }

        // 3. Save unknown token infos.
        info!("Saving unknown token infos ...");
        let unk_dictionary = UnknownTokenDictionary {
            class_morpheme_map: unk_class_morpheme_map,
            invoke_map: invoke_map,
            code_to_class_map: code_to_category_map,
        };
        let bin_unk_dictionary: Vec<u8> = match to_stdvec(&unk_dictionary) {
            Ok(bytes) => bytes,
            Err(_) => return Err("Failed to serialize unknown token dictionary".into()),
        };

        let mut file = match File::create(Path::new(output_path).join(UNK_FILENAME)) {
            Ok(f) => f,
            Err(_) => return Err("Failed to create output file".into()),
        };

        match file.write_all(bin_unk_dictionary.as_slice()) {
            Ok(_) => Ok(()),
            Err(_) => return Err("Failed to write unk dictionary".into()),
        }
    }

    // This method will build connection cost matrix.
    fn build_connection_cost(&self, input_path: &str, output_path: &str) -> Result<(), Error> {
        // 1. Read first line and check the number of columns.
        info!("Reading connection cost matrix ...");
        let connection_cost_file =
            match File::open(Path::new(input_path).join(MECAB_CON_COST_FILENAME)) {
                Ok(f) => f,
                Err(_) => return Err("Failed to open matrix.def file".into()),
            };
        let mut connection_cost_file_reader = BufReader::new(connection_cost_file);
        let mut first_def_line = String::new();
        match connection_cost_file_reader.read_line(&mut first_def_line) {
            Ok(_) => (),
            Err(_) => return Err("Failed to read first line of matrix.def file".into()),
        };
        let dimensions = first_def_line.trim().split(" ").collect::<Vec<_>>();
        if dimensions.len() != 2 {
            return Err(format!("Malformed matrix.def file, line: `{}`", first_def_line).into());
        }

        let forward_size = match dimensions[0].parse::<u32>() {
            Ok(size) => size,
            Err(_) => return Err(format!("malformed forward size: `{}`", dimensions[0]).into()),
        };
        let backward_size = match dimensions[1].parse::<u32>() {
            Ok(size) => size,
            Err(_) => return Err(format!("malformed backward size: `{}`", dimensions[1]).into()),
        };
        if forward_size <= 0 || backward_size <= 0 {
            return Err("Failed to parse matrix.def file, malformed matrix.def, forward/backward size must be positive".into());
        }

        // 2. Read all lines and build connection cost matrix.
        let mut connection_cost_matrix: Vec<i16> = (0..forward_size * backward_size)
            .into_iter()
            .map(|_| 0)
            .collect::<Vec<i16>>();
        for line in connection_cost_file_reader.lines() {
            let line = match line {
                Ok(l) => l,
                Err(_) => return Err("Failed to read line of matrix.def file".into()),
            };

            let splits = line.split(" ").collect::<Vec<_>>();

            let forward_id = match splits[0].parse::<u32>() {
                Ok(id) => id,
                Err(_) => return Err(format!("malformed forward id: `{}`", splits[0]).into()),
            };

            let backward_id = match splits[1].parse::<u32>() {
                Ok(id) => id,
                Err(_) => return Err(format!("malformed backward id: `{}`", splits[1]).into()),
            };

            let cost = match splits[2].parse::<i16>() {
                Ok(cost) => cost,
                Err(_) => return Err(format!("malformed cost: `{}`", splits[2]).into()),
            };

            connection_cost_matrix[(forward_id * backward_size + backward_id) as usize] = cost;
        }

        // 3. Save conn cost.
        info!("Saving connection cost matrix ...");
        let connection_cost = ConnectionCost {
            costs: connection_cost_matrix,
            forward_size: forward_size,
            backward_size: backward_size,
        };
        let bin_connection_cost: Vec<u8> = match to_stdvec(&connection_cost) {
            Ok(bytes) => bytes,
            Err(_) => return Err("Failed to serialize connection cost".into()),
        };

        let mut file = match File::create(Path::new(output_path).join(CON_COST_FILENAME)) {
            Ok(f) => f,
            Err(_) => return Err("Failed to create output file".into()),
        };

        match file.write_all(bin_connection_cost.as_slice()) {
            Ok(_) => Ok(()),
            Err(_) => return Err("Failed to write connection cost".into()),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_build_token_infos() {
        let tmpdir = tempfile::tempdir().unwrap();

        let builder = DictionaryBuilder::new(None);
        let input_path = "testdata/tokenizer/dictionary/builder";
        let result =
            builder.build_token_infos(input_path, tmpdir.path().display().to_string().as_str());

        assert!(result.is_ok(), "{:#?}", result.err().unwrap());
        assert!(
            tmpdir.path().join(FST_FILENAME).is_file(),
            "Cannot find output: FST file"
        );
        assert!(
            tmpdir.path().join(TOKEN_FILENAME).is_file(),
            "Cannot find output: TOKEN file"
        );
    }

    #[test]
    fn test_build_unk_dictionary() {
        let tmpdir = tempfile::tempdir().unwrap();

        let builder = DictionaryBuilder::new(None);
        let input_path = "testdata/tokenizer/dictionary/builder";
        let result =
            builder.build_unk_dictionary(input_path, tmpdir.path().display().to_string().as_str());

        assert!(result.is_ok(), "{:#?}", result.err().unwrap());
        assert!(
            tmpdir.path().join(UNK_FILENAME).is_file(),
            "Cannot find output: UNK file"
        );
    }

    #[test]
    fn test_build_connection_cost() {
        let tmpdir = tempfile::tempdir().unwrap();

        let builder = DictionaryBuilder::new(None);
        let input_path = "testdata/tokenizer/dictionary/builder";
        let result =
            builder.build_connection_cost(input_path, tmpdir.path().display().to_string().as_str());

        assert!(result.is_ok(), "{:#?}", result.err().unwrap());
        assert!(
            tmpdir.path().join(CON_COST_FILENAME).is_file(),
            "Cannot find output: CON COST file"
        );
    }
}
