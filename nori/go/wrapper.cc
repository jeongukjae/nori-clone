#include "nori/go/wrapper.h"

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

void freeTokenizer(Dictionary* dictionary, Tokenizer* tokenizer) {
  delete reinterpret_cast<nori::dictionary::Dictionary*>(dictionary);
  delete reinterpret_cast<nori::NoriTokenizer*>(tokenizer);
}

int tokenize(const Tokenizer* tokenizer, const char* str, void** latticeOut) {
  nori::Lattice* lattice = new nori::Lattice(std::string(str));
  *latticeOut = lattice;

  absl::Status status =
      reinterpret_cast<const nori::NoriTokenizer*>(tokenizer)->tokenize(
          *lattice);
  if (!status.ok()) {
    return 1;
  }
  return 0;
}

void freeLattice(const void* lattice) { delete lattice; }
}
