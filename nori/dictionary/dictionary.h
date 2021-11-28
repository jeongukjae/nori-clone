#ifndef __NORI_DICTIONARY_H__
#define __NORI_DICTIONARY_H__

#include <absl/string/string_view.h>

#include "nori/constant.h"

namespace nori {
namespace dictionary {

class Morpheme {
 public:
  Morpheme(Tag posTag, absl::string_view surfaceForm) {
    this.posTag = posTag;
    this.surfaceForm = surfaceForm;
  }

 private:
  POS::Tag posTag;
  absl::string_view surfaceForm;
}

class Dictionary {
};

}  // namespace dictionary
}  // namespace nori

#endif  //__NORI_DICTIONARY_H__
