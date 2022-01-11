#include "nori/lib/dictionary/dictionary.h"

#include <darts.h>
#include <glog/logging.h>

#include <fstream>
#include <sstream>

#include "absl/status/status.h"
#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "icu4c/source/common/unicode/unistr.h"
#include "nori/lib/utils.h"
#include "re2/re2.h"
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

absl::Status exactMatchMorpheme(
    const Darts::DoubleArray* trie,
    const nori::TokenInfoDictionary* tokenDictionary, const std::string word,
    const nori::Morpheme*& outputMorpheme) {
  int searchResult;
  trie->exactMatchSearch(word.data(), searchResult);

  if (searchResult == -1) {
    return absl::InternalError(
        absl::StrCat("Cannot exact match morpheme with word ", word));
  }

  auto morphemeList = &tokenDictionary->morphemelistmap().at(searchResult);
  if (morphemeList->morphemes_size() != 1) {
    return absl::InternalError("Cannot get right id with jongsung");
  }
  outputMorpheme = &morphemeList->morphemes(0);

  return absl::OkStatus();
}

}  // namespace internal

// Dictionary

absl::Status Dictionary::loadPrebuilt(absl::string_view input) {
  this->bosEosSurface = "BOS/EOS";
  this->bosEosMorpheme.set_leftid(0);
  this->bosEosMorpheme.set_rightid(0);
  this->bosEosMorpheme.set_wordcost(0);

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
  connectionCostData = connectionCost.costlists().data();
  connectionCostMax = connectionCost.costlists_size();

  LOG(INFO) << "Done reading dictionary.";
  initialized = true;

  return absl::OkStatus();
}

absl::Status Dictionary::loadUser(absl::string_view filename) {
  const nori::Morpheme *morphemeWithJongsung, *morphemeWithHangul;
  auto status = internal::exactMatchMorpheme(&trie, &tokenDictionary, "놀이방",
                                             morphemeWithJongsung);
  if (!status.ok()) return status;

  status = internal::exactMatchMorpheme(&trie, &tokenDictionary, "딱지놀이",
                                        morphemeWithHangul);
  if (!status.ok()) return status;

  status = userDictionary.load(filename, morphemeWithJongsung->leftid(),
                               morphemeWithHangul->rightid(),
                               morphemeWithJongsung->rightid());
  if (status.ok()) userInitialized = true;
  return status;
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

// User Dictionary

absl::Status UserDictionary::load(absl::string_view filename, int leftId,
                                  int rightId, int rightIdWithJongsung) {
  trie.clear();
  morphemes.clear();

  std::ifstream ifs(filename);
  if (ifs.fail())
    return absl::InvalidArgumentError(absl::StrCat(filename, " is missing"));

  std::string rawLine;
  RE2 spacePattern(R"(\s+)");
  if (!spacePattern.ok()) {
    return absl::InternalError("Cannot build re2 pattern");
  }

  std::vector<std::vector<std::string>> terms;
  while (std::getline(ifs, rawLine)) {
    RE2::Replace(&rawLine, spacePattern, " ");
    absl::string_view line(rawLine);

    {
      // remove trailing comments
      auto it = std::find_if(line.begin(), line.end(),
                             [](char x) { return x == '#'; });
      line = line.substr(0, it - line.begin());
    }
    line = absl::StripAsciiWhitespace(line);
    if (line == "") {
      // skip empty line
      continue;
    }

    std::vector<std::string> splits = absl::StrSplit(line, " ");
    terms.push_back(splits);
  }

  ifs.close();

  std::stable_sort(
      terms.begin(), terms.end(),
      [](const std::vector<std::string> a, const std::vector<std::string> b) {
        return a[0] < b[0];
      });

  std::vector<const char*> keys;
  for (const auto& term : terms) {
    keys.push_back(term[0].data());

    nori::Morpheme morpheme;

    morpheme.set_leftid(leftId);
    // TODO(jeongukaje): set right id
    morpheme.set_rightid(rightId);
    morpheme.set_wordcost(-100000);
    morpheme.set_postype(term.size() == 1 ? nori::POSType::MORPHEME
                                          : nori::POSType::COMPOUND);

    morpheme.add_postag(nori::POSTag::NNG);
    for (int i = 2; i < term.size(); i++)
      morpheme.add_postag(nori::POSTag::NNG);
    for (int i = 1; i < term.size() - 1; i++) {
      auto expr = morpheme.add_expression();
      expr->set_postag(nori::POSTag::NNG);
      expr->set_surface(term[i]);
    }

    morphemes.push_back(morpheme);
  }

  if (trie.build(keys.size(), const_cast<char**>(&keys[0])) != 0)
    return absl::InternalError("Cannot build trie.");

  // search second item to check Trie is built properly
  int searchResult;
  trie.exactMatchSearch(keys[1], searchResult);
  if (searchResult != 1)
    return absl::InternalError("Trie isn't built properly.");

  return absl::OkStatus();
}

}  // namespace dictionary
}  // namespace nori
