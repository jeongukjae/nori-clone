#include "nori/dictionary/builder.h"

#include <glog/logging.h>

#include <algorithm>
#include <fstream>
#include <sstream>

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "nori/protos/dictionary.pb.h"
#include "nori/utils.h"

namespace nori {
namespace dictionary {
namespace builder {

namespace internal {

void convertMeCabCSVEntry(const std::vector<std::string>& entry,
                          nori::Dictionary::Morpheme* morpheme) {
  morpheme->set_leftid(utils::internal::simpleAtoi(entry.at(1)));
  morpheme->set_rightid(utils::internal::simpleAtoi(entry.at(2)));
  morpheme->set_wordcost(utils::internal::simpleAtoi(entry.at(3)));

  const POSType posType = utils::resolvePOSType(entry.at(8));
  morpheme->set_postype(posType);

  POSTag rightPOS, leftPOS;

  if (posType == POSType::MORPHEME || posType == POSType::COMPOUND ||
      entry.at(9) == "*") {
    rightPOS = leftPOS = utils::resolvePOSTag(entry.at(4));
    CHECK(entry.at(9) == "*" && entry.at(10) == "*")
        << "Cannot parse MeCab CSV entry. term: " << entry.at(0)
        << ", left pos: " << entry.at(9) << ", right pos: " << entry.at(10);
  } else {
    leftPOS = utils::resolvePOSTag(entry.at(9));
    rightPOS = utils::resolvePOSTag(entry.at(10));
  }

  if (entry.at(7) != "*" && (entry.at(0) != entry.at(7)))
    morpheme->set_reading(entry.at(7));

  if (entry.at(11) != "*") {
    std::string expression = entry.at(11);
    std::vector<std::string> expressionTokens = absl::StrSplit(expression, '+');
    for (const auto& expressionToken : expressionTokens) {
      std::vector<std::string> tokenSplit =
          absl::StrSplit(expressionToken, '/');
      CHECK(tokenSplit.size() == 3)
          << "Cannot parse expression: " << expression;

      const auto token = morpheme->add_expression();
      token->set_surface(tokenSplit.at(0));
      token->set_postag(utils::resolvePOSTag(tokenSplit.at(1)));
    }
  }
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
  for (const auto& path : paths) {
    std::ifstream ifs(path);
    CHECK(!ifs.fail()) << path << " is missing!!";

    std::string line;
    while (std::getline(ifs, line)) {
      std::vector<std::string> entry;
      entry.reserve(12);

      if (normalize) {
        std::string normalized;
        auto status =
            utils::internal::normalizeUTF8(line, normalized, normalizationForm);
        CHECK(status.ok()) << "Cannot normalize string " << line;
        line = normalized;
      }
      utils::internal::parseCSVLine(line, entry);

      CHECK(entry.size() >= 12)
          << "Entry in CSV is not valid (12 field values expected): " << line;
      utils::internal::trimWhitespaces(entry[0]);

      entries.push_back(entry);
    }

    LOG(INFO) << "Read " << path << ". # terms: " << entries.size();
  }

  LOG(INFO) << "Sort all terms";
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

    internal::convertMeCabCSVEntry(entries[i],
                                   (*morphemeMap)[entryValue].add_morphemes());
  }

  LOG(INFO) << "Build trie. keys[0]: " << keys[0] << ", keys[10]: " << keys[10];
  trie = std::unique_ptr<Darts::DoubleArray>(new Darts::DoubleArray);
  CHECK(trie->build(keys.size(), const_cast<char**>(&keys[0]), nullptr, nullptr,
                    [](size_t current, size_t total) {
                      if (current % 100000 == 0)
                        LOG(INFO) << current << "/" << total;
                      return 0;
                    }) == 0)
      << "Cannot build trie.";

  // search 10'th item to check Trie is built properly
  int searchResult;
  trie->exactMatchSearch(keys[10], searchResult);
  CHECK(searchResult == 10) << "Trie isn't built properly.";

  return absl::OkStatus();
}

absl::Status TokenInfoDictionaryBuilder::save(
    absl::string_view outputDirectory) {
  // TODO(jeongukjae): continue here
  std::string arrayPath =
      utils::internal::joinPath(outputDirectory, DARTS_FILE_NAME);
  trie->save(arrayPath.c_str());
  return absl::OkStatus();
}

// UnknownDictionaryBuilder class

absl::Status UnknownDictionaryBuilder::parse(absl::string_view inputDirectory) {
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
