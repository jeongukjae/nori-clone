#include "nori/lib/graphviz_visualize.h"

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

TEST(GraphVisualizer, addNodeAndAddEos) {
  nori::GraphvizVisualizer visualizer("testGraph", "Helvetica", "BOS", "EOS");

  nori::protos::Morpheme fromMopheme, toMorpheme;

  visualizer.reset();
  visualizer.addNode(0, 1, &fromMopheme, 1, 2, &toMorpheme, "abc", 1000, 100,
                     3500);
  visualizer.addEos(1, 2, &toMorpheme);
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
      "  1.2 [label=\"0:0, cost: 3500\"]\n"
      "  0.1 -> 1.2 [label=\"abc 1000, 100 & \"]\n"
      "  eos2 [style=invis]\n"
      "  1.2 -> eos2 [label=\"EOS\"]\n"
      "}");
}
