#ifndef __NORI_DICTIONARY_BUILDER_H__
#define __NORI_DICTIONARY_BUILDER_H__

#include <darts.h>

#include <functional>
#include <memory>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "nori/lib/protos/dictionary.pb.h"

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
                                  nori::protos::Morpheme* morpheme);

}  // namespace internal

class DictionaryBuilder {
 public:
  DictionaryBuilder(bool normalize, const std::string normalizationForm)
      : normalize(normalize), normalizationForm(normalizationForm) {}

  ~DictionaryBuilder() = default;
  absl::Status build(absl::string_view inputDirectory);
  absl::Status save(std::string outputFilename);

 private:
  absl::Status buildTokenInfos(absl::string_view inputDirectory);
  absl::Status buildUnknownTokenInfos(absl::string_view inputDirectory);
  absl::Status buildConnectionCost(absl::string_view inputDirectory);
  absl::Status findLeftRightIds(absl::string_view inputDirectory);

  nori::protos::Dictionary noriDictionary;

  bool normalize;
  const std::string normalizationForm;
};

}  // namespace builder
}  // namespace dictionary
}  // namespace nori

#endif  // __NORI_DICTIONARY_BUILDER_H__
