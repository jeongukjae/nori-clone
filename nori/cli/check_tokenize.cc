#include <gflags/gflags.h>
#include <glog/logging.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <chrono>
#include <fstream>

#include "absl/strings/str_cat.h"
#include "nori/dictionary/dictionary.h"
#include "nori/graphviz_visualize.h"
#include "nori/protos/dictionary.pb.h"
#include "nori/tokenizer.h"
#include "nori/utils.h"

DEFINE_string(dictionary, "/Users/jeongukjae/Documents/GitHub/nori/dictionary",
              "Path to nori dictionary");
DEFINE_string(input,
              "Nori-clone은 C++로 Nori를 재작성하기 위한 프로젝트입니다.",
              "Text to analyze");
DEFINE_string(output, "nori.dot", "Output path for dotfile");
DEFINE_int32(n_repeat, 1000, "num repeats");

int main(int argc, char** argv) {
  FLAGS_alsologtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  gflags::SetUsageMessage("Check nori dictionary files");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  nori::dictionary::Dictionary dictionary;
  auto status = dictionary.load(FLAGS_dictionary);
  CHECK(status.ok()) << status.message();

  nori::NoriTokenizer tokenizer(&dictionary);
  LOG(INFO) << "Input message: " << FLAGS_input;
  nori::Lattice lattice;

  std::chrono::system_clock::time_point start =
      std::chrono::system_clock::now();

  for (int i = 0; i < FLAGS_n_repeat; i++) {
    lattice.clear();
    lattice.setSentence(FLAGS_input);
    tokenizer.tokenize(lattice).IgnoreError();
  }

  std::chrono::milliseconds elapsedMs =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now() - start);

  LOG(INFO) << "Elapsed: " << elapsedMs.count() << "ms, Tokenization Result. ";
  for (const auto& token : *lattice.getTokens()) {
    std::string posTagStr = "";
    for (const auto& postag : token->morpheme->postag()) {
      absl::StrAppend(&posTagStr, nori::POSTag_Name(postag), "+");
    }
    posTagStr.pop_back();

    LOG(INFO) << token->surface << ", " << posTagStr;
  }

  LOG(INFO) << "Done.";

  google::protobuf::ShutdownProtobufLibrary();
}
