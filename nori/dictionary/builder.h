#ifndef __NORI_DICTIONARY_BUILDER_H__
#define __NORI_DICTIONARY_BUILDER_H__

#include <functional>
#include <memory>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"

namespace nori {
namespace dictionary {
namespace builder {

namespace internal {

// list all files in the directory. This function returns paths as sorted order.
// If functor returns false for given paths, this function will filter them.
//
// Example:
//   internal::listDirectory("./testdata/listDirectory", paths,
//     [](absl::string_view path) { return true; });
void listDirectory(absl::string_view directory, std::vector<std::string>& paths,
                   std::function<bool(std::string)> functor);

// Parse csv from raw string `line` and put outputs into `entries`.
void parsCSVLine(std::string line, std::vector<std::string>& entries);

}  // namespace internal

// Abstract class for interfaces that builds each type (unknown, csvs, or costs)
// of dictionaries in MeCab's.
class IDictionaryBuilder {
 public:
  virtual ~IDictionaryBuilder() = default;
  virtual absl::Status parse(absl::string_view inputDirectory) = 0;
  virtual absl::Status save(absl::string_view outputDirectory) = 0;
};

// Build Nori Dictionary from MeCab Dictionary.
class MeCabDictionaryBuilder {
 public:
  // An argument `normalize` means whether to normalize terms in dictionary.
  MeCabDictionaryBuilder(bool normalize, const std::string normalizationForm);
  ~MeCabDictionaryBuilder();

  // This method builds dictionary from `inputDirectory` and write built
  // dictionary to `outputDirectory`.
  void build(absl::string_view inputDirectory,
             absl::string_view outputDirectory);

 private:
  std::vector<std::unique_ptr<IDictionaryBuilder>> builders;
};

// Read all csv files in MeCab dictionary directory, and convert it to nori's
// FST dictionary format.
class TokenInfoDictionaryBuilder : public IDictionaryBuilder {
 public:
  TokenInfoDictionaryBuilder(const bool normalize,
                             const std::string normalizationForm)
      : normalize(normalize), normalizationForm(normalizationForm) {}
  ~TokenInfoDictionaryBuilder() {}

  absl::Status parse(absl::string_view inputDirectory);
  absl::Status save(absl::string_view outputDirectory);

 private:
  bool normalize;
  const std::string normalizationForm;
};

// Read unk.def and char.def and convert them to nori's dictionary format.
class UnknownDictionaryBuilder : public IDictionaryBuilder {
 public:
  ~UnknownDictionaryBuilder() {}

  absl::Status parse(absl::string_view inputDirectory);
  absl::Status save(absl::string_view outputDirectory);
};

// Read matrix.def and convert it to nori's dictionary format.
class ConnectionCostsBuilder : public IDictionaryBuilder {
 public:
  ~ConnectionCostsBuilder() {}

  absl::Status parse(absl::string_view inputDirectory);
  absl::Status save(absl::string_view outputDirectory);
};

}  // namespace builder
}  // namespace dictionary
}  // namespace nori

#endif  // __NORI_DICTIONARY_BUILDER_H__
