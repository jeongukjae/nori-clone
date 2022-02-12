#ifndef __NORI_DICTIONARY_H__
#define __NORI_DICTIONARY_H__

#include <darts.h>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "nori/lib/protos/dictionary.pb.h"
#include "nori/lib/utils.h"

#define NORI_DICT_FILE "dictionary.bin"
#define NORI_DICT_META_FILE "dictionary_meta.pb"
#define NORI_UNK_FILE "unk.pb"
#define NORI_CHAR_FILE "char.pb"
#define NORI_CONNECTION_COST_FILE "costs.pb"

namespace nori {
namespace dictionary {

// User dictionary interface.
// you don't need to access this class directly, because
// nori::dictionary::Dictionary class has userDictionary field.
class UserDictionary {
 public:
  // load dictionary from given path
  absl::Status load(std::string filename, int leftId, int rightId,
                    int rightId_T, int rightId_F);

  // return trie dictionary.
  const Darts::DoubleArray* getTrie() const { return &trie; }

  // get all morphemes for the trie.
  const std::vector<nori::protos::Morpheme>* getMorphemes() const {
    return &morphemes;
  }

 private:
  Darts::DoubleArray trie;
  std::vector<nori::protos::Morpheme> morphemes;
};

class Normalizer {
 public:
  Normalizer() : doNormalize(false) {}

  void setDoNormalize(bool doNormalize, const std::string normalizationForm) {
    this->doNormalize = doNormalize;
    this->normalizationForm = normalizationForm;
  }

  absl::Status normalize(const std::string in, std::string& out) const {
    if (doNormalize)
      return utils::internal::normalizeUTF8(in, out, normalizationForm);

    out.assign(in);
    return absl::OkStatus();
  }

 private:
  bool doNormalize;
  std::string normalizationForm;
};

class Dictionary {
 public:
  // load prebuilt dictionary from given path
  absl::Status loadPrebuilt(std::string path);

  // load user dictionary from given path
  absl::Status loadUser(std::string filename);

  // return is initialized
  bool isInitialized() const { return initialized; }

  // return is initialized
  bool isUserInitialized() const { return userInitialized; }

  // return trie dictionary
  const Darts::DoubleArray* getTrie() const { return &trie; }

  // return user dictionary
  const UserDictionary* getUserDict() const { return &userDictionary; }

  // return token dictionary
  const nori::protos::Tokens* getTokens() const { return &dictionary.tokens(); }

  // return unk dictionary
  const nori::protos::UnknownTokens* getUnkTokens() const {
    return &dictionary.unknown_tokens();
  }

  // return character calss
  const nori::protos::CharacterClass getCharClass(const char* text) const;

  const nori::protos::UnknownTokens::CategoryDefinition* getCharDef(
      const char* text) const {
    auto cls = this->getCharClass(text);
    return &dictionary.unknown_tokens().invoke_map().at(cls);
  }

  // return conneciton costs
  const nori::protos::ConnectionCost* getConnectionCosts() const {
    return &dictionary.connection_cost();
  }

  // return connection costs from right, left ids
  const inline int getConnectionCost(const int rightId,
                                     const int leftId) const {
    return connectionCostData[backwardSize * rightId + leftId];
  }

  const nori::protos::Morpheme* getBosEosMorpheme() const {
    return &this->bosEosMorpheme;
  }

  absl::string_view getBosEosSurface() const { return this->bosEosSurface; }

  const Normalizer* getNormalizer() const { return &normalizer; }

 private:
  bool initialized = false;
  bool userInitialized = false;

  Darts::DoubleArray trie;
  nori::protos::Dictionary dictionary;
  Normalizer normalizer;
  UserDictionary userDictionary;

  // for tokenizer
  nori::protos::Morpheme bosEosMorpheme;
  std::string bosEosSurface;

  // from connectionCost
  int backwardSize, forwardSize;
  int connectionCostMax;
  const google::protobuf::int32* connectionCostData;
};

}  // namespace dictionary
}  // namespace nori

#endif  //__NORI_DICTIONARY_H__
