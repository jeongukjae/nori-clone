#ifndef __NORI_GRAPHVIZ_VISUALIZE_H__
#define __NORI_GRAPHVIZ_VISUALIZE_H__

#include <string>

#include "nori/lib/protos/dictionary.pb.h"

namespace nori {

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

  void reset();
  void addNode(size_t fromIndex, size_t fromNodeId,
               const nori::Morpheme* fromMorpheme, size_t toIndex,
               size_t toNodeId, const nori::Morpheme* toMorpheme,
               const std::string stringForm, int wordCost, int connectionCost,
               int cost);
  void addEos(size_t fromIndex, size_t fromNodeId,
              const nori::Morpheme* fromMorpheme);
  void finish();

  const std::string str() const { return data; }

 private:
  const std::string graphName, fontName, bosLabel, eosLabel;
  std::string data;
};

}  // namespace nori

#endif  // __NORI_GRAPHVIZ_VISUALIZE_H__
