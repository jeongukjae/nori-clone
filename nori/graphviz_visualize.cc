#include "nori/graphviz_visualize.h"

namespace nori {

void GraphvizVisualizer::reset() {
  this->data =
      ("digraph " + this->graphName +
       " {\n"
       "  graph [fontsize=30 labelloc=\"t\" label=\"\" splines=true "
       "overlap=false rankdir = \"LR\"];\n"
       "  edge [fontname=\"" +
       this->fontName +
       "\" fontcolor=\"red\" color=\"#606060\"]\n"
       "  node [style=\"filled\" fillcolor=\"#e8e8f0\" shape=\"Mrecord\" "
       "fontname=\"" +
       this->fontName +
       "\"]\n"
       "  init [style=invis]\n"
       "  init -> 0.0 [label=\"" +
       this->bosLabel + "\"]\n");
}

void GraphvizVisualizer::finish() { this->data += "}"; }

}  // namespace nori
