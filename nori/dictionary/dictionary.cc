#include "nori/dictionary/dictionary.h"

#include <glog/logging.h>

#include <fstream>
#include <sstream>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "icu4c/source/common/unicode/unistr.h"
#include "nori/utils.h"
#include "snappy.h"

namespace nori {
namespace dictionary {

namespace internal {

// read, uncompress, and parse protobuf message
template <class T>
absl::Status deserializeProtobuf(const std::string& path, T& message) {
  std::ifstream ifs(path, std::ios::in | std::ios::binary);
  if (ifs.fail())
    return absl::InvalidArgumentError(absl::StrCat("Cannot open file ", path));

  std::stringstream buf;
  buf << ifs.rdbuf();
  ifs.close();

  std::string compressedData = buf.str();
  std::string uncompressed;
  if (!snappy::Uncompress(compressedData.c_str(), compressedData.size(),
                          &uncompressed))
    return absl::InternalError(absl::StrCat("Cannot uncompress data ", path));

  if (!message.ParseFromString(uncompressed)) {
    return absl::InternalError(
        absl::StrCat("Cannot deserialize message", path));
  }

  return absl::OkStatus();
}

}  // namespace internal

absl::Status Dictionary::load(absl::string_view input) {
  {
    std::string path = utils::internal::joinPath(input, NORI_DICT_FILE);
    LOG(INFO) << "Read trie dictionary " << path;
    if (trie.open(path.c_str()) != 0) {
      return absl::InvalidArgumentError(
          absl::StrCat("Canont read trie dictionary from ", path));
    }
  }

  {
    std::string path = utils::internal::joinPath(input, NORI_DICT_META_FILE);
    LOG(INFO) << "Read token info dictionary " << path;
    auto status = internal::deserializeProtobuf(path, tokenDictionary);
    if (!status.ok()) return status;
  }

  {
    std::string path = utils::internal::joinPath(input, NORI_UNK_FILE);
    LOG(INFO) << "Read unk dictionary " << path;
    auto status = internal::deserializeProtobuf(path, unkDictionary);
    if (!status.ok()) return status;
  }

  {
    std::string path = utils::internal::joinPath(input, NORI_CHAR_FILE);
    LOG(INFO) << "Read char dictionary " << path;
    auto status = internal::deserializeProtobuf(path, charDictionary);
    if (!status.ok()) return status;
  }

  {
    std::string path =
        utils::internal::joinPath(input, NORI_CONNECTION_COST_FILE);
    LOG(INFO) << "Read connection costs " << path;
    auto status = internal::deserializeProtobuf(path, connectionCost);
    if (!status.ok()) return status;
  }

  // backwardSize
  backwardSize = connectionCost.backwardsize();
  forwardSize = connectionCost.forwardsize();

  LOG(INFO) << "Done reading dictionary.";

  return absl::OkStatus();
}

const nori::CharacterClass Dictionary::getCharClass(const char* text) const {
  // Get next utf-8 character using ICU
  UChar32 c;
  int length = 1;
  int i = 0;
  U8_NEXT(text, i, length, c);

  auto it = charDictionary.codetocategorymap().find(c);
  if (it != charDictionary.codetocategorymap().end()) {
    return it->second;
  }
  return nori::CharacterClass::DEFAULT;
}

const int Dictionary::getConnectionCost(nori::Morpheme* rightMorpheme,
                                        nori::Morpheme* leftMorpheme) const {
  // TODO(jeongukjae) error handling
  int rightId = (rightMorpheme == nullptr) ? 0 : rightMorpheme->rightid();
  int leftId = (leftMorpheme == nullptr) ? 0 : leftMorpheme->leftid();

  return connectionCost.costlists().at(backwardSize * rightId + leftId);
}

}  // namespace dictionary
}  // namespace nori
