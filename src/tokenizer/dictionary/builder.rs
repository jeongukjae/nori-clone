use crate::tokenizer::dictionary::model;

use std::{
    fs::{self, File},
    io::{BufWriter, Write, BufReader, BufRead},
    path::{Path, PathBuf},
};

use postcard::to_stdvec;
use unicode_normalization::UnicodeNormalization;

use fst::MapBuilder;
use model::*;

use super::TokenDictionary;

// Supported normalization form to build dictionary.
pub enum NormalizationForm {
    NFKC,
}

pub struct DictionaryBuilder {
    normalize: bool,
    normalization_form: NormalizationForm,
}

impl DictionaryBuilder {
    pub fn new(
        normalize: Option<bool>,
        normalization_form: Option<NormalizationForm>,
    ) -> DictionaryBuilder {
        let normalize_ = normalize.unwrap_or(false);
        let normalization_form_ = normalization_form.unwrap_or(NormalizationForm::NFKC);

        DictionaryBuilder {
            normalize: normalize_,
            normalization_form: normalization_form_,
        }
    }

    pub fn build(&self, input_path: &str, output_path: &str) -> Result<(), &str> {
        _ = match self.build_token_infos(input_path, output_path) {
            Ok(_) => (),
            Err(e) => return Err(e),
        };

        _ = match self.build_unk_dictionary(input_path, output_path) {
            Ok(_) => (),
            Err(e) => return Err(e),
        };

        Ok(())
    }

    // This method builds token informations from the input file and save FST and token dictionary.
    fn build_token_infos(&self, input_path: &str, output_path: &str) -> Result<(), &'static str> {
        // 1. List all csv files.
        let files = match fs::read_dir(input_path) {
            Ok(files) => files,
            Err(_) => return Err("Failed to read input directory"),
        };

        let files: Vec<PathBuf> = files
            .map(|r| r.map(|d| d.path()))
            .filter(|r| {
                r.is_ok()
                    && r.as_deref().unwrap().is_file()
                    && r.as_deref().unwrap().extension().unwrap() == "csv"
            })
            .map(|r| r.unwrap())
            .collect();

        if files.len() == 0 {
            return Err("No csv files found");
        }

        // 2. Read all csvs.
        let out_fst_file = match File::create(Path::new(output_path).join(FST_FILENAME)) {
            Ok(f) => f,
            Err(_) => return Err("Failed to create output file"),
        };
        let fst_writer = BufWriter::new(out_fst_file);
        let mut fst_builder = match MapBuilder::new(fst_writer) {
            Ok(b) => b,
            Err(_) => return Err("Failed to create fst builder"),
        };

        let mut records: Vec<MeCabCSVRecord> = Vec::new();

        for file in files {
            let mut contents = match fs::read_to_string(file) {
                Ok(contents) => contents,
                Err(_) => return Err("Failed to read csv file"),
            };
            if self.normalize {
                contents = normalize_unicode(contents, &self.normalization_form)
            }
            let mut csv_reader = csv::ReaderBuilder::new()
                .has_headers(false)
                .from_reader(contents.as_bytes());
            let mut records_for_file: Vec<MeCabCSVRecord> = csv_reader
                .deserialize::<MeCabCSVRecord>()
                .filter(|r| r.is_ok())
                .map(|r| r.unwrap())
                .collect();
            records.append(&mut records_for_file);
        }

        // 3. Build FST from read rows.
        //
        // We build FST for search and TokenDictionary struct for metadata.
        let mut morphemes_by_surface: Vec<Vec<Morpheme>> = Vec::new();
        records.sort_by(|a, b| a.surface.cmp(&b.surface));

        let mut fst_index: usize = 0;
        for (index, record) in records.iter().enumerate() {
            if index == 0 || record.surface != records[index - 1].surface {
                match fst_builder.insert(record.surface.clone(), fst_index.try_into().unwrap()) {
                    Ok(_) => (),
                    Err(_) => return Err("Failed to insert into fst"),
                };

                morphemes_by_surface.push(Vec::new());
                fst_index += 1;
            }

            let morpheme = match Morpheme::from_csv_record(record) {
                Ok(morpheme) => morpheme,
                Err(e) => return Err(e),
            };

            morphemes_by_surface[fst_index - 1].push(morpheme)
        }

        // 4. Finishing the process.
        match fst_builder.finish() {
            Ok(_) => (),
            Err(_) => return Err("Failed to build FST"),
        };

        let token_dictionary = TokenDictionary {
            morphemes: morphemes_by_surface,
        };
        let bin_token_dictionary: Vec<u8> = match to_stdvec(&token_dictionary) {
            Ok(bytes) => bytes,
            Err(_) => return Err("Failed to serialize token dictionary"),
        };

        let mut file = match File::create(Path::new(output_path).join(TOKEN_FILENAME)) {
            Ok(f) => f,
            Err(_) => return Err("Failed to create output file"),
        };

        match file.write_all(bin_token_dictionary.as_slice()) {
            Ok(_) => Ok(()),
            Err(_) => return Err("Failed to write token dictionary"),
        }
    }

    // This method will build unknown token dictionary using unk.def and char.def files.
    fn build_unk_dictionary(
        &self,
        input_path: &str,
        output_path: &str,
    ) -> Result<(), &'static str> {
        // 1. Build unknown token infos.
        let unk_file = match File::open(Path::new(input_path).join(MECAB_UNK_FILENAME)) {
            Ok(f) => f,
            Err(_) => return Err("Failed to open unk.def file"),
        };
        let unk_reader = BufReader::new(unk_file);
        unk_reader.lines().filter(|r| r.is_ok()).map(|r| r.unwrap());
        // XXX: implment this.

        Ok(())
    }
}

fn normalize_unicode(input: String, form: &NormalizationForm) -> String {
    match form {
        NormalizationForm::NFKC => input.nfkc().collect(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_build_token_infos() {
        let tmpdir = tempfile::tempdir().unwrap();

        let builder = DictionaryBuilder::new(None, None);
        let input_path = "testdata/tokenizer/dictionary/builder";
        let result =
            builder.build_token_infos(input_path, tmpdir.path().display().to_string().as_str());

        assert!(result.is_ok(), "{}", result.err().unwrap());
        assert!(
            tmpdir.path().join(FST_FILENAME).is_file(),
            "Cannot find output: FST file"
        );
        assert!(
            tmpdir.path().join(TOKEN_FILENAME).is_file(),
            "Cannot find output: TOKEN file"
        );
    }
}
