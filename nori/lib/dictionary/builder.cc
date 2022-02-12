#include "nori/lib/dictionary/builder.h"

#include <glog/logging.h>

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "nori/lib/protos/dictionary.pb.h"
#include "nori/lib/utils.h"
#include "snappy.h"

namespace nori {
namespace dictionary {
namespace builder {

namespace internal {

absl::Status convertMeCabCSVEntry(const std::vector<std::string>& entry,
                                  nori::protos::Morpheme* morpheme) {
  morpheme->set_left_id(utils::internal::simpleAtoi(entry.at(1)));
  morpheme->set_right_id(utils::internal::simpleAtoi(entry.at(2)));
  morpheme->set_word_cost(utils::internal::simpleAtoi(entry.at(3)));

  const nori::protos::POSType posType = utils::resolvePOSType(entry.at(8));
  morpheme->set_pos_type(posType);

  auto posTagList = morpheme->mutable_pos_tags();
  std::string posTags = entry.at(4);
  std::vector<std::string> posTokens = absl::StrSplit(posTags, '+');
  for (const auto& posToken : posTokens) {
    posTagList->Add(utils::resolvePOSTag(posToken));
  }

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
      token->set_pos_tag(utils::resolvePOSTag(tokenSplit.at(1)));
      token->set_surface(tokenSplit.at(0));
    }
  }

  return absl::OkStatus();
}

// serialize, compress, and save protobuf message
template <class T>
absl::Status serializeCompressedProtobuf(const std::string path,
                                         const T& message) {
  std::string pbData;
  if (!message.SerializeToString(&pbData)) {
    return absl::InternalError("Cannot serialize dictionary");
  }

  std::string compressed;
  snappy::Compress(pbData.data(), pbData.size(), &compressed);

  std::ofstream ofs(path, std::ios::out | std::ios::binary);
  if (ofs.fail())
    return absl::InvalidArgumentError(absl::StrCat("Cannot open file ", path));
  ofs.write(compressed.data(), compressed.size());
  ofs.close();
  return absl::OkStatus();
}

}  // namespace internal

// DictionaryBuilder

absl::Status DictionaryBuilder::build(absl::string_view inputDirectory) {
  absl::Status status;

  noriDictionary.set_do_normalize(normalize);
  if (normalize) {
    noriDictionary.set_normalization_form(normalizationForm);
  }

  status = this->buildTokenInfos(inputDirectory);
  if (!status.ok()) return status;
  status = this->buildUnknownTokenInfos(inputDirectory);
  if (!status.ok()) return status;
  status = this->buildConnectionCost(inputDirectory);
  if (!status.ok()) return status;
  status = this->findLeftRightIds(inputDirectory);
  if (!status.ok()) return status;

  return absl::OkStatus();
}

absl::Status DictionaryBuilder::save(std::string outputFilename) {
  return internal::serializeCompressedProtobuf(outputFilename, noriDictionary);
}

absl::Status DictionaryBuilder::buildTokenInfos(absl::string_view input) {
  // 1. Read all csvs
  std::vector<std::string> paths;
  utils::internal::listDirectory(input, paths, [](absl::string_view path) {
    return absl::EndsWithIgnoreCase(path, ".csv");
  });
  if (paths.size() == 0)
    return absl::InvalidArgumentError(
        absl::StrCat("Cannot find any csv files, ", input));

  std::vector<std::vector<std::string>> entries;
  entries.reserve(1000000);

  // 2. read csv files and append all rows into entries.
  for (const auto& path : paths) {
    std::ifstream ifs(path);
    if (ifs.fail())
      return absl::InvalidArgumentError(absl::StrCat(path, " is missing"));

    std::string line;
    while (std::getline(ifs, line)) {
      // normalize the line if required
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

  // 3. prepare building trie dictionary and add token infos
  std::stable_sort(
      entries.begin(), entries.end(),
      [](const std::vector<std::string> a, const std::vector<std::string> b) {
        return a[0] < b[0];
      });

  std::vector<const char*> keys;
  keys.reserve(entries.size());

  std::string lastValue = "";
  int entryValue = -1;
  nori::protos::MorphemeList* lastMorphemeList;

  for (int i = 0; i < entries.size(); i++) {
    if (entries[i][0] != lastValue) {
      keys.push_back(entries[i][0].c_str());
      lastValue = entries[i][0];
      entryValue++;
      lastMorphemeList = noriDictionary.mutable_tokens()->add_morphemes_list();
    }

    auto status = internal::convertMeCabCSVEntry(
        entries[i], lastMorphemeList->add_morphemes());
    if (!status.ok()) return status;
  }

  // 4. Build tries
  LOG(INFO) << "Build trie. keys[0]: " << keys[0] << ", keys[10]: " << keys[10];
  std::unique_ptr<Darts::DoubleArray> trie =
      std::unique_ptr<Darts::DoubleArray>(new Darts::DoubleArray);
  if (trie->build(keys.size(), const_cast<char**>(&keys[0])) != 0)
    return absl::InternalError("Cannot build trie.");

  noriDictionary.mutable_darts_array()->assign(
      static_cast<const char*>(trie->array()), trie->total_size());

  trie->set_array(noriDictionary.darts_array().data(),
                  noriDictionary.darts_array().size());

  int searchResult;
  for (int i = 0; i < keys.size(); i++) {
    trie->exactMatchSearch(keys[i], searchResult);
    if (searchResult != i)
      return absl::InternalError("Trie isn't built properly.");
  }

  return absl::OkStatus();
}

// UnknownDictionaryBuilder class

absl::Status DictionaryBuilder::buildUnknownTokenInfos(
    absl::string_view input) {
  // Build unk.def
  {
    const auto path = utils::internal::joinPath(input, "unk.def");
    LOG(INFO) << "Read unk dicitonary " << path;
    std::ifstream ifs(path);
    if (ifs.fail())
      return absl::InvalidArgumentError(absl::StrCat(path, " is missing"));

    auto* morphemeMap =
        noriDictionary.mutable_unknown_tokens()->mutable_morpheme_map();
    std::vector<std::vector<std::string>> allLines;
    const auto append = [&morphemeMap](std::string line) -> absl::Status {
      std::vector<std::string> entry = utils::internal::parseCSVLine(line);
      nori::protos::CharacterClass chClass;
      if (!nori::protos::CharacterClass_Parse(entry[0], &chClass)) {
        return absl::InvalidArgumentError(
            absl::StrCat("Cannot get character class ", entry[0]));
      }
      (*morphemeMap)[chClass].set_left_id(
          utils::internal::simpleAtoi(entry.at(1)));
      (*morphemeMap)[chClass].set_right_id(
          utils::internal::simpleAtoi(entry.at(2)));
      (*morphemeMap)[chClass].set_word_cost(
          utils::internal::simpleAtoi(entry.at(3)));
      (*morphemeMap)[chClass].mutable_pos_tags()->Add(
          utils::resolvePOSTag(entry.at(4)));

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

  // Build char.def
  {
    const auto path = utils::internal::joinPath(input, "char.def");
    LOG(INFO) << "Read char.def " << path;
    std::ifstream ifs(path);
    if (ifs.fail())
      return absl::InvalidArgumentError(absl::StrCat(path, " is missing"));

    std::string line;
    std::regex spaceRegex("\\s+");
    std::regex commentRegex("\\s*#.*");
    auto* invokeMap =
        noriDictionary.mutable_unknown_tokens()->mutable_invoke_map();
    auto* codeToCategoryMap =
        noriDictionary.mutable_unknown_tokens()->mutable_code_to_category_map();

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
        nori::protos::CharacterClass chCls;
        if (!nori::protos::CharacterClass_Parse(splits[0], &chCls)) {
          ifs.close();
          return absl::InvalidArgumentError(
              absl::StrCat("Cannot read character class ", splits[0]));
        }
        (*invokeMap)[chCls].set_invoke(utils::internal::simpleAtoi(splits[1]));
        (*invokeMap)[chCls].set_group(utils::internal::simpleAtoi(splits[2]));
        (*invokeMap)[chCls].set_length(utils::internal::simpleAtoi(splits[3]));
      } else {
        std::vector<std::string> tokens = absl::StrSplit(line, " ");

        nori::protos::CharacterClass chCls;
        if (!nori::protos::CharacterClass_Parse(tokens[1], &chCls)) {
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

absl::Status DictionaryBuilder::buildConnectionCost(absl::string_view input) {
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

  std::vector<int> array(forwardSize * backwardSize);

  while (std::getline(ifs, line)) {
    std::vector<std::string> splits = absl::StrSplit(line, " ");

    if (splits.size() != 3) {
      ifs.close();
      return absl::InvalidArgumentError("Malformed matrix.def");
    }

    int forwardId = utils::internal::simpleAtoi(splits[0]);
    int backwardId = utils::internal::simpleAtoi(splits[1]);
    int cost = utils::internal::simpleAtoi(splits[2]);
    array[backwardSize * forwardId + backwardId] = cost;
  }
  noriDictionary.mutable_connection_cost()->mutable_cost_lists()->Assign(
      array.begin(), array.end());
  noriDictionary.mutable_connection_cost()->set_forward_size(forwardSize);
  noriDictionary.mutable_connection_cost()->set_backward_size(backwardSize);

  ifs.close();
  return absl::OkStatus();
}

absl::Status DictionaryBuilder::findLeftRightIds(absl::string_view input) {
  // find requried left id
  {
    const auto path = utils::internal::joinPath(input, "left-id.def");
    LOG(INFO) << "Read left-ids (left-id.def) " << path;
    std::ifstream ifs(path);
    if (ifs.fail())
      return absl::InvalidArgumentError(absl::StrCat(path, " is missing"));

    std::string line;
    while (std::getline(ifs, line)) {
      // NNG LeftId
      if (absl::StrContains(line, "NNG,*,*,*,*,*,*,*")) {
        std::vector<std::string> splits = absl::StrSplit(line, " ");
        noriDictionary.set_left_id_nng(utils::internal::simpleAtoi(splits[0]));
      }
    }
  }

  // find requried right ids
  {
    const auto path = utils::internal::joinPath(input, "right-id.def");
    LOG(INFO) << "Read right-ids (right-id.def) " << path;
    std::ifstream ifs(path);
    if (ifs.fail())
      return absl::InvalidArgumentError(absl::StrCat(path, " is missing"));

    std::string line;
    while (std::getline(ifs, line)) {
      // NNG rightId
      if (absl::StrContains(line, "NNG,*,*,*,*,*,*,*")) {
        std::vector<std::string> splits = absl::StrSplit(line, " ");
        noriDictionary.set_right_id_nng(utils::internal::simpleAtoi(splits[0]));
      }

      // NNG rightId with jongsung
      else if (absl::StrContains(line, "NNG,*,T,*,*,*,*,*")) {
        std::vector<std::string> splits = absl::StrSplit(line, " ");
        noriDictionary.set_right_id_nng_t(
            utils::internal::simpleAtoi(splits[0]));
      }

      // NNG rightId without jongsung
      else if (absl::StrContains(line, "NNG,*,F,*,*,*,*,*")) {
        std::vector<std::string> splits = absl::StrSplit(line, " ");
        noriDictionary.set_right_id_nng_f(
            utils::internal::simpleAtoi(splits[0]));
      }
    }
  }

  LOG(INFO) << "left-id for NNG: " << noriDictionary.left_id_nng()
            << ", right-id for NNG: " << noriDictionary.right_id_nng()
            << ", right-id with Jongsung: " << noriDictionary.right_id_nng_t()
            << ", right-id w/o Jongsung: " << noriDictionary.right_id_nng_f();

  return absl::OkStatus();
}

}  // namespace builder
}  // namespace dictionary
}  // namespace nori
