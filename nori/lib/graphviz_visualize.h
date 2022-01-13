#ifndef __NORI_GRAPHVIZ_VISUALIZE_H__
#define __NORI_GRAPHVIZ_VISUALIZE_H__

#include <string>

#include "nori/lib/protos/dictionary.pb.h"

namespace nori {

// Visualizing tokenizer path using dotfile format.
//
// you can convert the output of GraphvizVisualizer::str to png using below
// command.
//   dot -Tpng path-to-dotfile > outfile.png
class GraphvizVisualizer {
 public:
  GraphvizVisualizer(const std::string graphName = "nori",
                     const std::string fontName = "Helvetica",
                     const std::string bosLabel = "BOS",
                     const std::string eosLabel = "EOS")
      : graphName(graphName),
        fontName(fontName),
        bosLabel(bosLabel),
        eosLabel(eosLabel) {}

  // clear internal states
  void reset();

  // add node path
  void addNode(size_t fromIndex, size_t fromNodeId,
               const nori::Morpheme* fromMorpheme, size_t toIndex,
               size_t toNodeId, const nori::Morpheme* toMorpheme,
               const std::string stringForm, int wordCost, int connectionCost,
               int cost);

  // add final node path
  void addEos(size_t fromIndex, size_t fromNodeId,
              const nori::Morpheme* fromMorpheme);

  // wrap up
  void finish();

  // get final output data
  const std::string str() const { return data; }

 private:
  const std::string graphName, fontName, bosLabel, eosLabel;
  std::string data;
};

}  // namespace nori

#endif  // __NORI_GRAPHVIZ_VISUALIZE_H__
