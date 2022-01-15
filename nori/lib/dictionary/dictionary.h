#ifndef __NORI_DICTIONARY_H__
#define __NORI_DICTIONARY_H__

#include <darts.h>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "nori/lib/protos/dictionary.pb.h"

#define NORI_DICT_FILE "dictionary.bin"
#define NORI_DICT_META_FILE "dictionary_meta.pb"
#define NORI_UNK_FILE "unk.pb"
#define NORI_CHAR_FILE "char.pb"
#define NORI_CONNECTION_COST_FILE "costs.pb"

namespace nori {
namespace dictionary {

// User dictionary interface.
// you don't need to access this class directly, because
// nori::dictionary::Dictionary class has userDictionary path.
class UserDictionary {
 public:
  // load dictionary from given path
  absl::Status load(std::string filename, int leftId, int rightId,
                    int rightIdWithJongsung);

  // return trie dictionary.
  const Darts::DoubleArray* getTrie() const { return &trie; }

  // get all morphemes for the trie.
  const std::vector<nori::Morpheme>* getMorphemes() const { return &morphemes; }

 private:
  Darts::DoubleArray trie;
  std::vector<nori::Morpheme> morphemes;
};

// TODO(jeongukjae): change dictionary format.
// it is not good way to save large message in protobuf format.
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
  const nori::TokenInfoDictionary* getTokenDictionary() const {
    return &tokenDictionary;
  }

  // return unk dictionary
  const nori::UnknownDictionary* getUnkDictionary() const {
    return &unkDictionary;
  }

  // return char dictionary
  const nori::CharacterClassDictionary* getCharDictionary() const {
    return &charDictionary;
  }

  // return character calss
  const nori::CharacterClass getCharClass(const char* text) const;

  const nori::CharacterClassDictionary::CategoryDefinition* getCharDef(
      const char* text) const {
    auto cls = this->getCharClass(text);
    return &charDictionary.invokemap().at(cls);
  }

  // return conneciton costs
  const nori::ConnectionCost* getConnectionCosts() const {
    return &connectionCost;
  }

  // return connection costs from right, left ids
  const inline int getConnectionCost(const int rightId,
                                     const int leftId) const {
    return connectionCostData[backwardSize * rightId + leftId];
  }

  const nori::Morpheme* getBosEosMorpheme() const {
    return &this->bosEosMorpheme;
  }
  absl::string_view getBosEosSurface() const { return this->bosEosSurface; }

 private:
  bool initialized = false;
  bool userInitialized = false;

  Darts::DoubleArray trie;
  nori::TokenInfoDictionary tokenDictionary;
  nori::UnknownDictionary unkDictionary;
  nori::CharacterClassDictionary charDictionary;
  nori::ConnectionCost connectionCost;
  UserDictionary userDictionary;

  // for tokenizer
  nori::Morpheme bosEosMorpheme;
  std::string bosEosSurface;

  // from connectionCost
  int backwardSize, forwardSize;
  int connectionCostMax;
  const google::protobuf::int32* connectionCostData;
};

}  // namespace dictionary
}  // namespace nori

#endif  //__NORI_DICTIONARY_H__
