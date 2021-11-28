#ifndef __NORI_DICTIONARY_BUILDER_H__
#define __NORI_DICTIONARY_BUILDER_H__

#include <vector>

#include "absl/strings/string_view.h"

namespace nori {
namespace dictionary {
namespace builder {

// Abstract class for interfaces that builds each type (unknown, csvs, or costs)
// of dictionaries in MeCab's.
class DictionaryBuilder {
 public:
  virtual void parse(const absl::string_view inputDirectory) = 0;
  virtual void save(const absl::string_view outputDirectory) = 0;
};

// Build Nori Dictionary from MeCab Dictionary.
class MeCabDictionaryBuilder {
 public:
  MeCabDictionaryBuilder();
  MeCabDictionaryBuilder(std::vector<DictionaryBuilder> builders);

  // This method builds dictionary from `inputDirectory` and write built
  // dictionary to `outputDirectory`. An argument `normalize` means whether to
  // normalize terms in dictionary or not.
  void build(const absl::string_view inputDirectory,
             const absl::string_view outputDirectory, const bool normalize);

 private:
};

// Read all csv files in MeCab dictionary directory, and convert it to nori's
// FST dictionary format.
class TokenInfoDictionaryBuilder : DictionaryBuilder {
 public:
  TokenInfoDictionaryBuilder(const bool normalize);
};

// Read unk.def and char.def and convert them to nori's dictionary format.
class UnknownDictionaryBuilder : DictionaryBuilder {
 public:
  UnknownDictionaryBuilder();
};

// Read matrix.def and convert it to nori's dictionary format.
class ConnectionCostsBuilder : DictionaryBuilder {
 public:
  ConnectionCostsBuilder();
};

}  // namespace builder
}  // namespace dictionary
}  // namespace nori

#endif  // __NORI_DICTIONARY_BUILDER_H__
