use std::io::Read;
use std::{fs::File, io::BufReader, path::Path};

use crate::error::Error;

use super::*;
use fst::Map;
use postcard::from_bytes;

pub struct SystemDictionary {
    pub fst: Map<Vec<u8>>,
    pub token_dictionary: TokenDictionary,
    pub unk_dictionary: UnknownTokenDictionary,
    pub connection_cost: ConnectionCost,
}

impl SystemDictionary {
    pub fn load(input_path: &str) -> Result<Self, Error> {
        let fst = match Self::load_fst(input_path ) {
            Ok(fst) => fst,
            Err(e) => return Err(e),
        };
        let token_dict = match Self::load_token_dictionary(input_path) {
            Ok(f) => f,
            Err(e) => return Err(e),
        };
        let unk_dict = match Self::load_unk_dictionary(input_path) {
            Ok(f) => f,
            Err(e) => return Err(e),
        };
        let connection_cost = match Self::load_connection_cost(input_path) {
            Ok(f) => f,
            Err(e) => return Err(e),
        };

        Ok(SystemDictionary {
            fst: fst,
            token_dictionary: token_dict,
            unk_dictionary: unk_dict,
            connection_cost: connection_cost,
        })
    }

    fn load_fst(input_path: &str) -> Result<Map<Vec<u8>>, Error> {
        let fst_path = Path::new(input_path).join(FST_FILENAME);
        let f = match File::open(fst_path) {
            Ok(f) => f,
            Err(e) => return Err(format!("Failed to open fst file: ({:?})", e).into()),
        };
        let mut reader = BufReader::new(f);
        let mut buffer = Vec::new();

        match reader.read_to_end(&mut buffer) {
            Ok(_) => (),
            Err(e) => return Err(format!("Failed to read fst file: ({:?})", e).into()),
        };

        match Map::new(buffer) {
            Ok(fst) => Ok(fst),
            Err(e) => return Err(format!("Failed to load fst: ({:?})", e).into()),
        }
    }

    fn load_token_dictionary(input_path: &str) -> Result<TokenDictionary, Error> {
        let token_dict_path = Path::new(input_path).join(TOKEN_FILENAME);
        let f = match File::open(token_dict_path) {
            Ok(f) => f,
            Err(e) => return Err(format!("Failed to open token dictionary file: ({:?})", e).into()),
        };
        let mut reader = BufReader::new(f);
        let mut buffer = Vec::new();

        match reader.read_to_end(&mut buffer) {
            Ok(_) => (),
            Err(e) => return Err(format!("Failed to read token dictionary file: ({:?})", e).into()),
        };

        let token_dictionary = match from_bytes(&buffer) {
            Ok(token_dictionary) => token_dictionary,
            Err(e) => {
                return Err(format!("Failed to deserialize token dictionary file ({:?})", e).into())
            }
        };

        Ok(token_dictionary)
    }

    fn load_unk_dictionary(input_path: &str) -> Result<UnknownTokenDictionary, Error> {
        let unk_dict_path = Path::new(input_path).join(UNK_FILENAME);
        let f = match File::open(unk_dict_path) {
            Ok(f) => f,
            Err(e) => {
                return Err(format!("Failed to open unknown unk dictionary file ({:?})", e).into())
            }
        };
        let mut reader = BufReader::new(f);
        let mut buffer = Vec::new();

        match reader.read_to_end(&mut buffer) {
            Ok(_) => (),
            Err(e) => {
                return Err(format!("Failed to read unknown unk dictionary file ({:?})", e).into())
            }
        };

        let unk_dictionary = match from_bytes(&buffer) {
            Ok(unk_dictionary) => unk_dictionary,
            Err(e) => {
                return Err(format!(
                    "Failed to deserialize unknown unk dictionary file ({:?})",
                    e
                )
                .into())
            }
        };

        Ok(unk_dictionary)
    }

    fn load_connection_cost(input_path: &str) -> Result<ConnectionCost, Error> {
        let connection_cost_path = Path::new(input_path).join(CON_COST_FILENAME);
        let f = match File::open(connection_cost_path) {
            Ok(f) => f,
            Err(e) => return Err(format!("Failed to open connection cost file ({:?})", e).into()),
        };
        let mut reader = BufReader::new(f);
        let mut buffer = Vec::new();

        match reader.read_to_end(&mut buffer) {
            Ok(_) => (),
            Err(e) => return Err(format!("Failed to read connection cost file ({:?})", e).into()),
        };

        let connection_cost = match from_bytes(&buffer) {
            Ok(connection_cost) => connection_cost,
            Err(e) => {
                return Err(format!("Failed to deserialize connection cost file ({:?})", e).into())
            }
        };

        Ok(connection_cost)
    }
}
