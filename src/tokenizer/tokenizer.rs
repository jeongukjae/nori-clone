use crate::dictionary::{Morpheme, POSTag};
use crate::CommonPrefix;
use crate::{error::Error, SystemDictionary, UserDictionary};
use fst::IntoStreamer;
use fst::Streamer;
use log::error;

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
        let mut lattice = Lattice::new();

        let input_bytes = input_text.as_bytes();
        let end = input_bytes.len();
        let mut start = 0;
        let mut offset = 0;
        let mut current = 0;

        let mut nodes_by_position: Vec<Vec<FSTNode>> =
            (0..(end + 1)).into_iter().map(|_| Vec::new()).collect();

        nodes_by_position[0].push(FSTNode {
            unique_node_id: 0,
            cost: 0,
            last_position_index: 0,
            word_length: 0,
            morpheme: self.system_dictionary.bos_eos_morpheme.clone(),
        }); // Add bos node
        let mut node_id = 1;

        while start + offset < end {
            current = start + offset;

            if nodes_by_position[current].is_empty() {
                offset += 1;
                continue;
            }

            let mut num_spaces = 0;
            while input_bytes[current].is_ascii_whitespace() {
                current += 1;
                num_spaces += 1;
            }

            if current == end {
                break;
            }

            let query = CommonPrefix::new(input_text, current);

            // TODO: find user dictionary

            let mut system_stream = self.system_dictionary.fst.search(query).into_stream();
            let mut is_found = false;
            while let Some((k, v)) = system_stream.next() {
                is_found = true;

                for morpheme in &self.system_dictionary.token_dictionary.morphemes[v as usize] {
                    let (parent_index, connection_cost) =
                        self.select_parent_node(&nodes_by_position[offset], morpheme);
                    let parent = &nodes_by_position[offset][parent_index as usize];

                    let space_cost = Self::get_space_penalty(morpheme, num_spaces);
                    let word_cost = morpheme.word_cost as i32;
                    let cost = parent.cost + word_cost + connection_cost + space_cost;
                    let word_length = k.len() as i32;

                    let last_position_index = parent.last_position_index + num_spaces + word_length;

                    nodes_by_position[last_position_index as usize].push(FSTNode {
                        cost: cost,
                        unique_node_id: node_id,
                        last_position_index: last_position_index,
                        word_length: word_length,
                        morpheme: morpheme.clone(),
                    });
                }
            }

            if !is_found {
                // TODO: unk
            }

            offset += num_spaces as usize;
            offset += Self::get_next_unicode_char_length(input_bytes[current]);
        }

        Ok(lattice)
    }

    fn get_next_unicode_char_length(c: u8) -> usize {
        if c < 0x80 {
            1
        } else if c < 0xE0 {
            2
        } else if c < 0xF0 {
            3
        } else {
            4
        }
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

    fn select_parent_node(&self, candidates: &Vec<FSTNode>, morpheme: &Morpheme) -> (i32, i32) {
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
                min_cost = current_connection_cost;
                min_connection_cost = current_connection_cost;
                result_index = i as i32;
            }
        }

        (result_index, min_cost)
    }
}

#[derive(Debug)]
pub struct Lattice {
    pub nodes: Vec<Node>,
}

impl Lattice {
    pub fn new() -> Self {
        Lattice { nodes: vec![] }
    }
}

#[derive(Debug)]
pub struct Node {}

#[derive(Debug, Clone)]
pub struct FSTNode {
    cost: i32,
    unique_node_id: i32,

    word_length: i32,
    last_position_index: i32,

    morpheme: Morpheme,
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_get_next_unicode_char_length() {
        assert_eq!(
            NoriTokenizer::get_next_unicode_char_length("$".as_bytes()[0]),
            1
        );
        assert_eq!(
            NoriTokenizer::get_next_unicode_char_length("£".as_bytes()[0]),
            2
        );
        assert_eq!(
            NoriTokenizer::get_next_unicode_char_length("ह".as_bytes()[0]),
            3
        );
        assert_eq!(
            NoriTokenizer::get_next_unicode_char_length("€".as_bytes()[0]),
            3
        );
        assert_eq!(
            NoriTokenizer::get_next_unicode_char_length("한".as_bytes()[0]),
            3
        );
        assert_eq!(
            NoriTokenizer::get_next_unicode_char_length("𐍈".as_bytes()[0]),
            4
        );
    }
}
