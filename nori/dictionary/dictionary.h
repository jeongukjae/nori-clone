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

  // get character category for given codePoint using charDictionary
  int getCharacterCategory(int codePoint) const;

 private:
  Darts::DoubleArray trie;
  nori::ListDictionary tokenDictionary;
  nori::Dictionary unkDictionary;
  nori::CharacterClassDictionary charDictionary;
  nori::ConnectionCost connectionCost;
};

}  // namespace dictionary
}  // namespace nori

#endif  //__NORI_DICTIONARY_H__
