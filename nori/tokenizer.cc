#include "nori/tokenizer.h"

#include <darts.h>
#include <glog/logging.h>

#include <queue>
#include <vector>

#include "nori/protos/dictionary.pb.h"

typedef Darts::DoubleArray::result_pair_type DartsResults;

namespace nori {

namespace internal {

struct PathValue {
  size_t position = 0;
  size_t cost = 0;

  PathValue(size_t position, size_t cost) : position(position), cost(cost) {}
};

inline int getSpacePenalty(nori::POSTag tag) {
  switch (tag) {
    case nori::POSTag::E:
    case nori::POSTag::J:
    case nori::POSTag::VCP:
    case nori::POSTag::XSA:
    case nori::POSTag::XSN:
    case nori::POSTag::XSV:
      return 3000;
  }
  return 0;
}

}  // namespace internal

// NoriTokenizer class

absl::Status NoriTokenizer::tokenize(const std::string& text,
                                     Lattice* lattice) const {
  return absl::OkStatus();
}

}  // namespace nori
