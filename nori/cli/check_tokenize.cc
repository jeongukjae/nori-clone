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
DEFINE_string(input,
              "Nori-clone은 c++로 Nori를 재작성하기 위한 프로젝트입니다.",
              "Text to analyze");
DEFINE_string(output, "nori.dot", "Output path for dotfile");
DEFINE_int32(n_repeat, 1000, "num repeats");
DEFINE_bool(print_output, false, "print output");

int main(int argc, char** argv) {
  FLAGS_alsologtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  gflags::SetUsageMessage("Check nori dictionary files");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  nori::dictionary::Dictionary dictionary;
  auto status = dictionary.loadPrebuilt(FLAGS_dictionary);
  CHECK(status.ok()) << status.message();
  if (FLAGS_user_dictionary != "") {
    LOG(INFO) << "Read user dictionary: " << FLAGS_user_dictionary;
    status = dictionary.loadUser(FLAGS_user_dictionary);
    CHECK(status.ok()) << status.message();
  }

  nori::NoriTokenizer tokenizer(&dictionary);
  LOG(INFO) << "Input message: " << FLAGS_input;
  nori::Lattice lattice;
  status = lattice.setSentence(FLAGS_input);
  CHECK(status.ok()) << status.message();

  std::chrono::system_clock::time_point start =
      std::chrono::system_clock::now();

  for (int i = 0; i < FLAGS_n_repeat; i++) {
    lattice.clear();
    lattice.setSentence(FLAGS_input).IgnoreError();
    tokenizer.tokenize(lattice).IgnoreError();
  }

  std::chrono::microseconds elapsedMs =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now() - start);

  LOG(INFO) << "Elapsed: " << elapsedMs.count() << " micro seconds. ";

  if (FLAGS_print_output) {
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
