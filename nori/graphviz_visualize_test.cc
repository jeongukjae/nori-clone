#include "nori/graphviz_visualize.h"

#include "gtest/gtest.h"

TEST(GraphvizVisualizer, emptyGraph) {
  nori::GraphvizVisualizer visualizer("testGraph", "Helvetica", "BOS", "EOS");

  visualizer.reset();
  visualizer.finish();

  ASSERT_EQ(
      visualizer.str(),
      "digraph testGraph {\n"
      "  graph [fontsize=30 labelloc=\"t\" label=\"\" splines=true "
      "overlap=false rankdir = \"LR\"];\n"
      "  edge [fontname=\"Helvetica\" fontcolor=\"red\" color=\"#606060\"]\n"
      "  node [style=\"filled\" fillcolor=\"#e8e8f0\" shape=\"Mrecord\" "
      "fontname=\"Helvetica\"]\n"
      "  init [style=invis]\n"
      "  init -> 0.0 [label=\"BOS\"]\n"
      "}");
}
