use std::collections::HashMap;

use crate::dictionary::POSTag;

pub struct GraphViz {
    data: String,
    nodes: HashMap<u32, NodePoint>,
    edges: HashMap<(u32, u32), NodeEdgeInfo>,
}

#[derive(Clone)]
pub struct NodePoint {
    pub text_index: usize,
    pub node_id: u32,
    pub is_optimal: bool,
    pub is_invisible: bool,
    pub total_cost: i32,
}

impl NodePoint {
    pub fn new(text_index: usize, node_id: u32, total_cost: i32) -> NodePoint {
        NodePoint {
            text_index,
            node_id,
            total_cost,
            is_invisible: false,
            is_optimal: false,
        }
    }

    pub fn new_invisible(text_index: usize, node_id: u32, total_cost: i32) -> NodePoint {
        NodePoint {
            text_index,
            node_id,
            total_cost,
            is_invisible: true,
            is_optimal: false,
        }
    }
}

#[derive(Clone)]
pub struct NodeEdgeInfo {
    pub to_right_id: u16,
    pub to_left_id: u16,
    pub to_word_cost: i32,
    pub pos_tags: Vec<POSTag>,
    pub surface: String,
}

impl Default for GraphViz {
    fn default() -> Self {
        Self::new()
    }
}

impl GraphViz {
    pub fn new() -> GraphViz {
        GraphViz {
            data: String::from(""),
            nodes: HashMap::new(),
            edges: HashMap::new(),
        }
    }

    pub fn add_node(&mut self, from: &NodePoint, to: &NodePoint, node_edge_info: &NodeEdgeInfo) {
        self.nodes.entry(from.node_id).or_insert_with(|| from.clone());
        self.nodes.entry(to.node_id).or_insert_with(|| to.clone());
        self.edges
            .insert((from.node_id, to.node_id), node_edge_info.clone());
    }

    pub fn set_optimal(&mut self, node_id: u32) {
        if let Some(node) = self.nodes.get_mut(&node_id) {
            node.is_optimal = true;
        }
    }

    pub fn finalize(&mut self) {
        let mut data = String::from("");
        data.push_str("digraph nori_clone {\n");
        data.push_str("\tgraph [fontsize=30 labelloc=\"t\" label=\"\" splines=true overlap=false rankdir = \"LR\"];\n");
        data.push_str("\tedge [fontname=\"Helvetica\" fontcolor=\"red\" color=\"#606060\"]\n");
        data.push_str("\tnode [style=\"filled\" fillcolor=\"#e8e8f0\" shape=\"Mrecord\" fontname=\"Helvetica\"]\n");
        data.push_str("\tinit [style=invis]\n");
        data.push_str("\tinit -> 0.0 [label=\"BOS\", fontcolor=\"#7edb79\", color=\"#7edb79\", penwidth=\"3\"]\n");

        for (_, node) in self.nodes.iter() {
            let surface = Self::to_label(node);
            if node.is_invisible {
                data.push_str(&format!("\t{} [style=invis]\n", surface));
                continue;
            }

            if node.is_optimal {
                data.push_str(&format!(
                    "\t{} [label=\"total cost: {}\", fillcolor=\"#7edb79\"]\n",
                    surface, node.total_cost
                ));
            } else {
                data.push_str(&format!(
                    "\t{} [label=\"total cost: {}\"]\n",
                    surface, node.total_cost
                ));
            }
        }

        for ((from, to), edge) in self.edges.iter() {
            let from_node = self.nodes.get(from).unwrap();
            let to_node = self.nodes.get(to).unwrap();
            let from_label = Self::to_label(from_node);
            let to_label = Self::to_label(to_node);

            let mut arc_label = format!(
                "'{}', left: {}, right: {}, word cost: {}, pos tag: {}",
                edge.surface,
                edge.to_left_id,
                edge.to_right_id,
                edge.to_word_cost,
                Self::stringify_pos_tags(&edge.pos_tags),
            );
            if edge.surface == "EOS" {
                arc_label = String::from("EOS");
            }

            if from_node.is_optimal && to_node.is_optimal {
                data.push_str(&format!(
                    "\t{} -> {} [label=\"{}\", fontcolor=\"#7edb79\", color=\"#7edb79\", penwidth=\"3\"]\n",
                    from_label, to_label, arc_label
                ));
            } else {
                data.push_str(&format!(
                    "\t{} -> {} [label=\"{}\"]\n",
                    from_label, to_label, arc_label
                ));
            }
        }

        data.push_str("}\n");

        self.data = data;
    }

    pub fn to_dot(&self) -> String {
        self.data.clone()
    }

    fn to_label(node: &NodePoint) -> String {
        format!("{}.{}", node.text_index, node.node_id)
    }

    fn stringify_pos_tags(postag: &Vec<POSTag>) -> String {
        if postag.is_empty() {
            return String::from("None");
        }

        let mut result = format!("{:?}", postag[0]);
        for tag in postag.iter().skip(1) {
            result.push_str(&format!("+{:?},", tag));
        }
        result
    }
}
