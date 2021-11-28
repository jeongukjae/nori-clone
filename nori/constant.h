#ifndef __NORI_CONSTANT_H__
#define __NORI_CONSTANT_H__

#include <absl/strings/string_view.h>

namespace nori {
namespace POS {
namespace Type {

enum {
  // A simple morpheme.
  MORPHEME,
  // Compound noun.
  COMPOUND,
  // Inflected token.
  INFLECT,
  // Pre-analysis token.
  PREANALYSIS,
};

}

//
class Tag {
 public:
  Tag(const int code, const absl::string_view description)
      : code(code), description(description) {}

  // Returns the code associated with the tag (as defined in pos-id.def)
  int getCode() const { return code; }

  // Returns the description associated with the tag.
  absl::string_view getDescription() const { return description; }

 private:
  const int code;
  const absl::string_view description;
};

constexpr Tag E(100, "Verbal endings");
constexpr Tag IC(110, "Interjection");
constexpr Tag J(120, "Ending Particle");
constexpr Tag MAG(130, "General Adverb");
constexpr Tag MAJ(131, "Conjunctive adverb");
constexpr Tag MM(140, "Modifier");
constexpr Tag NNG(150, "General Noun");
constexpr Tag NNP(151, "Proper Noun");
constexpr Tag NNB(152, "Dependent noun");
constexpr Tag NNBC(153, "Dependent noun");
constexpr Tag NP(154, "Pronoun");
constexpr Tag NR(155, "Numeral");
constexpr Tag SF(160, "Terminal punctuation");
constexpr Tag SH(161, "Chinese Characeter");
constexpr Tag SL(162, "Foreign language");
constexpr Tag SN(163, "Number");
constexpr Tag SP(164, "Space");
constexpr Tag SSC(165, "Closing brackets");
constexpr Tag SSO(166, "Opening brackets");
constexpr Tag SC(167, "Separator");
constexpr Tag SY(168, "Other symbol");
constexpr Tag SE(169, "Ellipsis");
constexpr Tag VA(170, "Adjective");
constexpr Tag VCN(171, "Negative designator");
constexpr Tag VCP(172, "Positive designator");
constexpr Tag VV(173, "Verb");
constexpr Tag VX(174, "Auxiliary Verb or Adjective");
constexpr Tag XPN(181, "Prefix");
constexpr Tag XR(182, "Root");
constexpr Tag XSA(183, "Adjective Suffix");
constexpr Tag XSN(184, "Noun Suffix");
constexpr Tag XSV(185, "Verb Suffix");
constexpr Tag UNKNOWN(999, "Unknown");
constexpr Tag UNA(-1, "Unknown");
constexpr Tag NA(-1, "Unknown");
constexpr Tag VSV(-1, "Unknown");

}  // namespace POS
}  // namespace nori

#endif  // __NORI_CONSTANT_H__
