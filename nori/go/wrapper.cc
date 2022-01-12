#include "nori/go/wrapper.h"

#include "nori/lib/dictionary/dictionary.h"
#include "nori/lib/tokenizer.h"

int initializeTokenizer(const char* dictionaryPath,
                        const char* userDictionaryPath, void** dictionaryOutput,
                        void** tokenizerOutput) {
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
  nori::NoriTokenizer* tokenizer = new nori::NoriTokenizer(
      static_cast<const nori::dictionary::Dictionary*>(dictionary));
  *tokenizerOutput = tokenizer;
  *dictionaryOutput = dictionary;

  return 0;
}

void freeTokenizer(void* dictionary, void* tokenizer) {
  delete dictionary;
  delete tokenizer;
}

int tokenize(const void* tokenizer, const char* str, void** latticeOut) {
  nori::Lattice* lattice = new nori::Lattice(std::string(str));
  *latticeOut = lattice;

  absl::Status status =
      static_cast<const nori::NoriTokenizer*>(tokenizer)->tokenize(*lattice);
  if (!status.ok()) {
    return 1;
  }
  return 0;
}

void freeLattice(const void* lattice) { delete lattice; }
