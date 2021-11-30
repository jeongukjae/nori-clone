#ifndef __NORI_DICTIONARY_BUILDER_H__
#define __NORI_DICTIONARY_BUILDER_H__

#include <darts.h>

#include <functional>
#include <memory>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "nori/protos/dictionary.pb.h"

namespace nori {
namespace dictionary {
namespace builder {

namespace internal {

// Convert MeCab's csv row into morpheme proto
//
// mecab-ko-dic features
//
//  * 0   - surface
//  * 1   - left cost
//  * 2   - right cost
//  * 3   - word cost
//  * 4   - part of speech0+part of speech1+...
//  * 5   - semantic class
//  * 6   - T if the last character of the surface form has a coda, F otherwise
//  * 7   - reading
//  * 8   - POS type (*, Compound, Inflect, Preanalysis)
//  * 9   - left POS
//  * 10  - right POS
//  * 11  - expression
absl::Status convertMeCabCSVEntry(const std::vector<std::string>& entry,
                                  nori::Dictionary::Morpheme* morpheme);

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
  std::unique_ptr<Darts::DoubleArray> trie;
  nori::Dictionary dictionary;
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
