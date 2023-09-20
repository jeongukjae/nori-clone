use std::io::Read;
use std::path::PathBuf;
use std::rc::Rc;
use std::{fs::File, io::BufReader, path::Path};

use crate::error::Error;

use super::*;
use daachorse::CharwiseDoubleArrayAhoCorasick;
use postcard::from_bytes;

pub struct SystemDictionary {
    pub ahocorasick: CharwiseDoubleArrayAhoCorasick<usize>,
    pub token_dictionary: TokenDictionary,
    pub unk_dictionary: UnknownTokenDictionary,
    pub connection_cost: ConnectionCost,

    pub bos_eos_morpheme: Rc<Morpheme>,
}

impl SystemDictionary {
    pub fn load_from_bytes(
        ahocorasick_buffer: Vec<u8>,
        token_dict_buffer: Vec<u8>,
        unk_dict_buffer: Vec<u8>,
        conn_cost_buffer: Vec<u8>,
    ) -> Result<Self, Error> {
        let token_dict = match from_bytes(&token_dict_buffer) {
            Ok(dict) => dict,
            Err(e) => return Err(format!("Failed to load token dictionary ({:?})", e).into()),
        };

        let (ahocorasick, _) = unsafe {
            CharwiseDoubleArrayAhoCorasick::<usize>::deserialize_unchecked(&ahocorasick_buffer)
        };

        let unk_dict = match from_bytes(&unk_dict_buffer) {
            Ok(dict) => dict,
            Err(e) => return Err(format!("Failed to load unk dictionary ({:?})", e).into()),
        };

        let connection_cost = match from_bytes(&conn_cost_buffer) {
            Ok(dict) => dict,
            Err(e) => return Err(format!("Failed to load unk dictionary ({:?})", e).into()),
        };

        Ok(SystemDictionary {
            token_dictionary: token_dict,
            unk_dictionary: unk_dict,
            connection_cost,
            ahocorasick,

            bos_eos_morpheme: Rc::new(Morpheme {
                left_id: 0,
                right_id: 0,
                word_cost: 0,
                pos_type: POSType::MORPHEME,
                pos_tags: vec![POSTag::UNKNOWN],
                expressions: vec![],
            }),
        })
    }

    pub fn load_from_input_directory(input_path: &str) -> Result<Self, Error> {
        let ahocorasick_buffer =
            match Self::read_file(Path::new(input_path).join(AHOCORASICK_FILENAME)) {
                Ok(buffer) => buffer,
                Err(e) => return Err(format!("Failed to load ahocorasick: ({:?})", e).into()),
            };

        let token_dict_buffer = match Self::read_file(Path::new(input_path).join(TOKEN_FILENAME)) {
            Ok(buffer) => buffer,
            Err(e) => return Err(format!("Failed to load token dictionary ({:?})", e).into()),
        };

        let unk_dict_buffer = match Self::read_file(Path::new(input_path).join(UNK_FILENAME)) {
            Ok(buffer) => buffer,
            Err(e) => return Err(format!("Failed to load unk dictionary ({:?})", e).into()),
        };

        let conn_cost_buffer = match Self::read_file(Path::new(input_path).join(CON_COST_FILENAME))
        {
            Ok(buffer) => buffer,
            Err(e) => return Err(format!("Failed to load unk dictionary ({:?})", e).into()),
        };

        Self::load_from_bytes(
            ahocorasick_buffer,
            token_dict_buffer,
            unk_dict_buffer,
            conn_cost_buffer,
        )
    }

    fn read_file(path: PathBuf) -> Result<Vec<u8>, Error> {
        let f = match File::open(path) {
            Ok(f) => f,
            Err(e) => return Err(format!("Failed to open file: ({:?})", e).into()),
        };
        let mut reader = BufReader::new(f);
        let mut buffer = Vec::new();

        match reader.read_to_end(&mut buffer) {
            Ok(_) => (),
            Err(e) => return Err(format!("Failed to read file: ({:?})", e).into()),
        };

        Ok(buffer)
    }
}