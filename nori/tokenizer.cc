#include "nori/tokenizer.h"

#include <darts.h>
#include <glog/logging.h>

#include <queue>
#include <vector>

typedef Darts::DoubleArray::result_pair_type DartsResults;

namespace nori {

namespace internal {

struct PathValue {
  size_t position = 0;
  size_t cost = 0;

  PathValue(size_t position, size_t cost) : position(position), cost(cost) {}
};

}  // namespace internal

// NoriTokenizer class

absl::Status NoriTokenizer::tokenize(const std::string& text,
                                     Lattice* lattice) const {
  return absl::OkStatus();
}

}  // namespace nori
