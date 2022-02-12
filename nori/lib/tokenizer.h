#ifndef __NORI_TOKENIZER_H__
#define __NORI_TOKENIZER_H__

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "nori/lib/dictionary/dictionary.h"
#include "nori/lib/graphviz_visualize.h"
#include "nori/lib/utils.h"

namespace nori {

// Token output of nori::Lattice
//
// surface is absl::string_view type because original input data will be stored
// in nori::Lattice::sentence. We don't need additional copy.
//
// You can access all sub-tokens of the compound token via
// nori::Token::morpheme::expression.
struct Token {
  const absl::string_view surface;
  const nori::protos::Morpheme* morpheme;
  const size_t offset;
  const size_t length;

  Token(const absl::string_view surface, const nori::protos::Morpheme* morpheme,
        const size_t offset, const size_t length)
      : surface(surface), morpheme(morpheme), offset(offset), length(length) {}
};

// Lattice struct.
//
// You have to initialize this struct and pass to tokenizer method to get
// output.
struct Lattice {
 private:
  std::string sentence;
  std::vector<Token> tokens;

 public:
  Lattice() {}

  // clear internal states
  void clear() {
    clearState();
    sentence.clear();
  }

  // clear only tokens
  void clearState() { tokens.clear(); }

  // set sentence
  absl::Status setSentence(std::string sentence,
                           const dictionary::Normalizer* normalizer) {
    return normalizer->normalize(sentence, this->sentence);
  }

  // get sentence
  const absl::string_view getSentence() const { return this->sentence; }

  // get output tokens
  const std::vector<Token>* getTokens() const { return &this->tokens; }

  // get mutable tokens.
  // I recommend not to call this method. This method is added for using inside
  // of nori::Tokenizer
  std::vector<Token>* getMutableTokens() { return &this->tokens; }
};

// Tokenizer class
class NoriTokenizer {
 public:
  NoriTokenizer(const nori::dictionary::Dictionary* dictionary,
                size_t maxTrieResults = 1024)
      : dictionary(dictionary), maxTrieResults(maxTrieResults) {}

  // Tokenize input text and save tokenized information to lattice
  absl::Status tokenize(Lattice& lattice,
                        GraphvizVisualizer* visualizer = nullptr) const;

  const nori::dictionary::Dictionary* getDictionary() const { return dictionary; }

 private:
  const nori::dictionary::Dictionary* dictionary;
  const size_t maxTrieResults;
};

}  // namespace nori

#endif  // __NORI_TOKENIZER_H__
