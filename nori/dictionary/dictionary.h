#ifndef __NORI_DICTIONARY_H__
#define __NORI_DICTIONARY_H__

#include <darts.h>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "nori/protos/dictionary.pb.h"

#define NORI_DICT_FILE "dictionary.bin"
#define NORI_DICT_META_FILE "dictionary_meta.pb"
#define NORI_UNK_FILE "unk.pb"
#define NORI_CHAR_FILE "char.pb"
#define NORI_CONNECTION_COST_FILE "costs.pb"

namespace nori {
namespace dictionary {

class Dictionary {
 public:
  // load dictionary from given path
  absl::Status load(absl::string_view path);

  // return trie dictionary
  const Darts::DoubleArray* getTrie() const { return &trie; }

  // return token dictionary
  const nori::ListDictionary* getTokenDictionary() const {
    return &tokenDictionary;
  }

  // return unk dictionary
  const nori::Dictionary* getUnkDictionary() const { return &unkDictionary; }

  // return char dictionary
  const nori::CharacterClassDictionary* getCharDictionary() const {
    return &charDictionary;
  }

  // return character calss
  const nori::CharacterClass getCharClass(const char* text) const;

  // return conneciton costs
  const nori::ConnectionCost* getConnectionCosts() const {
    return &connectionCost;
  }

  // return connection costs from right, left ids
  const int getCost(int rightId, int leftId) const;

 private:
  Darts::DoubleArray trie;
  nori::ListDictionary tokenDictionary;
  nori::Dictionary unkDictionary;
  nori::CharacterClassDictionary charDictionary;
  nori::ConnectionCost connectionCost;

  // from connectionCost
  int backwardSize, forwardSize;
};

}  // namespace dictionary
}  // namespace nori

#endif  //__NORI_DICTIONARY_H__
