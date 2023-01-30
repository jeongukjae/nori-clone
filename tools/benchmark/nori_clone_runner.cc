#include <gflags/gflags.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <chrono>
#include <fstream>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "nori/lib/dictionary/dictionary.h"
#include "nori/lib/graphviz_visualize.h"
#include "nori/lib/protos/dictionary.pb.h"
#include "nori/lib/tokenizer.h"
#include "nori/lib/utils.h"

DEFINE_string(dictionary, "./dictionary/latest-dictionary.nori",
              "Path to nori dictionary");
DEFINE_string(user_dictionary, "./dictionary/latest-userdict.txt",
              "Path to nori user dictionary");
DEFINE_string(input, "./tools/benchmark/data.txt", "Text file to analyze");
DEFINE_int32(n, 1000, "n lines");

int main(int argc, char** argv) {
  gflags::SetUsageMessage("Check nori dictionary files");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  nori::dictionary::Dictionary dictionary;
  auto status = dictionary.loadPrebuilt(FLAGS_dictionary);
  CHECK(status.ok()) << status.message();
  if (FLAGS_user_dictionary != "") {
    status = dictionary.loadUser(FLAGS_user_dictionary);
    CHECK(status.ok()) << status.message();
  }

  nori::NoriTokenizer tokenizer(&dictionary);
  auto normalizer = dictionary.getNormalizer();
  nori::Lattice lattice;
  status = lattice.setSentence(FLAGS_input, normalizer);
  CHECK(status.ok()) << status.message();

  std::vector<std::string> lines;
  {
    std::ifstream ifs(FLAGS_input);
    CHECK(ifs.good()) << "Cannot open " << FLAGS_input;
    std::string line;
    int c = 0;
    while (std::getline(ifs, line)) {
      lines.push_back(line);
      if (c++ == FLAGS_n) {
        break;
      }
    }
  }

  std::chrono::system_clock::time_point start =
      std::chrono::system_clock::now();

  for (const auto& line : lines) {
    nori::Lattice lattice;
    lattice.setSentence(line, normalizer).IgnoreError();
    tokenizer.tokenize(lattice).IgnoreError();
  }

  std::chrono::milliseconds elapsedMs =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now() - start);

  std::cout << elapsedMs.count() << std::endl;
  google::protobuf::ShutdownProtobufLibrary();
}
