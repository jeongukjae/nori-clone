#include "nori/c/c_api.h"

#include "nori/lib/dictionary/dictionary.h"
#include "nori/lib/tokenizer.h"

extern "C" {

int initializeTokenizer(const char* dictionaryPath,
                        const char* userDictionaryPath,
                        Dictionary** dictionaryOutput,
                        Tokenizer** tokenizerOutput) {
  nori::dictionary::Dictionary* dictionary = new nori::dictionary::Dictionary();
  absl::Status status = dictionary->loadPrebuilt(std::string(dictionaryPath));
  if (!status.ok()) {
    delete dictionary;
    return 1;
  }

  if (userDictionaryPath != NULL) {
    status = dictionary->loadUser(std::string(userDictionaryPath));
    if (!status.ok()) {
      delete dictionary;
      return 2;
    }
  }
  nori::NoriTokenizer* tokenizer = new nori::NoriTokenizer(dictionary);
  *tokenizerOutput = reinterpret_cast<Tokenizer*>(tokenizer);
  *dictionaryOutput = reinterpret_cast<Dictionary*>(dictionary);

  return 0;
}

void freeTokenizer(const Dictionary* dictionary, const Tokenizer* tokenizer) {
  delete reinterpret_cast<const nori::dictionary::Dictionary*>(dictionary);
  delete reinterpret_cast<const nori::NoriTokenizer*>(tokenizer);
}

int tokenize(const Tokenizer* rawTokenizer, const char* str,
             Lattice** latticeOut) {
  auto tokenizer = reinterpret_cast<const nori::NoriTokenizer*>(rawTokenizer);
  nori::Lattice lattice;
  auto status = lattice.setSentence(
      std::string(str), tokenizer->getDictionary()->getNormalizer());
  if (!status.ok()) {
    return 1;
  }

  status = tokenizer->tokenize(lattice);
  if (!status.ok()) {
    return 2;
  }

  *latticeOut = new Lattice;
  (*latticeOut)->sentence = new char[lattice.getSentence().size() + 1]{
      0,
  };
  std::memcpy((*latticeOut)->sentence, lattice.getSentence().data(),
              lattice.getSentence().size());
  int tokenSize = lattice.getTokens()->size();
  (*latticeOut)->tokenLength = tokenSize;
  (*latticeOut)->tokens = new Token[tokenSize];

  for (int i = 1; i < tokenSize - 1; i++) {
    auto* token = ((*latticeOut)->tokens + i);
    auto& ccToken = lattice.getTokens()->at(i);
    token->offset = ccToken.offset;
    token->length = ccToken.length;
    token->morpheme.leftId = ccToken.morpheme->left_id();
    token->morpheme.rightId = ccToken.morpheme->right_id();
    token->morpheme.wordCost = ccToken.morpheme->word_cost();
    token->morpheme.posType = ccToken.morpheme->pos_type();

    int posTagLength = ccToken.morpheme->pos_tags().size();
    token->morpheme.posTagLength = posTagLength;
    token->morpheme.posTag = new int[posTagLength];
    for (int j = 0; j < posTagLength; j++) {
      token->morpheme.posTag[j] = ccToken.morpheme->pos_tags(j);
    }

    int exprLength = ccToken.morpheme->expression_size();
    token->morpheme.exprLength = exprLength;
    if (exprLength != 0) {
      token->morpheme.exprSurface = new const char*[exprLength];
      token->morpheme.exprPosTag = new int[exprLength];
      for (int j = 0; j < exprLength; j++) {
        token->morpheme.exprSurface[j] =
            ccToken.morpheme->expression(j).surface().data();
        token->morpheme.exprPosTag[j] =
            ccToken.morpheme->expression(j).pos_tag();
      }
    }
  }

  return 0;
}

void freeLattice(const Lattice* lattice) {
  // Except bos/eos
  for (int i = 1; i < lattice->tokenLength - 1; i++) {
    delete[] lattice->tokens[i].morpheme.posTag;
    if (lattice->tokens[i].morpheme.exprLength != 0) {
      delete[] lattice->tokens[i].morpheme.exprPosTag;
      delete[] lattice->tokens[i].morpheme.exprSurface;
    }
  }

  delete[] lattice->tokens;
  delete[] lattice->sentence;
  delete lattice;
}
}
