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

}  // namespace internal

// Dictionary

absl::Status Dictionary::loadPrebuilt(std::string input) {
  this->bosEosSurface = "BOS/EOS";
  this->bosEosMorpheme.set_left_id(0);
  this->bosEosMorpheme.set_right_id(0);
  this->bosEosMorpheme.set_word_cost(0);

  auto status = internal::deserializeProtobuf(input, dictionary);
  if (!status.ok()) return status;

  trie.set_array(dictionary.darts_array().data(),
                 dictionary.darts_array().size());

  // backwardSize
  backwardSize = dictionary.connection_cost().backward_size();
  forwardSize = dictionary.connection_cost().forward_size();
  connectionCostData = dictionary.connection_cost().cost_lists().data();
  connectionCostMax = dictionary.connection_cost().cost_lists_size();

  initialized = true;

  return absl::OkStatus();
}

absl::Status Dictionary::loadUser(std::string filename) {
  auto status = userDictionary.load(
      filename, dictionary.left_id_nng(), dictionary.right_id_nng(),
      dictionary.right_id_nng_t(), dictionary.right_id_nng_f());
  if (absl::IsCancelled(status)) {
    LOG(WARNING) << status.message();
    return absl::OkStatus();
  }

  if (status.ok()) userInitialized = true;
  return status;
}

const nori::protos::CharacterClass Dictionary::getCharClass(
    const char* text) const {
  // Get next utf-8 character using ICU
  UChar32 c;
  int length = 1;
  int i = 0;
  U8_NEXT(text, i, length, c);

  auto it = dictionary.unknown_tokens().code_to_category_map().find(c);
  if (it != dictionary.unknown_tokens().code_to_category_map().end()) {
    return it->second;
  }
  return nori::protos::CharacterClass::HANGUL;
}

// User Dictionary

absl::Status UserDictionary::load(std::string filename, int leftId, int rightId,
                                  int rightId_T, int rightId_F) {
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

  if (terms.size() == 0) {
    // early exit to prevent error
    return absl::CancelledError("# terms in User Dictionary == 0");
  }

  std::stable_sort(
      terms.begin(), terms.end(),
      [](const std::vector<std::string> a, const std::vector<std::string> b) {
        return a[0] < b[0];
      });

  std::vector<const char*> keys;
  for (const auto& term : terms) {
    keys.push_back(term[0].data());

    nori::protos::Morpheme morpheme;

    morpheme.set_left_id(leftId);
    if (utils::internal::hasJongsungAtLast(term[0])) {
      morpheme.set_right_id(rightId_T);
    } else {
      morpheme.set_right_id(rightId);
    }
    morpheme.set_word_cost(-100000);
    morpheme.set_pos_type(term.size() == 1 ? nori::protos::POSType::MORPHEME
                                           : nori::protos::POSType::COMPOUND);

    morpheme.add_pos_tags(nori::protos::POSTag::NNG);
    for (int i = 2; i < term.size(); i++)
      morpheme.add_pos_tags(nori::protos::POSTag::NNG);
    for (int i = 1; i < term.size() - 1; i++) {
      auto expr = morpheme.add_expression();
      expr->set_pos_tag(nori::protos::POSTag::NNG);
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
