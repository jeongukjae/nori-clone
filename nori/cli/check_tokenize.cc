#include <sys/stat.h>
#include <sys/types.h>

#include <chrono>
#include <fstream>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/check.h"
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
ABSL_FLAG(std::string, input,
          "Nori-clone은 c++로 Nori를 재작성하기 위한 프로젝트입니다.",
          "Text to analyze");
ABSL_FLAG(int, n_repeat, 1000, "num repeats");
ABSL_FLAG(bool, print_output, false, "print output");

int main(int argc, char** argv) {
  absl::SetProgramUsageMessage("Check nori dictionary files");
  absl::ParseCommandLine(argc, argv);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  auto inputFlag = absl::GetFlag(FLAGS_input);
  auto dictionaryFlag = absl::GetFlag(FLAGS_dictionary);
  auto userDictionaryFlag = absl::GetFlag(FLAGS_user_dictionary);

  nori::dictionary::Dictionary dictionary;
  auto status = dictionary.loadPrebuilt(dictionaryFlag);
  CHECK(status.ok()) << status.message();
  if (userDictionaryFlag != "") {
    LOG(INFO) << "Read user dictionary: " << userDictionaryFlag;
    status = dictionary.loadUser(userDictionaryFlag);
    CHECK(status.ok()) << status.message();
  }

  nori::NoriTokenizer tokenizer(&dictionary);
  auto normalizer = dictionary.getNormalizer();
  LOG(INFO) << "Input message: " << inputFlag;
  nori::Lattice lattice;
  status = lattice.setSentence(inputFlag, normalizer);
  CHECK(status.ok()) << status.message();

  std::chrono::system_clock::time_point start =
      std::chrono::system_clock::now();

  for (int i = 0; i < absl::GetFlag(FLAGS_n_repeat); i++) {
    lattice.clear();
    lattice.setSentence(inputFlag, normalizer).IgnoreError();
    tokenizer.tokenize(lattice).IgnoreError();
  }

  std::chrono::microseconds elapsedMs =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now() - start);

  LOG(INFO) << "Elapsed: " << elapsedMs.count() << " micro seconds. ";

  if (absl::GetFlag(FLAGS_print_output)) {
    LOG(INFO) << "Tokenization Results.";
    for (const auto& token : *lattice.getTokens()) {
      std::vector<std::string> posTagStr;
      for (const auto& postag : token.morpheme->pos_tags()) {
        posTagStr.push_back(nori::protos::POSTag_Name(postag));
      }

      LOG(INFO) << token.surface << ", " << absl::StrJoin(posTagStr, "+");
    }
  }

  LOG(INFO) << "Done.";

  google::protobuf::ShutdownProtobufLibrary();
}
