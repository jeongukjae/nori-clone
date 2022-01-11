#ifndef __NORI_TOKEN_FILTER_H__
#define __NORI_TOKEN_FILTER_H__

namespace nori {
namespace token_filters {

class KoreanNumberFilter {
 public:
  KoreanNumberFilter(bool discardPunctuation);

 private:
};

}  // namespace token_filters
}  // namespace nori

#endif  // __NORI_TOKEN_FILTER_H__
