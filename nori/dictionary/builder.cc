#include "nori/dictionary/builder.h"

#include <dirent.h>
#include <glog/logging.h>

#include <algorithm>
#include <fstream>

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"

namespace nori {
namespace dictionary {
namespace builder {

namespace internal {

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

}  // namespace internal

// MeCabDictionary class

MeCabDictionaryBuilder::MeCabDictionaryBuilder(const bool normalize) {
  builders.push_back(new TokenInfoDictionaryBuilder(normalize));
  builders.push_back(new UnknownDictionaryBuilder());
  builders.push_back(new ConnectionCostsBuilder());
}

MeCabDictionaryBuilder::~MeCabDictionaryBuilder() {
  for (auto builder : builders) delete builder;
  builders.clear();
}

void MeCabDictionaryBuilder::build(absl::string_view inputDirectory,
                                   absl::string_view outputDirectory) {
  for (auto builder : builders) {
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

  for (const auto& path : paths) {
    std::ifstream ifs(path);
    CHECK(!ifs.fail()) << path << " is missing";

    // TODO(jay)
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
