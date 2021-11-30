#include "nori/dictionary/builder.h"

#include <glog/logging.h>

#include <algorithm>
#include <fstream>
#include <sstream>

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "nori/dictionary/dictionary.h"
#include "nori/protos/dictionary.pb.h"
#include "nori/utils.h"

namespace nori {
namespace dictionary {
namespace builder {

namespace internal {

absl::Status convertMeCabCSVEntry(const std::vector<std::string>& entry,
                                  nori::Dictionary::Morpheme* morpheme) {
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

}  // namespace internal

// MeCabDictionary class

MeCabDictionaryBuilder::MeCabDictionaryBuilder(
    bool normalize, const std::string normalizationForm) {
  builders.emplace_back(
      new TokenInfoDictionaryBuilder(normalize, normalizationForm));
  builders.emplace_back(new UnknownDictionaryBuilder());
  builders.emplace_back(new ConnectionCostsBuilder());
}

void MeCabDictionaryBuilder::build(absl::string_view inputDirectory,
                                   absl::string_view outputDirectory) {
  for (auto& builder : builders) {
    auto status = builder->parse(inputDirectory);
    CHECK(status.ok()) << "Unexpected error " << status;

    status = builder->save(outputDirectory);
    CHECK(status.ok()) << "Unexpected error " << status;
  }
}

// TokenInfoDictionaryBuilder class

absl::Status TokenInfoDictionaryBuilder::parse(
    absl::string_view inputDirectory) {
  std::vector<std::string> paths;
  utils::internal::listDirectory(
      inputDirectory, paths, [](absl::string_view path) {
        return absl::EndsWithIgnoreCase(path, ".csv");
      });

  std::vector<std::vector<std::string>> entries;
  entries.reserve(1000000);

  // read csv files and append all rows into entries.
  for (const auto& path : paths) {
    std::ifstream ifs(path);
    if (ifs.fail())
      return absl::InvalidArgumentError(absl::StrCat(path, " is missing!!"));

    std::string line;
    while (std::getline(ifs, line)) {
      std::vector<std::string> entry;
      entry.reserve(12);

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
      utils::internal::parseCSVLine(line, entry);

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
  auto morphemeMap = dictionary.mutable_morphememap();

  for (int i = 0; i < entries.size(); i++) {
    if (entries[i][0] != lastValue) {
      keys.push_back(entries[i][0].c_str());
      lastValue = entries[i][0];

      nori::Dictionary::MorphemeList morphemeList;
      (*morphemeMap)[++entryValue] = morphemeList;
    }

    auto status = internal::convertMeCabCSVEntry(
        entries[i], (*morphemeMap)[entryValue].add_morphemes());
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

absl::Status TokenInfoDictionaryBuilder::save(
    absl::string_view outputDirectory) {
  LOG(INFO) << "Save token info dictionary.";
  std::string arrayPath =
      utils::internal::joinPath(outputDirectory, NORI_DARTS_FILE_NAME);
  trie->save(arrayPath.c_str());

  std::string pbPath =
      utils::internal::joinPath(outputDirectory, NORI_DARTS_META_FILE_NAME);
  std::string dictionaryMeta;
  std::ofstream ofs(pbPath, std::ios::out | std::ios::binary);
  if (ofs.fail())
    return absl::PermissionDeniedError(
        absl::StrCat("Cannot open file ", pbPath));
  if (!dictionary.SerializeToOstream(&ofs)) {
    ofs.close();
    return absl::InternalError("Cannot serialize dictionary");
  }
  ofs.close();
  return absl::OkStatus();
}

// UnknownDictionaryBuilder class

absl::Status UnknownDictionaryBuilder::parse(absl::string_view inputDirectory) {
  const auto unkPath = utils::internal::joinPath(inputDirectory, "unk.def");
  const auto charPath = utils::internal::joinPath(inputDirectory, "char.def");

  return absl::OkStatus();
}

absl::Status UnknownDictionaryBuilder::save(absl::string_view outputDirectory) {
  return absl::OkStatus();
}

// ConnectionCostsBuilder class

absl::Status ConnectionCostsBuilder::parse(absl::string_view inputDirectory) {
  return absl::OkStatus();
}

absl::Status ConnectionCostsBuilder::save(absl::string_view outputDirectory) {
  return absl::OkStatus();
}

}  // namespace builder
}  // namespace dictionary
}  // namespace nori
