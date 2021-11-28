#include "nori/dictionary/builder.h"

#include <dirent.h>
#include <glog/logging.h>

#include <algorithm>
#include <fstream>
#include <sstream>

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "icu4c/source/common/unicode/errorcode.h"
#include "icu4c/source/common/unicode/normalizer2.h"
#include "icu4c/source/common/unicode/unistr.h"

namespace nori {
namespace dictionary {
namespace builder {

namespace internal {

absl::Status normalizeUTF8(const std::string input, std::string& output,
                           absl::string_view normalizationForm) {
  icu::ErrorCode icuError;
  const icu::Normalizer2* normalizer;
  // TODO(builder): add normalization form
  if (normalizationForm == "NFKC") {
    normalizer = icu::Normalizer2::getNFKCInstance(icuError);
  } else {
    return absl::InvalidArgumentError(absl::StrCat(
        "Cannot find proper normalizer for form ", normalizationForm));
  }

  if (!icuError.isSuccess())
    return absl::InternalError(
        absl::StrCat("Cannot normalize unicode strings. Cannot get ",
                     normalizationForm, " Instance."));

  icu::StringByteSink<std::string> byte_sink(&output);
  normalizer->normalizeUTF8(0, icu::StringPiece(input.c_str(), input.size()),
                            byte_sink, nullptr, icuError);

  if (!icuError.isSuccess())
    return absl::InternalError(
        "Cannot normalize unicode strings. Cannot normalize input string.");

  return absl::OkStatus();
}

void listDirectory(absl::string_view directory, std::vector<std::string>& paths,
                   std::function<bool(std::string)> functor) {
  std::string direcotryString = absl::StrCat(directory);
  bool isDirectoryEndsWithSlash = absl::EndsWith(directory, "/");
  dirent* entry;
  DIR* dir = opendir(direcotryString.c_str());
  std::string path;

  if (dir != NULL) {
    while ((entry = readdir(dir)) != NULL) {
      if (isDirectoryEndsWithSlash) {
        path = absl::StrCat(directory, entry->d_name);
      } else {
        path = absl::StrCat(directory, "/", entry->d_name);
      }

      if (functor(path)) {
        paths.push_back(path);
      }
    }

    closedir(dir);
  }
  std::sort(paths.begin(), paths.end());
}

void parsCSVLine(std::string line, std::vector<std::string>& entries) {
  bool insideQuote = false;
  int start = 0;

  for (int index = 0; index < line.length(); index++) {
    char c = line.at(index);

    if (c == '"') {
      insideQuote = !insideQuote;
    } else if (c == ',' && !insideQuote) {
      int length = index - start, offset = 0;
      while (offset < (index - start) / 2 &&
             (line.at(start + offset) == '"' &&
              line.at(index - offset - 1) == '"')) {
        length -= 2;
        start += 1;
        offset++;
      }
      entries.push_back(line.substr(start, length));
      start = index + 1;
    }
  }
  entries.push_back(line.substr(start));
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
  internal::listDirectory(inputDirectory, paths, [](absl::string_view path) {
    return absl::EndsWithIgnoreCase(path, ".csv");
  });

  std::vector<std::vector<std::string>> allLines(400000);
  for (const auto& path : paths) {
    std::ifstream ifs(path);
    CHECK(!ifs.fail()) << path << " is missing";

    std::string line;
    std::vector<std::string> entries;
    while (std::getline(ifs, line)) {
      internal::parsCSVLine(line, entries);

      CHECK(entries.size() >= 12)
          << "Entry in CSV is not valid (12 field values expected): " << line;

      if (normalize) {
        for (int i = 0; i < entries.size(); i++) {
          std::string normalized;
          auto status = internal::normalizeUTF8(entries[i], normalized,
                                                normalizationForm);
          CHECK(status.ok()) << "Cannot normalize string " << entries[i];
          entries[i] = normalized;
        }
      }
      allLines.push_back(entries);
    }
  }

  // TODO(builder): continue here
  // build fst

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
