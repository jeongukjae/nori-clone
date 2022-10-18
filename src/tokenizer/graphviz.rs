use crate::dictionary::POSTag;

#[derive(Debug, Clone)]
pub struct GraphViz {
    data: String,
}

pub struct NodePoint {
    pub text_index: usize,
    pub node_id: u32,
}

pub struct NodeEdgeInfo {
    pub to_right_id: u16,
    pub to_left_id: u16,
    pub to_word_cost: i32,
    pub pos_tags: Vec<POSTag>,
    pub surface: String,
    pub connection_cost: i32,
    pub total_cost: i32,
}

impl Default for GraphViz {
    fn default() -> Self {
        Self::new()
    }
}

impl GraphViz {
    pub fn new() -> GraphViz {
        let mut data = String::from("");
        data.push_str("digraph nori_clone {\n");
        data.push_str("\tgraph [fontsize=30 labelloc=\"t\" label=\"\" splines=true overlap=false rankdir = \"LR\"];\n");
        data.push_str("\tedge [fontname=\"Helvetica\" fontcolor=\"red\" color=\"#606060\"]\n");
        data.push_str("\tnode [style=\"filled\" fillcolor=\"#e8e8f0\" shape=\"Mrecord\" fontname=\"Helvetica\"]\n");
        data.push_str("\tinit [style=invis]\n");
        data.push_str("\tinit -> 0.0 [label=\"BOS\"]\n");

        GraphViz { data }
    }

    pub fn add_node(&mut self, from: &NodePoint, to: &NodePoint, node_edge_info: &NodeEdgeInfo) {
        let from_label = Self::to_label(from);
        let to_label = Self::to_label(to);

        let arc_label = format!(
            "'{}', left: {}, right: {}, word cost: {}, pos tag: {}",
            node_edge_info.surface,
            node_edge_info.to_left_id,
            node_edge_info.to_right_id,
            node_edge_info.to_word_cost,
            Self::stringify_pos_tags(&node_edge_info.pos_tags),
        );
        let node_label = format!(
            "conn: {}, total cost: {}",
            node_edge_info.connection_cost, node_edge_info.total_cost,
        );

        self.data
            .push_str(&format!("\t{} [label=\"{}\"]\n", to_label, node_label));
        self.data.push_str(&format!(
            "\t{} -> {} [label=\"{}\"]\n\n",
            from_label, to_label, arc_label
        ));
    }

    pub fn add_eos(&mut self, from: &NodePoint) {
        let from_label = Self::to_label(from);
        let to_label = "eos";

        self.data
            .push_str(&format!("\t{} [style=invis]\n", to_label));
        self.data.push_str(&format!(
            "\t{} -> {} [label=\"EOS\"]\n",
            from_label, to_label
        ));
    }

    pub fn finalize(&mut self) {
        self.data.push('}');
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
