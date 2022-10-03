use crate::dictionary::{CategoryDefinition, CharacterClass, Morpheme, POSTag};
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
        let mut lattice = Lattice::new();

        let input_bytes = input_text.as_bytes();
        let end = input_bytes.len();
        let mut start = 0;
        let mut offset = 0;
        let mut current = 0;

        let mut nodes_by_position: Vec<Vec<FSTNode>> =
            (0..(end + 1)).into_iter().map(|_| Vec::new()).collect();

        nodes_by_position[0].push(FSTNode {
            cost: 0,
            last_position_index: 0,
            word_length: 0,
            morpheme: self.system_dictionary.bos_eos_morpheme.clone(),
        }); // Add bos node

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
            let current_ch_cls = match self
                .system_dictionary
                .unk_dictionary
                .get_character_class(input_bytes, current)
            {
                Ok(ch_cls) => ch_cls,
                Err(e) => {
                    return Err(format!(
                        "Unknown character cls: `{}`, {}",
                        input_bytes[current],
                        e.description()
                    )
                    .into())
                }
            };

            let current_ch_def = self.system_dictionary.unk_dictionary.invoke_map[&current_ch_cls];

            let is_found =
                self.find_system_dictionary(&query, offset, num_spaces, &mut nodes_by_position);
            if !is_found || current_ch_def.invoke == 1 {
                self.find_unk_dictionary(
                    input_text,
                    offset,
                    num_spaces,
                    &mut nodes_by_position,
                    current_ch_cls,
                    current_ch_def,
                )
            }

            offset += num_spaces as usize;
            offset += utils::uchar::get_next_length(input_bytes[current]);
        }

        Ok(lattice)
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
                let parent = &nodes_by_position[offset][parent_index as usize];

                let space_cost = Self::get_space_penalty(morpheme, num_spaces);
                let word_cost = morpheme.word_cost as i32;
                let cost = parent.cost + word_cost + connection_cost + space_cost;
                let word_length = k.len() as i32;

                let last_position_index = parent.last_position_index + num_spaces + word_length;

                nodes_by_position[last_position_index as usize].push(FSTNode {
                    cost: cost,
                    last_position_index: last_position_index,
                    word_length: word_length,
                    morpheme: morpheme.clone(),
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
        current_ch_cls: CharacterClass,
        current_ch_def: CategoryDefinition,
    ) {
        let text = &input_text[offset..];
        //   int length = internal::groupingUnknownCharacters(
        //       current, end, category, dictionary, charDef->group() == 1);

        //   const nori::protos::Morpheme* morpheme =
        //       &dictionary->getUnkTokens()->morpheme_map().at(category);
        //   const int wordCost = morpheme->word_cost();
        //   auto spaceCost = internal::getSpacePenalty(morpheme, numSpaces);
        //   int connectionCost;
        //   auto parent = internal::selectParent(nodesByPos[offset], morpheme,
        //                                        this->dictionary, connectionCost);

        //   auto lastPositionIndex = parent->lastPositionIndex + numSpaces + length;
        //   auto lastNodeId = nodeId;
        //   auto cost = parent->cost + wordCost + connectionCost + spaceCost;

        //   nodesByPos[lastPositionIndex].emplace_back(
        //       nodeId++, cost, lastPositionIndex, length, morpheme, parent);
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

    fn group_unknown_chars(&self, x: &str) {
        let chars = x.chars().collect::<Vec<char>>();
        if chars.len() == 0 {
            return;
        }

        let current_script = chars[0].script();
        let is_first_common_or_inherited =
            current_script == Script::Common || current_script == Script::Inherited;
    }

    #[inline]
    fn is_punctuation(c: char) -> bool {
        if c as u32 == 4510 { // Hangul Letter Araea
            return true;
        }

        match get_general_category(c) {
            GeneralCategory::SpaceSeparator
            | GeneralCategory::LineSeparator
            | GeneralCategory::ParagraphSeparator
            | GeneralCategory::Control
            | GeneralCategory::Format
            | GeneralCategory::DashPunctuation
            // TODO: find below category
            //
            // | U_START_PUNCTUATION
            // | U_END_PUNCTUATION
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

    word_length: i32,
    last_position_index: i32,

    morpheme: Morpheme,
}
