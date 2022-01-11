#ifndef __NORI_TOKENIZER_H__
#define __NORI_TOKENIZER_H__

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "nori/lib/dictionary/dictionary.h"
#include "nori/lib/graphviz_visualize.h"

namespace nori {

struct Token {
  const absl::string_view surface;
  const nori::Morpheme* morpheme;
  const size_t offset;
  const size_t length;

  Token(const absl::string_view surface, const nori::Morpheme* morpheme,
        const size_t offset, const size_t length)
      : surface(surface), morpheme(morpheme), offset(offset), length(length) {}
};

struct Lattice {
 public:
  Lattice() {}
  Lattice(std::string sentence) : sentence(sentence) {}

  void clear() {
    clearState();
    sentence.clear();
  }
  void clearState() { tokens.clear(); }
  void setSentence(std::string sentence) { this->sentence = sentence; }
  const absl::string_view getSentence() const { return this->sentence; }
  const std::vector<Token>* getTokens() const { return &this->tokens; }
  std::vector<Token>* getMutableTokens() { return &this->tokens; }

 private:
  std::string sentence;
  std::vector<Token> tokens;
};

class NoriTokenizer {
 public:
  NoriTokenizer(const nori::dictionary::Dictionary* dictionary,
                size_t maxTrieResults = 1024)
      : dictionary(dictionary), maxTrieResults(maxTrieResults) {}

  // Tokenize input text and save tokenized information to lattice
  absl::Status tokenize(Lattice& lattice,
                        GraphvizVisualizer* visualizer = nullptr) const;

 private:
  const nori::dictionary::Dictionary* dictionary;
  const size_t maxTrieResults;
};

}  // namespace nori

#endif  // __NORI_TOKENIZER_H__