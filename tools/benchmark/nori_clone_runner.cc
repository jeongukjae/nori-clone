#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <chrono>
#include <fstream>

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

int main(int argc, char** argv) {
  FLAGS_alsologtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  gflags::SetUsageMessage("Check nori dictionary files");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  nori::dictionary::Dictionary dictionary;
  LOG(INFO) << "Read pre-built dictionary: " << FLAGS_dictionary;
  auto status = dictionary.loadPrebuilt(FLAGS_dictionary);
  CHECK(status.ok()) << status.message();
  if (FLAGS_user_dictionary != "") {
    LOG(INFO) << "Read user dictionary: " << FLAGS_user_dictionary;
    status = dictionary.loadUser(FLAGS_user_dictionary);
    CHECK(status.ok()) << status.message();
  }

  nori::NoriTokenizer tokenizer(&dictionary);
  auto normalizer = dictionary.getNormalizer();
  LOG(INFO) << "Input file: " << FLAGS_input;
  nori::Lattice lattice;
  status = lattice.setSentence(FLAGS_input, normalizer);
  CHECK(status.ok()) << status.message();

  std::vector<std::string> lines;
  {
    std::ifstream ifs(FLAGS_input);
    CHECK(ifs.good()) << "Cannot open " << FLAGS_input;
    std::string line;
    while (std::getline(ifs, line)) {
      lines.push_back(line);
    }
  }

  LOG(INFO) << "Start benchmarking: " << FLAGS_input;
  std::chrono::system_clock::time_point start =
      std::chrono::system_clock::now();

  for (const auto& line : lines) {
    lattice.clear();
    lattice.setSentence(line, normalizer).IgnoreError();
    tokenizer.tokenize(lattice).IgnoreError();
  }

  std::chrono::microseconds elapsedMs =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now() - start);

  LOG(INFO) << "Elapsed: " << elapsedMs.count() << " micro seconds. ";
  google::protobuf::ShutdownProtobufLibrary();
  LOG(INFO) << "Done.";
}
