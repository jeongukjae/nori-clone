#include "nori/lib/graphviz_visualize.h"

#include "absl/strings/str_cat.h"
#include "nori/lib/protos/dictionary.pb.h"

namespace nori {

namespace internal {

inline std::string getNodeLabel(size_t position, size_t nodeId) {
  return absl::StrCat(position, ".", nodeId);
}

std::string getPosTagString(const nori::protos::Morpheme* morpheme) {
  if (morpheme == nullptr) {
    return "";
  }

  std::string result = "";
  auto size = morpheme->pos_tags_size();
  for (int i = 0; i < size; i++) {
    if (i != size - 1)
      absl::StrAppend(&result, nori::protos::POSTag_Name(morpheme->pos_tags(i)),
                      "+");
    else
      absl::StrAppend(&result,
                      nori::protos::POSTag_Name(morpheme->pos_tags(i)));
  }

  return result;
}

}  // namespace internal

void GraphvizVisualizer::reset() {
  this->data = absl::StrCat(
      "digraph ", this->graphName,
      " {\n"
      "  graph [fontsize=30 labelloc=\"t\" label=\"\" splines=true "
      "overlap=false rankdir = \"LR\"];\n"
      "  edge [fontname=\"",
      this->fontName,
      "\" fontcolor=\"red\" color=\"#606060\"]\n"
      "  node [style=\"filled\" fillcolor=\"#e8e8f0\" shape=\"Mrecord\" "
      "fontname=\"",
      this->fontName,
      "\"]\n"
      "  init [style=invis]\n"
      "  init -> 0.0 [label=\"",
      this->bosLabel, "\"]\n");
}

void GraphvizVisualizer::addNode(size_t fromIndex, size_t fromNodeId,
                                 const nori::protos::Morpheme* fromMorpheme,
                                 size_t toIndex, size_t toNodeId,
                                 const nori::protos::Morpheme* toMorpheme,
                                 const std::string stringForm, int wordCost,
                                 int connectionCost, int cost) {
  auto fromNodeLabel = internal::getNodeLabel(fromIndex, fromNodeId);
  auto toNodeLabel = internal::getNodeLabel(toIndex, toNodeId);

  // node ref
  absl::StrAppend(&this->data, "  ", toNodeLabel, " [label=\"",
                  toMorpheme->left_id(), ":", toMorpheme->right_id(),
                  ", cost: ", cost, "\"]\n");
  // arc
  absl::StrAppend(&this->data, "  ", fromNodeLabel, " -> ", toNodeLabel,
                  " [label=\"", stringForm, " ", wordCost, ", ", connectionCost,
                  " & ", internal::getPosTagString(toMorpheme), "\"]\n");
}

void GraphvizVisualizer::addEos(size_t fromIndex, size_t fromNodeId,
                                const nori::protos::Morpheme* fromMorpheme) {
  auto fromNodeLabel = internal::getNodeLabel(fromIndex, fromNodeId);
  auto toNodeLabel = absl::StrCat("eos", fromNodeId);

  absl::StrAppend(&this->data, "  ", toNodeLabel, " [style=invis]\n");
  absl::StrAppend(&this->data, "  ", fromNodeLabel, " -> ", toNodeLabel,
                  " [label=\"", eosLabel, "\"]\n");
}

void GraphvizVisualizer::finish() { absl::StrAppend(&this->data, "}"); }

}  // namespace nori
