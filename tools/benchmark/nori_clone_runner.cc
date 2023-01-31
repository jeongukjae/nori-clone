#include <sys/stat.h>
#include <sys/types.h>

#include <chrono>
#include <fstream>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/check.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "nori/lib/dictionary/dictionary.h"
#include "nori/lib/graphviz_visualize.h"
#include "nori/lib/protos/dictionary.pb.h"
#include "nori/lib/tokenizer.h"
#include "nori/lib/utils.h"

ABSL_FLAG(std::string, dictionary, "./dictionary/latest-dictionary.nori",
          "Path to nori dictionary");
ABSL_FLAG(std::string, user_dictionary, "./dictionary/latest-userdict.txt",
          "Path to nori user dictionary");
ABSL_FLAG(std::string, input, "./tools/benchmark/data.txt",
          "Text file to analyze");
ABSL_FLAG(int, n, 1000, "n lines");

int main(int argc, char** argv) {
  absl::SetProgramUsageMessage("Benchmark nori tokenizer");
  absl::ParseCommandLine(argc, argv);

  absl::InitializeLog();

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  auto dictionaryFlag = absl::GetFlag(FLAGS_dictionary);
  auto userDictionaryFlag = absl::GetFlag(FLAGS_user_dictionary);
  auto inputFlag = absl::GetFlag(FLAGS_input);
  auto nFlag = absl::GetFlag(FLAGS_n);

  nori::dictionary::Dictionary dictionary;
  auto status = dictionary.loadPrebuilt(dictionaryFlag);
  CHECK(status.ok()) << status.message();
  if (userDictionaryFlag != "") {
    status = dictionary.loadUser(userDictionaryFlag);
    CHECK(status.ok()) << status.message();
  }

  nori::NoriTokenizer tokenizer(&dictionary);
  auto normalizer = dictionary.getNormalizer();
  nori::Lattice lattice;
  status = lattice.setSentence(inputFlag, normalizer);
  CHECK(status.ok()) << status.message();

  std::vector<std::string> lines;
  {
    std::ifstream ifs(inputFlag);
    CHECK(ifs.good()) << "Cannot open " << inputFlag;
    std::string line;
    int c = 0;
    while (std::getline(ifs, line)) {
      lines.push_back(line);
      if (c++ == nFlag) {
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
