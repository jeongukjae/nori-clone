use std::{
    borrow::Borrow,
    io::{BufRead, Read},
};

use fst::{Map, MapBuilder};
use regex::Regex;

use crate::{error::Error, utils};

use super::{AdditionalMetadata, Morpheme, MorphemeExpression, POSTag, POSType};

pub struct UserDictionary {
    pub fst: Map<Vec<u8>>,
    pub morphemes: Vec<Morpheme>,
}

impl UserDictionary {
    pub fn load(
        terms: Vec<(String, Vec<String>)>,
        additional_metadata: &AdditionalMetadata,
    ) -> Result<Self, Error> {
        let mut terms = terms;
        terms.sort_by_key(|k| k.0.clone());

        let mut builder = MapBuilder::memory();
        let mut morphemes = Vec::new();
        for (index, terms) in terms.into_iter().enumerate() {
            match builder.insert(terms.0.as_bytes(), index as u64) {
                Ok(_) => (),
                Err(e) => return Err(format!("failed to insert key: {}", e).into()),
            };

            let expression = (1..terms.1.len())
                .map(|i| MorphemeExpression {
                    surface: terms.1[i].clone(),
                    pos_tag: POSTag::NNG,
                })
                .collect();
            let pos_tags = if terms.1.is_empty() {
                vec![POSTag::NNG]
            } else {
                (0..terms.1.len() - 1).map(|_| POSTag::NNG).collect()
            };
            let pos_type = if terms.1.len() == 1 {
                POSType::MORPHEME
            } else {
                POSType::COMPOUND
            };

            let (is_hangul, has_coda) = utils::uchar::check_coda(&terms.0);
            let right_id = if !is_hangul {
                additional_metadata.right_id_nng
            } else if !has_coda {
                additional_metadata.right_id_nng_wo_coda
            } else {
                additional_metadata.right_id_nng_w_coda
            };

            // NOTE: we treat all words in the user dictionary as NNG.
            morphemes.push(Morpheme {
                word_cost: -100000,
                left_id: additional_metadata.left_id_nng,
                right_id,
                pos_type,
                pos_tags,
                expressions: expression,
            })
        }

        let data = match builder.into_inner() {
            Ok(fst) => fst,
            Err(e) => return Err(format!("failed to build user dictionary: {:?}", e).into()),
        };

        let fst = match Map::new(data) {
            Ok(map) => map,
            Err(e) => return Err(format!("failed to build user dictionary: {:?}", e).into()),
        };

        Ok(UserDictionary { fst, morphemes })
    }

    pub fn load_from_bytes(
        b: Vec<u8>,
        additional_metadata: &AdditionalMetadata,
    ) -> Result<Self, Error> {
        let pat_for_remove_space = Regex::new(r"\s+").unwrap();
        let pat_for_remove_comment = Regex::new(r"#.*").unwrap();

        let mut terms: Vec<(String, Vec<String>)> = Vec::new();
        for line in b.lines() {
            let line = match line {
                Ok(line) => line,
                Err(e) => return Err(format!("failed to read line: {}", e).into()),
            };

            let line = pat_for_remove_space.replace_all(line.as_str(), " ");
            let line = pat_for_remove_comment.replace_all(line.borrow(), "");
            let line = line.trim();
            if line.is_empty() {
                continue;
            }

            let splits: Vec<String> = line.split(' ').map(|k| k.to_owned()).collect();
            terms.push((splits[0].clone(), splits[1..].to_vec()));
        }

        Self::load(terms, additional_metadata)
    }

    pub fn load_from_file(
        input_path: &str,
        additional_metadata: &AdditionalMetadata,
    ) -> Result<Self, Error> {
        let mut f = match std::fs::File::open(input_path) {
            Ok(f) => f,
            Err(e) => return Err(format!("failed to open file: {}", e).into()),
        };
        let mut b = Vec::new();
        match f.read_to_end(&mut b) {
            Ok(_) => (),
            Err(e) => return Err(format!("failed to read file: {}", e).into()),
        };
        Self::load_from_bytes(b, additional_metadata)
    }

    // Read all *.txt files from input_dir and concat all file contents.
    pub fn load_from_input_directory(
        input_dir: &str,
        additional_metadata: &AdditionalMetadata,
    ) -> Result<Self, Error> {
        let mut all_buffers: Vec<u8> = Vec::new();
        for entry in std::fs::read_dir(input_dir)? {
            let entry = match entry {
                Ok(entry) => entry,
                Err(e) => return Err(format!("failed to read directory: {}", e).into()),
            };
            let path = entry.path();
            if path.extension().unwrap() != "txt" {
                continue;
            }

            let mut f = match std::fs::File::open(path) {
                Ok(f) => f,
                Err(e) => return Err(format!("failed to open file: {}", e).into()),
            };
            let mut b = Vec::new();
            match f.read_to_end(&mut b) {
                Ok(_) => (),
                Err(e) => return Err(format!("failed to read file: {}", e).into()),
            };
            all_buffers.append(&mut b);
            all_buffers.append(&mut "\n".as_bytes().to_vec())
        }

        Self::load_from_bytes(all_buffers, additional_metadata)
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_load_ok() {
        let terms: Vec<(String, Vec<String>)> = vec![
            ("hello".into(), vec!["hello".into()]),
            ("world".into(), vec!["world".into()]),
            ("hello world".into(), vec!["hello world".into()]),
        ];
        let user_dict = UserDictionary::load(
            terms,
            &AdditionalMetadata {
                left_id_nng: 0,
                right_id_nng: 0,
                right_id_nng_w_coda: 0,
                right_id_nng_wo_coda: 0,
            },
        );
        assert!(user_dict.is_ok());
    }
}
