use crate::{error::Error, utils};
use std::collections::HashMap;

use serde::{Deserialize, Serialize};
use serde_repr::*;

pub const MECAB_UNK_FILENAME: &str = "unk.def";
pub const MECAB_CHAR_FILENAME: &str = "char.def";
pub const MECAB_CON_COST_FILENAME: &str = "matrix.def";
pub const MECAB_LEFT_ID_FILENAME: &str = "left-id.def";
pub const MECAB_RIGHT_ID_FILENAME: &str = "right-id.def";

pub const FST_FILENAME: &str = "fst.bin";
pub const TOKEN_FILENAME: &str = "token.bin";
pub const UNK_FILENAME: &str = "unk.bin";
pub const CON_COST_FILENAME: &str = "matrix.bin";

#[derive(Serialize, Deserialize, Debug)]
pub struct TokenDictionary {
    pub morphemes: Vec<Vec<Morpheme>>, // the morphemes those have the same surfaces are grouped.
}

#[derive(Serialize, Deserialize, Debug)]
pub struct UnknownTokenDictionary {
    pub code_to_class_map: HashMap<u32, CharacterClass>,
    pub class_morpheme_map: HashMap<CharacterClass, Morpheme>,
    pub invoke_map: HashMap<CharacterClass, CategoryDefinition>,
}

impl UnknownTokenDictionary {
    pub fn get_ch_cls(&self, ch: char) -> CharacterClass {
        match self.code_to_class_map.get(&(ch as u32)) {
            Some(ch_cls) => *ch_cls,
            None => CharacterClass::HANGUL,
        }
    }

    pub fn get_ch_cls_with_vec(
        &self,
        bytes: &[u8],
        offset: usize,
    ) -> Result<CharacterClass, Error> {
        let ch = match utils::uchar::get_next_char(bytes, offset) {
            Ok(ch) => ch,
            Err(e) => return Err(e),
        };

        Ok(self.get_ch_cls(ch))
    }

    pub fn get_char_def(&self, bytes: &[u8], offset: usize) -> Result<CategoryDefinition, Error> {
        let ch_cls = match self.get_ch_cls_with_vec(bytes, offset) {
            Ok(ch_cls) => ch_cls,
            Err(e) => return Err(e),
        };

        Ok(self.invoke_map[&ch_cls])
    }
}

#[derive(Serialize, Deserialize, Debug)]
pub struct ConnectionCost {
    // index: backward_size * forward_index + backward_index
    pub costs: Vec<i16>,
    pub forward_size: u32,
    pub backward_size: u32,

    // Additional information for user dictionary
    pub additional: AdditionalMetadata,
}

impl ConnectionCost {
    pub fn get_cost(&self, right_id: u16, left_id: u16) -> i16 {
        self.costs[(self.backward_size * (right_id as u32) + (left_id as u32)) as usize]
    }
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct Morpheme {
    pub left_id: u16,
    pub right_id: u16,
    pub word_cost: i32,
    pub pos_type: POSType,
    pub pos_tags: Vec<POSTag>,
    pub expressions: Vec<MorphemeExpression>,
}

impl Morpheme {
    pub fn from_csv_record(entry: &MeCabTokenCSVRecord) -> Result<Morpheme, Error> {
        let pos_type = match POSType::from_name(entry.pos_type.as_str()) {
            Some(ptype) => ptype,
            None => return Err(Error(format!("Invalid POS type `{}`", entry.pos_type))),
        };

        let pos_tags = match entry
            .pos_tags
            .split("+")
            .map(|p| POSTag::from_name(p))
            .collect::<Option<Vec<_>>>()
        {
            Some(ptags) => ptags,
            None => return Err(Error(format!("Invalid POS tags `{}`", entry.pos_tags))),
        };

        let mut expressions = Vec::<MorphemeExpression>::new();
        if entry.expression != "*" {
            for expr in entry
                .expression
                .split("+")
                .map(|s| s.split("/").collect::<Vec<&str>>())
            {
                if expr.len() != 3 {
                    return Err(Error(format!("Invalid expression `{:?}`", expr)));
                }

                expressions.push(MorphemeExpression {
                    surface: expr[0].to_string(),
                    pos_tag: match POSTag::from_name(expr[1]) {
                        Some(ptag) => ptag,
                        None => return Err(Error(format!("Invalid POS tag `{}`", expr[1]))),
                    },
                });
            }
        }

        Ok(Morpheme {
            left_id: entry.left_id,
            right_id: entry.right_id,
            word_cost: entry.word_cost,
            pos_type: pos_type,
            pos_tags: pos_tags,
            expressions: expressions,
        })
    }
}

#[derive(Serialize_repr, Deserialize_repr, PartialEq, Debug, Clone, Copy)]
#[repr(u8)]
pub enum POSType {
    MORPHEME = 0,    // A simple morpheme.
    COMPOUND = 1,    // Compound noun.
    INFLECT = 2,     // Inflected token.
    PREANALYSIS = 3, // Pre-analysis token.
}

impl POSType {
    pub fn from_name(s: &str) -> Option<POSType> {
        let s = s.to_uppercase();
        match s.as_str() {
            "MORPHEME" | "*" => Some(POSType::MORPHEME),
            "COMPOUND" => Some(POSType::COMPOUND),
            "INFLECT" => Some(POSType::INFLECT),
            "PREANALYSIS" => Some(POSType::PREANALYSIS),
            _ => None,
        }
    }
}

#[derive(Serialize_repr, Deserialize_repr, PartialEq, Debug, Clone, Copy)]
#[repr(u8)]
pub enum POSTag {
    UNKNOWN = 0, // Unknown

    E = 1,     // Verbal endings
    IC = 2,    // Interjection
    J = 3,     // Ending Particle
    MAG = 4,   // General Adverb
    MAJ = 5,   // Conjunctive adverb
    MM = 6,    // Modifier
    NNG = 7,   // General Noun
    NNP = 8,   // Proper Noun
    NNB = 9,   // Dependent noun
    NNBC = 10, // Dependent noun
    NP = 11,   // Pronoun
    NR = 12,   // Numeral
    SF = 13,   // Terminal punctuation
    SH = 14,   // Chinese Characeter
    SL = 15,   // Foreign language
    SN = 16,   // Number
    SP = 17,   // Space
    SSC = 18,  // Closing brackets
    SSO = 19,  // Opening brackets
    SC = 20,   // Separator
    SY = 21,   // Other symbol
    SE = 22,   // Ellipsis
    VA = 23,   // Adjective
    VCN = 24,  // Negative designator
    VCP = 25,  // Positive designator
    VV = 26,   // Verb
    VX = 27,   // Auxiliary Verb or Adjective
    XPN = 28,  // Prefix
    XR = 29,   // Root
    XSA = 30,  // Adjective Suffix
    XSN = 31,  // Noun Suffix
    XSV = 32,  // Verb Suffix
}

// Aliasing for the POSTag enum.
impl POSTag {
    pub const UNA: Self = Self::UNKNOWN;
    pub const NA: Self = Self::UNKNOWN;
    pub const VSV: Self = Self::UNKNOWN;

    pub fn from_name(s: &str) -> Option<Self> {
        let s = s.to_uppercase();
        if s.starts_with("J") {
            return Some(Self::J);
        }
        if s.starts_with("E") {
            return Some(Self::E);
        }

        Some(match s.as_str() {
            "UNKNOWN" | "UNA" | "NA" | "VSV" => Self::UNKNOWN,
            "E" => Self::E,
            "IC" => Self::IC,
            "J" => Self::J,
            "MAG" => Self::MAG,
            "MAJ" => Self::MAJ,
            "MM" => Self::MM,
            "NNG" => Self::NNG,
            "NNP" => Self::NNP,
            "NNB" => Self::NNB,
            "NNBC" => Self::NNBC,
            "NP" => Self::NP,
            "NR" => Self::NR,
            "SF" => Self::SF,
            "SH" => Self::SH,
            "SL" => Self::SL,
            "SN" => Self::SN,
            "SP" => Self::SP,
            "SSC" => Self::SSC,
            "SSO" => Self::SSO,
            "SC" => Self::SC,
            "SY" => Self::SY,
            "SE" => Self::SE,
            "VA" => Self::VA,
            "VCN" => Self::VCN,
            "VCP" => Self::VCP,
            "VV" => Self::VV,
            "VX" => Self::VX,
            "XPN" => Self::XPN,
            "XR" => Self::XR,
            "XSA" => Self::XSA,
            "XSN" => Self::XSN,
            "XSV" => Self::XSV,
            _ => return None,
        })
    }
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct MorphemeExpression {
    pub pos_tag: POSTag,
    pub surface: String,
}

// Character Classes.
// Similar with unicode character groups.
#[derive(Serialize_repr, Deserialize_repr, PartialEq, Hash, Eq, Debug, Clone, Copy)]
#[repr(u8)]
pub enum CharacterClass {
    NGRAM = 0,
    DEFAULT = 1,
    SPACE = 2,
    SYMBOL = 3,
    NUMERIC = 4,
    ALPHA = 5,
    CYRILLIC = 6,
    GREEK = 7,
    HIRAGANA = 8,
    KATAKANA = 9,
    KANJI = 10,
    HANGUL = 11,
    HANJA = 12,
    HANJANUMERIC = 13,
}

impl CharacterClass {
    pub fn from_name(s: &str) -> Option<Self> {
        Some(match s {
            "NGRAM" => Self::NGRAM,
            "DEFAULT" => Self::DEFAULT,
            "SPACE" => Self::SPACE,
            "SYMBOL" => Self::SYMBOL,
            "NUMERIC" => Self::NUMERIC,
            "ALPHA" => Self::ALPHA,
            "CYRILLIC" => Self::CYRILLIC,
            "GREEK" => Self::GREEK,
            "HIRAGANA" => Self::HIRAGANA,
            "KATAKANA" => Self::KATAKANA,
            "KANJI" => Self::KANJI,
            "HANGUL" => Self::HANGUL,
            "HANJA" => Self::HANJA,
            "HANJANUMERIC" => Self::HANJANUMERIC,
            _ => return None,
        })
    }
}

#[derive(Serialize, Deserialize, Debug, Copy, Clone)]
pub struct CategoryDefinition {
    pub invoke: u8,
    pub group: u8,
    pub length: u8,
}

// Additional information for the user dictionary.
#[derive(Serialize, Deserialize, Debug)]
pub struct AdditionalMetadata {
    pub left_id_nng: u16,
    pub right_id_nng: u16,
    pub right_id_nng_w_coda: u16,
    pub right_id_nng_wo_coda: u16,
}

// CSV Record struct to read the MeCab dictionary.
#[derive(Deserialize, Debug)]
pub struct MeCabTokenCSVRecord {
    pub surface: String,
    pub left_id: u16,
    pub right_id: u16,
    pub word_cost: i32,
    pub pos_tags: String,
    pub semantic_class: String,
    pub is_coda: String,
    pub reading_form: String,
    pub pos_type: String,
    pub left_pos: String,
    pub right_pos: String,
    pub expression: String,
}

#[derive(Deserialize)]
pub struct MeCabUnkCSVRecord {
    pub category: String,
    pub left_id: u16,
    pub right_id: u16,
    pub word_cost: i32,
    pub pos_tags: String,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_postag_from_name() {
        assert_eq!(POSTag::from_name("UNKNOWN"), Some(POSTag::UNKNOWN));
        assert_eq!(POSTag::from_name("NNG"), Some(POSTag::NNG));
        assert_eq!(POSTag::from_name("NNP"), Some(POSTag::NNP));
        assert_eq!(POSTag::from_name("NNB"), Some(POSTag::NNB));
        assert_eq!(POSTag::from_name("XR"), Some(POSTag::XR));
    }

    #[test]
    fn test_postype_from_name() {
        assert_eq!(POSType::from_name("MORPHEME"), Some(POSType::MORPHEME));
        assert_eq!(POSType::from_name("COMPOUND"), Some(POSType::COMPOUND));
    }

    #[test]
    fn test_morpheme_from_csv_record() {
        let entry = MeCabTokenCSVRecord {
            surface: "아버지".to_string(),
            left_id: 0,
            right_id: 1,
            word_cost: 0,
            semantic_class: "".to_string(),
            reading_form: "".to_string(),
            is_coda: "F".to_string(),
            pos_type: "MORPHEME".to_string(),
            pos_tags: "NNG".to_string(),
            expression: "*".to_string(),
            left_pos: "*".to_string(),
            right_pos: "*".to_string(),
        };

        let morpheme = Morpheme::from_csv_record(&entry).unwrap();
        assert_eq!(morpheme.left_id, 0);
        assert_eq!(morpheme.right_id, 1);
        assert_eq!(morpheme.word_cost, 0);
        assert_eq!(morpheme.pos_type, POSType::MORPHEME);
        assert_eq!(morpheme.pos_tags, vec![POSTag::NNG]);
        assert_eq!(morpheme.expressions.len(), 0);
    }
}
