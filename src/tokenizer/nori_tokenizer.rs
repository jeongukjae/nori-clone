use std::collections::HashSet;
use std::rc::Rc;

use crate::dictionary::{CharacterClass, Morpheme, POSTag};
use crate::{error::Error, SystemDictionary, UserDictionary};
use log::error;
use unicode_general_category::{get_general_category, GeneralCategory};
use unicode_script::{Script, UnicodeScript};

const DEFAULT_TOTAL_COST: i32 = 1000000000;

pub struct NoriTokenizer {
    system_dictionary: SystemDictionary,
    user_dictionary: UserDictionary,
}

impl NoriTokenizer {
    pub fn new(system_dictionary: SystemDictionary, user_dictionary: UserDictionary) -> Self {
        NoriTokenizer {
            system_dictionary,
            user_dictionary,
        }
    }

    pub fn tokenize(&self, input_text: &str) -> Result<Lattice, Error> {
        let input_bytes = input_text.as_bytes();
        let input_bytes_length = input_bytes.len();

        let mut nodes_by_position: Vec<Vec<ACNode>> = (0..(input_bytes_length + 1))
            .into_iter()
            .map(|_| Vec::new())
            .collect();
        let mut founds_position: HashSet<usize> = HashSet::new();

        // Add bos node.
        nodes_by_position[0].push(ACNode {
            space_cost: 0,
            total_cost: 0,
            start_with_space: 0,
            start: 0,
            end: 0,
            morpheme: self.system_dictionary.bos_eos_morpheme.clone(),
            parent_node_index: 0,
        });

        self.find_user_dictionary(input_text, &mut nodes_by_position, &mut founds_position);
        self.find_system_dictionary(input_text, &mut nodes_by_position, &mut founds_position);
        self.find_unknown_words(input_text, &mut nodes_by_position, &founds_position);

        for i in 1..input_bytes_length + 1 {
            if nodes_by_position[i].is_empty() {
                continue;
            }

            for j in 0..nodes_by_position[i].len() {
                let node = &nodes_by_position[i][j];

                let (parent_index, connection_cost) = self
                    .select_parent_node(&nodes_by_position[node.start_with_space], &node.morpheme);
                if let Some(parent) = nodes_by_position[node.start_with_space].get(parent_index) {
                    let parent_cost = parent.total_cost;
                    let total_cost =
                        parent_cost + node.space_cost + connection_cost + node.morpheme.word_cost;

                    nodes_by_position[i][j].total_cost = total_cost;
                    nodes_by_position[i][j].parent_node_index = parent_index;
                }
            }
        }

        // Building output lattice with search result;
        let last_num_spaces = Self::count_space_before_word(input_text, input_bytes.len());
        let end = input_bytes_length - last_num_spaces as usize;
        let (eos_parent_index, _) = self.select_parent_node(
            &nodes_by_position[end],
            &self.system_dictionary.bos_eos_morpheme,
        );

        let mut nodes: Vec<Node> = Vec::new();
        nodes.push(Node {
            surface: "EOS".to_string(),
            morpheme: self.system_dictionary.bos_eos_morpheme.clone(),
            offset: end,
            length: 0,
        });

        let mut current_node = &nodes_by_position[end][eos_parent_index];
        while current_node.end != 0 {
            let node = Node {
                surface: input_text[current_node.start..current_node.end].to_string(),
                morpheme: current_node.morpheme.clone(),
                offset: current_node.start,
                length: current_node.end - current_node.start,
            };
            nodes.push(node);
            current_node =
                &nodes_by_position[current_node.start_with_space][current_node.parent_node_index];
        }

        nodes.push(Node {
            surface: "BOS".to_string(),
            morpheme: current_node.morpheme.clone(),
            offset: 0,
            length: 0,
        });
        nodes.reverse();

        Ok(Lattice { nodes })
    }

    fn find_system_dictionary(
        &self,
        input_text: &str,
        nodes_by_position: &mut [Vec<ACNode>],
        founds_position: &mut HashSet<usize>,
    ) {
        let it = self
            .system_dictionary
            .ahocorasick
            .find_overlapping_iter(input_text);

        for m in it {
            let num_spaces = Self::count_space_before_word(input_text, m.start());
            founds_position.insert(m.start());

            for morpheme in &self.system_dictionary.token_dictionary.morphemes[m.value()] {
                let space_cost = match num_spaces > 0 {
                    true => Self::get_space_cost(morpheme),
                    false => 0,
                };

                nodes_by_position[m.end()].push(ACNode {
                    space_cost,
                    total_cost: DEFAULT_TOTAL_COST,
                    start_with_space: m.start() - num_spaces,
                    start: m.start(),
                    end: m.end(),
                    morpheme: morpheme.clone(),
                    parent_node_index: 0,
                });
            }
        }
    }

    fn find_user_dictionary(
        &self,
        input_text: &str,
        nodes_by_position: &mut [Vec<ACNode>],
        founds_position: &mut HashSet<usize>,
    ) {
        if let Some(user_dict_ahocorasick) = &self.user_dictionary.ahocorasick {
            let it = user_dict_ahocorasick.find_overlapping_iter(input_text);

            // remove completely overlapped case?
            for m in it {
                founds_position.insert(m.start());
                let morpheme = &self.user_dictionary.morphemes[m.value()];

                let num_spaces = Self::count_space_before_word(input_text, m.start());
                let space_cost = match num_spaces > 0 {
                    true => Self::get_space_cost(morpheme),
                    false => 0,
                };

                nodes_by_position[m.end()].push(ACNode {
                    space_cost,
                    total_cost: DEFAULT_TOTAL_COST,
                    start_with_space: m.start() - num_spaces,
                    start: m.start(),
                    end: m.end(),
                    morpheme: self.user_dictionary.morphemes[m.value()].clone(),
                    parent_node_index: 0,
                });
            }
        }
    }

    fn find_unknown_words(
        &self,
        input_text: &str,
        nodes_by_position: &mut [Vec<ACNode>],
        founds_position: &HashSet<usize>,
    ) {
        let mut last_pushed_index = 0;

        for (start, ch) in input_text.char_indices() {
            if last_pushed_index > start {
                continue;
            }

            if founds_position.contains(&start) {
                continue;
            }

            if Self::is_whitespace(ch) {
                continue;
            }

            let char_def = self.system_dictionary.unk_dictionary.get_char_def(ch);

            let (unk_length, ch_cls) =
                self.group_unknown_chars(&input_text[start..], char_def.group == 1);
            let end = start + unk_length;
            let morpheme =
                self.system_dictionary.unk_dictionary.class_morpheme_map[&ch_cls].clone();

            let num_spaces = Self::count_space_before_word(input_text, start);
            let space_cost = match num_spaces > 0 {
                true => Self::get_space_cost(&morpheme),
                false => 0,
            };

            nodes_by_position[end].push(ACNode {
                space_cost,
                total_cost: DEFAULT_TOTAL_COST,
                start_with_space: start - num_spaces,
                start,
                end,
                morpheme,
                parent_node_index: 0,
            });

            last_pushed_index = end;
        }
    }

    fn count_space_before_word(input_text: &str, offset_before: usize) -> usize {
        let mut num_space = 0;
        for c in input_text[..offset_before].chars().rev() {
            if !Self::is_whitespace(c) {
                break;
            }
            num_space += c.len_utf8();
        }
        num_space
    }

    fn get_space_cost(morpheme: &Morpheme) -> i32 {
        if morpheme.pos_tags.is_empty() {
            error!("Morpheme has no pos tags");
            return 0;
        }

        match morpheme.pos_tags[0] {
            POSTag::E | POSTag::J | POSTag::VCP | POSTag::XSA | POSTag::XSN | POSTag::XSV => 3000,
            _ => 0,
        }
    }

    fn select_parent_node(&self, candidates: &[ACNode], morpheme: &Morpheme) -> (usize, i32) {
        if candidates.is_empty() {
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
        let mut min_cost = candidates[0].total_cost + min_connection_cost;

        for (i, candidate) in candidates.iter().enumerate().skip(1) {
            let current_connection_cost =
                self.system_dictionary
                    .connection_cost
                    .get_cost(candidate.morpheme.right_id, morpheme.left_id) as i32;
            let current_cost = candidate.total_cost + current_connection_cost;
            if current_cost < min_cost {
                min_cost = current_cost;
                min_connection_cost = current_connection_cost;
                result_index = i;
            }
        }

        (result_index, min_connection_cost)
    }

    fn group_unknown_chars(&self, x: &str, do_group: bool) -> (usize, CharacterClass) {
        let char_indice_it = x.char_indices();
        let char_len = x.chars().count();
        if char_len == 0 {
            return (0, CharacterClass::DEFAULT);
        }

        let (_, first_ch) = match char_indice_it.clone().next() {
            Some((start, ch)) => (start, ch),
            None => return (0, CharacterClass::DEFAULT),
        };

        let mut result_class = self.system_dictionary.unk_dictionary.get_ch_cls(first_ch);
        let mut result_offset = first_ch.len_utf8();

        if !do_group {
            return (result_offset, result_class);
        }

        let mut first_script = first_ch.script();
        let is_first_common_or_inherited =
            first_script == Script::Common || first_script == Script::Inherited;
        let is_first_punctuation = Self::is_punctuation(first_ch);
        let is_first_digit = first_ch.is_ascii_digit();

        for (_, ch) in char_indice_it.skip(1) {
            let current_script = ch.script();

            let is_common_or_inherited =
                current_script == Script::Common || current_script == Script::Inherited;
            let is_same_script = ((current_script == first_script)
                || is_first_common_or_inherited
                || is_common_or_inherited)
                && !Self::is_whitespace(ch);
            let is_punctuation = Self::is_punctuation(ch);
            let is_digit = ch.is_ascii_digit();

            if !is_same_script
                || (is_first_punctuation != is_punctuation)
                || (is_first_digit != is_digit)
            {
                return (result_offset, result_class);
            }

            if is_first_common_or_inherited && !is_punctuation {
                first_script = current_script;
                result_class = self.system_dictionary.unk_dictionary.get_ch_cls(ch);
            }

            result_offset += ch.len_utf8();
        }

        (result_offset, result_class)
    }

    #[inline]
    fn is_punctuation(c: char) -> bool {
        if c as u32 == 4510 {
            // Hangul Letter Araea
            return true;
        }

        matches!(
            get_general_category(c),
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
                | GeneralCategory::FinalPunctuation
        )
    }

    #[inline]
    fn is_whitespace(c: char) -> bool {
        c.is_whitespace()
    }
}

#[derive(Debug)]
pub struct Lattice {
    pub nodes: Vec<Node>,
}

#[derive(Debug)]
pub struct Node {
    pub surface: String,
    pub morpheme: Rc<Morpheme>,
    pub offset: usize,
    pub length: usize, // byte length
}

#[derive(Debug)]
pub struct ACNode {
    space_cost: i32,
    total_cost: i32,

    start_with_space: usize,
    start: usize,
    end: usize,

    morpheme: Rc<Morpheme>,
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
            ("가나다라 마바", true, (12, CharacterClass::HANGUL)),
            ("'가나다' 마바", true, (1, CharacterClass::SYMBOL)),
            ("淚,", true, (3, CharacterClass::HANJA)),
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
            ("", vec![], vec![]),
            (
                "ABC '텍스트'텍스트 텍스트.",
                vec!["ABC", "'", "텍스트", "'", "텍스트", "텍스트", "."],
                vec![
                    vec![POSTag::SL],
                    vec![POSTag::SY],
                    vec![POSTag::NNG],
                    vec![POSTag::SY],
                    vec![POSTag::NNG],
                    vec![POSTag::NNG],
                    vec![POSTag::SF],
                ],
            ),
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

    fn create_legacy_dictionary() -> SystemDictionary {
        match SystemDictionary::load_from_input_directory("./dictionary/legacy") {
            Ok(d) => d,
            Err(e) => panic!("Failed to load system dictionary: {:?}", e),
        }
    }

    fn create_legacy_dictionary_tokenizer() -> NoriTokenizer {
        let system_dictionary = create_legacy_dictionary();
        let user_dictionary =
            UserDictionary::load(vec![], &system_dictionary.connection_cost.additional).unwrap();

        NoriTokenizer::new(system_dictionary, user_dictionary)
    }
}
