#ifndef __NORI_TOKENIZER_H__
#define __NORI_TOKENIZER_H__

#include <memory>
#include <string>

#include "absl/status/status.h"
#include "nori/dictionary/dictionary.h"
#include "nori/graphviz_visualize.h"

namespace nori {

class Token {};

class Lattice {
 public:
  Lattice() {}

 private:
};

class NoriTokenizer {
 public:
  NoriTokenizer(const nori::dictionary::Dictionary* dictionary,
                size_t maxTrieResults = 1024)
      : dictionary(dictionary), maxTrieResults(maxTrieResults) {}

  // Tokenize input text and save tokenized information to lattice
  absl::Status tokenize(const std::string& text, Lattice& lattice,
                        GraphvizVisualizer* visualizer = nullptr) const;

 private:
  const nori::dictionary::Dictionary* dictionary;
  const size_t maxTrieResults;
};

}  // namespace nori

#endif  // __NORI_TOKENIZER_H__
