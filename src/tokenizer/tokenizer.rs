use crate::dictionary::{CharacterClass, Morpheme, POSTag};
use crate::{error::Error, SystemDictionary, UserDictionary};
use crate::{utils, CommonPrefix};
use fst::IntoStreamer;
use fst::Streamer;
use log::error;
use unicode_general_category::{get_general_category, GeneralCategory};
use unicode_script::{Script, UnicodeScript};

pub struct NoriTokenizer {
    system_dictionary: SystemDictionary,
    user_dictionary: UserDictionary,
}

impl NoriTokenizer {
    pub fn new(system_dictionary: SystemDictionary, user_dictionary: UserDictionary) -> Self {
        NoriTokenizer {
            system_dictionary: system_dictionary,
            user_dictionary: user_dictionary,
        }
    }

    pub fn tokenize(&self, input_text: &str) -> Result<Lattice, Error> {
        let input_bytes = input_text.as_bytes();
        let end = input_bytes.len();
        let mut offset = 0;
        let mut last_num_spaces = 0;

        let mut nodes_by_position: Vec<Vec<FSTNode>> =
            (0..(end + 1)).into_iter().map(|_| Vec::new()).collect();

        // Add bos node.
        nodes_by_position[0].push(FSTNode {
            cost: 0,
            last_position_index: 0,
            word_length: 0,
            num_space_before_node: 0,
            morpheme: self.system_dictionary.bos_eos_morpheme.clone(),
            parent_node_index: 0,
        });

        // Search over input text.
        while offset < end {
            let mut current = offset;
            let mut num_spaces = 0;

            if nodes_by_position[current].is_empty() {
                offset += utils::uchar::get_next_length(input_bytes[current]);
                continue;
            }

            while current < end && input_bytes[current].is_ascii_whitespace() {
                current += 1;
                num_spaces += 1;
            }

            if current == end {
                last_num_spaces = num_spaces;
                break;
            }

            let query = CommonPrefix::new(input_text, current);

            // TODO: find user dictionary

            let current_ch_cls = match self
                .system_dictionary
                .unk_dictionary
                .get_ch_cls_with_vec(input_bytes, current)
            {
                Ok(ch_cls) => ch_cls,
                Err(e) => return Err(format!("Unknown character cls: {}", e.description()).into()),
            };

            let current_ch_def = self.system_dictionary.unk_dictionary.invoke_map[&current_ch_cls];

            let is_found =
                self.find_system_dictionary(&query, offset, num_spaces, &mut nodes_by_position);
            if !is_found || current_ch_def.invoke == 1 {
                match self.find_unk_dictionary(
                    input_text,
                    offset,
                    num_spaces,
                    &mut nodes_by_position,
                ) {
                    Ok(_) => {}
                    Err(e) => return Err(format!("Failed to find unk dictionary: {:?}", e).into()),
                };
            }

            offset += num_spaces as usize;
            offset += utils::uchar::get_next_length(input_bytes[current]);
        }

        // Building output lattice with search result;
        let end = end - last_num_spaces as usize;
        let eos_morpeheme = self.system_dictionary.bos_eos_morpheme.clone();
        let (eos_parent_index, _) =
            self.select_parent_node(&nodes_by_position[end], &eos_morpeheme);

        let mut nodes: Vec<Node> = Vec::new();
        nodes.push(Node {
            surface: "EOS".to_string(),
            morpheme: eos_morpeheme,
            offset: end,
            length: 0,
        });

        let mut current_node = &nodes_by_position[end][eos_parent_index];
        while current_node.last_position_index != 0 {
            let start = (current_node.last_position_index - current_node.word_length) as usize;
            let end = current_node.last_position_index as usize;
            let node = Node {
                surface: input_text[start..end].to_string(),
                morpheme: current_node.morpheme.clone(),
                offset: start,
                length: end - start,
            };
            nodes.push(node);
            let parent_node_inex = start - current_node.num_space_before_node as usize;
            current_node = &nodes_by_position[parent_node_inex][current_node.parent_node_index];
        }

        nodes.push(Node {
            surface: "BOS".to_string(),
            morpheme: current_node.morpheme.clone(),
            offset: 0,
            length: 0,
        });
        nodes.reverse();

        Ok(Lattice { nodes: nodes })
    }

    fn find_system_dictionary(
        &self,
        query: &CommonPrefix,
        offset: usize,
        num_spaces: i32,
        nodes_by_position: &mut Vec<Vec<FSTNode>>,
    ) -> bool {
        let mut system_stream = self.system_dictionary.fst.search(query).into_stream();
        let mut is_found = false;

        while let Some((k, v)) = system_stream.next() {
            is_found = true;
            for morpheme in &self.system_dictionary.token_dictionary.morphemes[v as usize] {
                let (parent_index, connection_cost) =
                    self.select_parent_node(&nodes_by_position[offset], morpheme);
                let parent = &nodes_by_position[offset][parent_index];

                let space_cost = Self::get_space_penalty(morpheme, num_spaces);
                let word_cost = morpheme.word_cost as i32;
                let cost = parent.cost + word_cost + connection_cost + space_cost;
                let word_length = k.len() as i32;

                let last_position_index = parent.last_position_index + num_spaces + word_length;

                nodes_by_position[last_position_index as usize].push(FSTNode {
                    cost: cost,
                    num_space_before_node: num_spaces,
                    last_position_index: last_position_index,
                    word_length: word_length,
                    morpheme: morpheme.clone(),
                    parent_node_index: parent_index,
                });
            }
        }

        is_found
    }

    fn find_unk_dictionary(
        &self,
        input_text: &str,
        offset: usize,
        num_spaces: i32,
        nodes_by_position: &mut Vec<Vec<FSTNode>>,
    ) -> Result<(), Error> {
        let char_def = match self
            .system_dictionary
            .unk_dictionary
            .get_char_def(input_text.as_bytes(), offset)
        {
            Ok(char_def) => char_def,
            Err(e) => return Err(format!("Unknown character def: {}", e.description()).into()),
        };

        let (unk_length, ch_cls) =
            self.group_unknown_chars(&input_text[offset..], char_def.group == 1);
        let morpheme = &self.system_dictionary.unk_dictionary.class_morpheme_map[&ch_cls];
        let (parent_index, connection_cost) =
            self.select_parent_node(&nodes_by_position[offset], morpheme);
        let parent = &nodes_by_position[offset][parent_index];

        let word_cost = morpheme.word_cost as i32;
        let space_cost = Self::get_space_penalty(morpheme, num_spaces);
        let last_position_index = parent.last_position_index + num_spaces + unk_length;

        let cost = parent.cost + word_cost + connection_cost + space_cost;

        nodes_by_position[last_position_index as usize].push(FSTNode {
            cost: cost,
            num_space_before_node: num_spaces,
            last_position_index: last_position_index,
            word_length: unk_length,
            morpheme: morpheme.clone(),
            parent_node_index: parent_index,
        });

        Ok(())
    }

    fn get_space_penalty(morpheme: &Morpheme, num_spaces: i32) -> i32 {
        if num_spaces == 0 {
            return 0;
        }

        if morpheme.pos_tags.len() == 0 {
            error!("Morpheme has no pos tags");
            return 0;
        }

        match morpheme.pos_tags[0] {
            POSTag::E | POSTag::J | POSTag::VCP | POSTag::XSA | POSTag::XSN | POSTag::XSV => 3000,
            _ => 0,
        }
    }

    fn select_parent_node(&self, candidates: &Vec<FSTNode>, morpheme: &Morpheme) -> (usize, i32) {
        if candidates.len() == 0 {
            return (0, 0);
        }

        let mut min_connection_cost =
            self.system_dictionary
                .connection_cost
                .get_cost(candidates[0].morpheme.right_id, morpheme.left_id) as i32;
        if candidates.len() == 1 {
            return (0, min_connection_cost);
        }

        let mut result_index = 0;
        let mut min_cost = candidates[0].cost + min_connection_cost;

        for i in 1..candidates.len() {
            let current_connection_cost = candidates[i].cost
                + self
                    .system_dictionary
                    .connection_cost
                    .get_cost(candidates[i].morpheme.right_id, morpheme.left_id)
                    as i32;
            let current_cost = candidates[i].cost + current_connection_cost;
            if current_cost < min_cost {
                min_cost = candidates[i].cost + current_connection_cost;
                min_connection_cost = current_connection_cost;
                result_index = i;
            }
        }

        (result_index, min_connection_cost)
    }

    fn group_unknown_chars(&self, x: &str, do_group: bool) -> (i32, CharacterClass) {
        if !do_group {
            return (0, CharacterClass::DEFAULT);
        }

        let chars = x.chars().collect::<Vec<char>>();
        let char_len = chars.len();
        if char_len == 0 {
            return (0, CharacterClass::DEFAULT);
        }

        let mut first_script = chars[0].script();
        let is_first_common_or_inherited =
            first_script == Script::Common || first_script == Script::Inherited;
        let is_first_punctuation = Self::is_punctuation(chars[0]);
        let is_first_digit = chars[0].is_digit(10);

        let mut result_class = self.system_dictionary.unk_dictionary.get_ch_cls(chars[0]);
        let mut result_offset = chars[0].len_utf8() as i32;
        let mut char_offset = 1;

        while char_offset < char_len {
            let current_script = chars[char_offset].script();

            let is_common_or_inherited =
                current_script == Script::Common || current_script == Script::Inherited;
            let is_same_script = ((current_script == first_script)
                || is_first_common_or_inherited
                || is_common_or_inherited)
                && !chars[char_offset].is_whitespace();
            let is_punctuation = Self::is_punctuation(chars[char_offset]);
            let is_digit = chars[0].is_digit(10);

            if !is_same_script
                || (is_first_punctuation != is_punctuation)
                || (is_first_digit != is_digit)
            {
                return (result_offset, result_class);
            }

            if is_first_common_or_inherited && !is_punctuation {
                first_script = current_script;
                result_class = self
                    .system_dictionary
                    .unk_dictionary
                    .get_ch_cls(chars[char_offset]);
            }

            result_offset += chars[char_offset].len_utf8() as i32;
            char_offset += 1;
        }

        (result_offset, result_class)
    }

    #[inline]
    fn is_punctuation(c: char) -> bool {
        if c as u32 == 4510 {
            // Hangul Letter Araea
            return true;
        }

        match get_general_category(c) {
            GeneralCategory::SpaceSeparator
            | GeneralCategory::LineSeparator
            | GeneralCategory::ParagraphSeparator
            | GeneralCategory::Control
            | GeneralCategory::Format
            | GeneralCategory::DashPunctuation
            | GeneralCategory::OpenPunctuation
            | GeneralCategory::ClosePunctuation
            | GeneralCategory::ConnectorPunctuation
            | GeneralCategory::OtherPunctuation
            | GeneralCategory::MathSymbol
            | GeneralCategory::CurrencySymbol
            | GeneralCategory::ModifierSymbol
            | GeneralCategory::OtherSymbol
            | GeneralCategory::InitialPunctuation
            | GeneralCategory::FinalPunctuation => true,
            _ => false,
        }
    }
}

#[derive(Debug)]
pub struct Lattice {
    pub nodes: Vec<Node>,
}

#[derive(Debug)]
pub struct Node {
    pub surface: String,
    pub morpheme: Morpheme,
    pub offset: usize,
    pub length: usize, // byte length
}

#[derive(Debug, Clone)]
pub struct FSTNode {
    cost: i32,

    num_space_before_node: i32,
    word_length: i32,
    last_position_index: i32,

    morpheme: Morpheme,

    parent_node_index: usize,
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_is_punctuation() {
        let expected_punctuations = vec![
            '‘', '’', '“', '”', '⁽', '］', '\'', '…', 'ᆞ', '⸆', '⸻', '⸺', '⸿',
        ];
        for c in expected_punctuations {
            assert!(
                NoriTokenizer::is_punctuation(c),
                "`{}` is not a punctuation. category: {:?}",
                c,
                get_general_category(c)
            );
        }
    }

    #[test]
    fn test_group_unknown_chars() {
        let tests = vec![
            ("", true, (0, CharacterClass::DEFAULT)),
            ("abcd efjk", true, (4, CharacterClass::ALPHA)),
            ("가나다라 마바", true, (12, CharacterClass::HANGUL)),
        ];

        let tokenizer = create_legacy_dictionary_tokenizer();

        for (input, do_group, expected) in tests {
            let (actual_offset, actual_class) = tokenizer.group_unknown_chars(input, do_group);
            assert_eq!(expected, (actual_offset, actual_class));
        }
    }

    #[test]
    fn test_tokenizer_can_tokenize_sentences() {
        // NOTE: we test it without bos/eos nodes.
        let tests = vec![
            (
                "화학 이외의 것",
                vec!["화학", "이외", "의", "것"],
                vec![
                    vec![POSTag::NNG],
                    vec![POSTag::NNG],
                    vec![POSTag::J],
                    vec![POSTag::NNB],
                ],
            ),
            (
                "화학             이외의              것  ",
                vec!["화학", "이외", "의", "것"],
                vec![
                    vec![POSTag::NNG],
                    vec![POSTag::NNG],
                    vec![POSTag::J],
                    vec![POSTag::NNB],
                ],
            ),
            (
                "가락지나물은 한국, 중국, 일본",
                vec!["가락지나물", "은", "한국", ",", "중국", ",", "일본"],
                vec![
                    vec![POSTag::NNG],
                    vec![POSTag::J],
                    vec![POSTag::NNP],
                    vec![POSTag::SC],
                    vec![POSTag::NNP],
                    vec![POSTag::SC],
                    vec![POSTag::NNP],
                ],
            ),
            // TODO: this test fails because of the bug in the cost estimation.
            //
            // (
            //     "10.1 인치 모니터",
            //     vec!["10", ".", "1", "인치", "모니터"],
            //     vec![
            //         vec![POSTag::SN],
            //         vec![POSTag::SY],
            //         vec![POSTag::SN],
            //         vec![POSTag::NNBC],
            //         vec![POSTag::NNG],
            //     ],
            // ),
            ("εἰμί", vec!["εἰμί"], vec![vec![POSTag::SL]]),
        ];

        let tokenizer = create_legacy_dictionary_tokenizer();

        for (input, expected_surface, expected_pos) in tests {
            let lattice = tokenizer.tokenize(input);
            assert!(lattice.is_ok(), "input: `{}`", input);
            let lattice = lattice.unwrap();

            // remove bos/eos nodes.
            let nodes = &lattice.nodes[1..lattice.nodes.len() - 1];

            assert_eq!(
                expected_surface.len(),
                nodes.len(),
                "input: `{}`, result: {:#?}",
                input,
                lattice
            );

            for (i, token) in nodes.iter().enumerate() {
                assert_eq!(
                    expected_surface[i], token.surface,
                    "input: `{}`, result: {:#?}",
                    input, lattice
                );
                assert_eq!(
                    expected_pos[i], token.morpheme.pos_tags,
                    "input: `{}`, result: {:#?}",
                    input, lattice
                );
            }
        }
    }

    fn create_legacy_dictionary_tokenizer() -> NoriTokenizer {
        let system_dictionary =
            match SystemDictionary::load_from_input_directory("./dictionary/legacy") {
                Ok(d) => d,
                Err(e) => panic!("Failed to load system dictionary: {:?}", e),
            };
        let user_dictionary = UserDictionary {};
        NoriTokenizer::new(system_dictionary, user_dictionary)
    }
}
