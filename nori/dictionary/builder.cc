#include "nori/dictionary/builder.h"

#include <glog/logging.h>

#include <algorithm>
#include <fstream>
#include <sstream>

#include "absl/strings/match.h"
#include "nori/utils.h"

namespace nori {
namespace dictionary {
namespace builder {

namespace internal {

void convertMeCabCSVEntry(const std::vector<std::string> entry,
                          nori::Dictionary::Morpheme* morpheme) {
  morpheme->set_leftid(utils::internal::simpleAtoi(entry.at(1)));
  morpheme->set_rightid(utils::internal::simpleAtoi(entry.at(2)));
  morpheme->set_wordcost(utils::internal::simpleAtoi(entry.at(3)));
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

  std::vector<std::vector<std::string>> entries(100000);
  for (const auto& path : paths) {
    std::ifstream ifs(path);
    CHECK(!ifs.fail()) << path << " is missing";

    std::string line;
    std::vector<std::string> entry(12);
    while (std::getline(ifs, line)) {
      utils::internal::parsCSVLine(line, entry);

      CHECK(entry.size() >= 12)
          << "Entry in CSV is not valid (12 field values expected): " << line;
      utils::internal::trimWhitespaces(entry[0]);

      if (normalize) {
        for (int i = 0; i < entry.size(); i++) {
          std::string normalized;
          auto status = utils::internal::normalizeUTF8(entry[i], normalized,
                                                       normalizationForm);
          CHECK(status.ok()) << "Cannot normalize string " << entry[i];
          entry[i] = normalized;
        }
      }
      entries.push_back(entry);
    }
  }

  std::stable_sort(entries.begin(), entries.end(),
                   [](std::vector<std::string> a, std::vector<std::string> b) {
                     return a[0].compare(b[0]) < 0;
                   });

  trie = std::unique_ptr<Darts::DoubleArray>(new Darts::DoubleArray);
  // TODO(builder): continue here
  nori::Dictionary dictionary;
  for (auto entry : entries) {
    internal::convertMeCabCSVEntry(entry, dictionary.add_morphemes());
  }

  return absl::OkStatus();
}

absl::Status TokenInfoDictionaryBuilder::save(
    absl::string_view outputDirectory) {
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
