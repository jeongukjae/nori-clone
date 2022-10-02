use serde::{Deserialize, Serialize};
use serde_repr::*;

pub const MECAB_UNK_FILENAME: &str = "unk.def";
pub const MECAB_CHAR_FILENAME: &str = "char.def";

pub const FST_FILENAME: &str = "fst.bin";
pub const TOKEN_FILENAME: &str = "token.bin";

// CSV Record struct to read the MeCab dictionary.
#[derive(Deserialize)]
pub struct MeCabCSVRecord {
    pub surface: String,
    pub left_id: u16,
    pub right_id: u16,
    pub word_cost: u16,
    pub pos_tags: String,
    pub semantic_class: String,
    pub is_coda: String,
    pub reading_form: String,
    pub pos_type: String,
    pub left_pos: String,
    pub right_pos: String,
    pub expression: String,
}

#[derive(Serialize_repr, Deserialize_repr, PartialEq, Debug)]
#[repr(u8)]
pub enum POSType {
    MORPHEME = 0,    // A simple morpheme.
    COMPOUND = 1,    // Compound noun.
    INFLECT = 2,     // Inflected token.
    PREANALYSIS = 3, // Pre-analysis token.
}

impl POSType {
    pub fn from_name(s: &str) -> Option<POSType> {
        match s {
            "MORPHEME" | "*" => Some(POSType::MORPHEME),
            "COMPOUND" => Some(POSType::COMPOUND),
            "INFLECT" => Some(POSType::INFLECT),
            "PREANALYSIS" => Some(POSType::PREANALYSIS),
            _ => None,
        }
    }
}

#[derive(Serialize_repr, Deserialize_repr, PartialEq, Debug)]
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
        if s.starts_with("J") {
            return Some(Self::J);
        }
        if s.starts_with("E") {
            return Some(Self::E);
        }

        Some(match s {
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

#[derive(Serialize, Deserialize, Debug)]
pub struct MorphemeExpression {
    pub pos_tag: POSTag,
    pub surface: String,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct Morpheme {
    pub left_id: u16,
    pub right_id: u16,
    pub word_cost: u16,
    pub pos_type: POSType,
    pub pos_tags: Vec<POSTag>,
    pub expressions: Vec<MorphemeExpression>,
}

impl Morpheme {
    pub fn from_csv_record(entry: &MeCabCSVRecord) -> Result<Morpheme, &'static str> {
        let pos_type = match POSType::from_name(entry.pos_type.as_str()) {
            Some(ptype) => ptype,
            None => return Err("Invalid POS type"),
        };

        let pos_tags = match entry
            .pos_tags
            .split("+")
            .map(|p| POSTag::from_name(p))
            .collect::<Option<Vec<_>>>()
        {
            Some(ptags) => ptags,
            None => return Err("Invalid POS tags"),
        };

        let mut expressions = Vec::<MorphemeExpression>::new();
        if entry.expression != "*" {
            for expr in entry
                .expression
                .split("+")
                .map(|s| s.split("/").collect::<Vec<&str>>())
            {
                if expr.len() != 2 {
                    return Err("Invalid expression");
                }

                expressions.push(MorphemeExpression {
                    pos_tag: match POSTag::from_name(expr[0]) {
                        Some(ptag) => ptag,
                        None => return Err("Invalid POS tag"),
                    },
                    surface: expr[1].to_string(),
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

#[derive(Serialize, Deserialize, Debug)]
pub struct TokenDictionary {
    pub morphemes: Vec<Vec<Morpheme>>, // the morphemes those have the same surfaces are grouped.
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
        let entry = MeCabCSVRecord {
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
