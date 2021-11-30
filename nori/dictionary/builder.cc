#include "nori/dictionary/builder.h"

#include <glog/logging.h>

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "nori/dictionary/dictionary.h"
#include "nori/protos/dictionary.pb.h"
#include "nori/utils.h"
#include "snappy.h"

namespace nori {
namespace dictionary {
namespace builder {

namespace internal {

absl::Status convertMeCabCSVEntry(const std::vector<std::string>& entry,
                                  nori::Morpheme* morpheme) {
  morpheme->set_leftid(utils::internal::simpleAtoi(entry.at(1)));
  morpheme->set_rightid(utils::internal::simpleAtoi(entry.at(2)));
  morpheme->set_wordcost(utils::internal::simpleAtoi(entry.at(3)));

  const POSType posType = utils::resolvePOSType(entry.at(8));
  morpheme->set_postype(posType);

  if (entry.at(11) != "*") {
    std::string expression = entry.at(11);
    std::vector<std::string> expressionTokens = absl::StrSplit(expression, '+');
    for (const auto& expressionToken : expressionTokens) {
      std::vector<std::string> tokenSplit =
          absl::StrSplit(expressionToken, '/');
      if (tokenSplit.size() != 3) {
        return absl::InvalidArgumentError(
            absl::StrCat("Cannot parse expression: ", expression));
      }

      const auto token = morpheme->add_expression();
      token->set_postag(utils::resolvePOSTag(tokenSplit.at(1)));
      token->set_surface(tokenSplit.at(0));
    }
  }
  return absl::OkStatus();
}

// serialize, compress, and save protobuf message
template <class T>
absl::Status serializeCompressedProtobuf(const std::string& path, T message) {
  std::string pbData;
  if (!message.SerializeToString(&pbData)) {
    return absl::InternalError("Cannot serialize dictionary");
  }

  std::string compressed;
  snappy::Compress(pbData.data(), pbData.size(), &compressed);

  std::ofstream ofs(path, std::ios::out | std::ios::binary);
  if (ofs.fail())
    return absl::PermissionDeniedError(absl::StrCat("Cannot open file ", path));
  ofs.write(compressed.data(), compressed.size());
  ofs.close();
  return absl::OkStatus();
}

}  // namespace internal

// MeCabDictionary class

MeCabDictionaryBuilder::MeCabDictionaryBuilder(
    bool normalize, const std::string normalizationForm) {
  builders.emplace_back(
      new TokenInfoDictionaryBuilder(normalize, normalizationForm));
  builders.emplace_back(new UnknownDictionaryBuilder());
  builders.emplace_back(new ConnectionCostsBuilder());
}

void MeCabDictionaryBuilder::build(absl::string_view input,
                                   absl::string_view output) {
  for (auto& builder : builders) {
    auto status = builder->parse(input);
    CHECK(status.ok()) << status;

    status = builder->save(output);
    CHECK(status.ok()) << status;
  }
}

// TokenInfoDictionaryBuilder class

absl::Status TokenInfoDictionaryBuilder::parse(absl::string_view input) {
  std::vector<std::string> paths;
  utils::internal::listDirectory(input, paths, [](absl::string_view path) {
    return absl::EndsWithIgnoreCase(path, ".csv");
  });

  std::vector<std::vector<std::string>> entries;
  entries.reserve(1000000);

  // read csv files and append all rows into entries.
  for (const auto& path : paths) {
    std::ifstream ifs(path);
    if (ifs.fail())
      return absl::InvalidArgumentError(absl::StrCat(path, " is missing"));

    std::string line;
    while (std::getline(ifs, line)) {
      if (normalize) {
        std::string normalized;
        auto status =
            utils::internal::normalizeUTF8(line, normalized, normalizationForm);
        if (!status.ok()) {
          ifs.close();
          return absl::InternalError(
              absl::StrCat("Cannot normalize string", line));
        }
        line = normalized;
      }
      auto entry = utils::internal::parseCSVLine(line);

      if (entry.size() < 12) {
        ifs.close();
        return absl::InvalidArgumentError(absl::StrCat(
            "Entry in CSV is not valid (12 field values expected): ", line));
      }
      utils::internal::trimWhitespaces(entry[0]);

      entries.push_back(entry);
    }
    ifs.close();
    LOG(INFO) << "Read " << path << ". # terms: " << entries.size();
  }

  std::stable_sort(
      entries.begin(), entries.end(),
      [](const std::vector<std::string> a, const std::vector<std::string> b) {
        return a[0] < b[0];
      });

  std::vector<const char*> keys;
  keys.reserve(entries.size());

  std::string lastValue = "";
  int entryValue = -1;
  auto morphemeListMap = dictionary.mutable_morphemelistmap();

  for (int i = 0; i < entries.size(); i++) {
    if (entries[i][0] != lastValue) {
      keys.push_back(entries[i][0].c_str());
      lastValue = entries[i][0];

      nori::MorphemeList morphemeList;
      (*morphemeListMap)[++entryValue] = morphemeList;
    }

    auto status = internal::convertMeCabCSVEntry(
        entries[i], (*morphemeListMap)[entryValue].add_morphemes());
    if (!status.ok()) return status;
  }

  LOG(INFO) << "Build trie. keys[0]: " << keys[0] << ", keys[10]: " << keys[10];
  trie = std::unique_ptr<Darts::DoubleArray>(new Darts::DoubleArray);
  if (trie->build(keys.size(), const_cast<char**>(&keys[0])) != 0)
    return absl::InternalError("Cannot build trie.");

  // search 10'th item to check Trie is built properly
  int searchResult;
  trie->exactMatchSearch(keys[10], searchResult);
  if (searchResult != 10)
    return absl::InternalError("Trie isn't built properly.");

  return absl::OkStatus();
}

absl::Status TokenInfoDictionaryBuilder::save(absl::string_view output) {
  {
    LOG(INFO) << "Save token info dictionary.";
    std::string arrayPath = utils::internal::joinPath(output, NORI_DICT_FILE);
    trie->save(arrayPath.c_str());
  }

  {
    std::string path = utils::internal::joinPath(output, NORI_DICT_META_FILE);
    LOG(INFO) << "Write dictionary meta " << path;
    auto status = internal::serializeCompressedProtobuf(path, dictionary);
    if (!status.ok()) return status;
  }
  return absl::OkStatus();
}

// UnknownDictionaryBuilder class

absl::Status UnknownDictionaryBuilder::parse(absl::string_view input) {
  {
    const auto path = utils::internal::joinPath(input, "unk.def");
    LOG(INFO) << "Read unk dicitonary " << path;
    std::ifstream ifs(path);
    if (ifs.fail())
      return absl::InvalidArgumentError(absl::StrCat(path, " is missing"));

    auto* morphemeMap = unkDictionary.mutable_morphememap();
    std::vector<std::vector<std::string>> allLines;
    const auto append = [&morphemeMap](std::string line) -> absl::Status {
      std::vector<std::string> entry = utils::internal::parseCSVLine(line);
      nori::CharacterClass chClass;
      if (!nori::CharacterClass_Parse(entry[0], &chClass)) {
        return absl::InvalidArgumentError(
            absl::StrCat("Cannot get character class ", entry[0]));
      }
      (*morphemeMap)[chClass].set_leftid(
          utils::internal::simpleAtoi(entry.at(1)));
      (*morphemeMap)[chClass].set_rightid(
          utils::internal::simpleAtoi(entry.at(2)));
      (*morphemeMap)[chClass].set_wordcost(
          utils::internal::simpleAtoi(entry.at(3)));

      return absl::OkStatus();
    };

    // put ngram definition
    append("NGRAM,1798,3559,3677,SY,*,*,*,*,*,*,*").IgnoreError();
    std::string line;
    while (std::getline(ifs, line)) {
      auto status = append(line);
      if (!status.ok()) {
        ifs.close();
        return status;
      };
    }
    ifs.close();
  }

  {
    const auto path = utils::internal::joinPath(input, "char.def");
    LOG(INFO) << "Read char.def " << path;
    std::ifstream ifs(path);
    if (ifs.fail())
      return absl::InvalidArgumentError(absl::StrCat(path, " is missing"));

    std::string line;
    std::regex spaceRegex("\\s+");
    std::regex commentRegex("\\s*#.*");
    auto charCategoryDefinitionMap =
        charDictionary.mutable_charcategorydefinitionmap();
    auto codeToCategoryMap = charDictionary.mutable_codetocategorymap();

    while (std::getline(ifs, line)) {
      if (absl::StartsWith(line, "#")) continue;  // skip comments
      utils::internal::trimWhitespaces(line);
      if (line.length() == 0) continue;  // skip empty line

      // remove comments
      line = std::regex_replace(line, commentRegex, "");
      line = std::regex_replace(line, spaceRegex, " ");

      if (!absl::StartsWith(line, "0x")) {
        // char category definition
        std::vector<std::string> splits = absl::StrSplit(line, " ");
        nori::CharacterClass chCls;
        if (!nori::CharacterClass_Parse(splits[0], &chCls)) {
          ifs.close();
          return absl::InvalidArgumentError(
              absl::StrCat("Cannot read character class ", splits[0]));
        }
        auto categoryDef = (*charCategoryDefinitionMap)[chCls];
        categoryDef.set_invoke(utils::internal::simpleAtoi(splits[1]));
        categoryDef.set_invoke(utils::internal::simpleAtoi(splits[2]));
        categoryDef.set_invoke(utils::internal::simpleAtoi(splits[3]));
      } else {
        std::vector<std::string> tokens = absl::StrSplit(line, " ");

        nori::CharacterClass chCls;
        if (!nori::CharacterClass_Parse(tokens[1], &chCls)) {
          ifs.close();
          return absl::InvalidArgumentError(
              absl::StrCat("Cannot read character class ", tokens[1]));
        }

        if (absl::StrContains(tokens[0], "..")) {
          std::vector<std::string> codePoints = absl::StrSplit(tokens[0], "..");
          int codePointFrom = utils::internal::simpleHexAtoi(codePoints[0]);
          int codePointTo = utils::internal::simpleHexAtoi(codePoints[1]);

          for (int i = codePointFrom; i <= codePointTo; i++)
            (*codeToCategoryMap)[i] = chCls;
        } else {
          int codePoint = utils::internal::simpleHexAtoi(tokens[0]);
          (*codeToCategoryMap)[codePoint] = chCls;
        }
      }
    }
    ifs.close();
  }

  return absl::OkStatus();
}

absl::Status UnknownDictionaryBuilder::save(absl::string_view output) {
  {
    std::string path = utils::internal::joinPath(output, NORI_UNK_FILE);
    LOG(INFO) << "save unk dictionary file " << path;
    auto status = internal::serializeCompressedProtobuf(path, unkDictionary);
    if (!status.ok()) return status;
  }

  {
    std::string path = utils::internal::joinPath(output, NORI_CHAR_FILE);
    LOG(INFO) << "save char dictionary file " << path;
    auto status = internal::serializeCompressedProtobuf(path, charDictionary);
    if (!status.ok()) return status;
  }

  return absl::OkStatus();
}

// ConnectionCostsBuilder class

absl::Status ConnectionCostsBuilder::parse(absl::string_view input) {
  const auto path = utils::internal::joinPath(input, "matrix.def");
  LOG(INFO) << "Read connection costs (matrix.def) " << path;
  std::ifstream ifs(path);
  if (ifs.fail())
    return absl::InvalidArgumentError(absl::StrCat(path, " is missing"));

  std::string line;
  std::getline(ifs, line);
  std::vector<std::string> dimensions = absl::StrSplit(line, " ");
  if (dimensions.size() != 2) {
    ifs.close();
    return absl::InvalidArgumentError("Malformed matrix.def");
  }
  int forwardSize = utils::internal::simpleAtoi(dimensions[0]);
  int backwardSize = utils::internal::simpleAtoi(dimensions[1]);
  if (forwardSize <= 0 || backwardSize <= 0) {
    ifs.close();
    return absl::InvalidArgumentError("Malformed matrix.def");
  }

  LOG(INFO) << "Forward Size: " << forwardSize
            << ", Backward Size: " << backwardSize;

  std::vector<std::vector<int>> array(forwardSize);
  for (int i = 0; i < forwardSize; i++) array[i].resize(backwardSize);

  while (std::getline(ifs, line)) {
    std::vector<std::string> splits = absl::StrSplit(line, " ");

    if (splits.size() != 3) {
      ifs.close();
      return absl::InvalidArgumentError("Malformed matrix.def");
    }

    int forwardId = utils::internal::simpleAtoi(splits[0]);
    int backwardId = utils::internal::simpleAtoi(splits[1]);
    int cost = utils::internal::simpleAtoi(splits[2]);
    array[forwardId][backwardId] = cost;
  }

  for (int i = 0; i < forwardSize; i++) {
    auto costs = connectionCost.add_costlists();
    for (int j = 0; j < backwardSize; j++) costs->add_cost(array[i][j]);
  }

  ifs.close();
  return absl::OkStatus();
}

absl::Status ConnectionCostsBuilder::save(absl::string_view output) {
  std::string path =
      utils::internal::joinPath(output, NORI_CONNECTION_COST_FILE);
  LOG(INFO) << "save connection costs dictionary file " << path;
  return internal::serializeCompressedProtobuf(path, connectionCost);
}

}  // namespace builder
}  // namespace dictionary
}  // namespace nori
