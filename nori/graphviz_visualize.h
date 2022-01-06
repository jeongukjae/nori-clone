#ifndef __NORI_GRAPHVIZ_VISUALIZE_H__
#define __NORI_GRAPHVIZ_VISUALIZE_H__

#include <string>

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
  void addNode();
  void finish();

  const std::string str() const { return data; }

 private:
  const std::string graphName, fontName, bosLabel, eosLabel;
  std::string data;
};

}  // namespace nori

#endif  // __NORI_GRAPHVIZ_VISUALIZE_H__
