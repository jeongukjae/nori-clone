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
DEFINE_string(
    input,
    "처음 네 컬럼은 MeCab 형식의 공통 필드입니다. 첫 번째 컬럼은 "
    "단어(도서관)의 표층형(surface form)입니다. 마지막 컬럼은 분해된 "
    "모습(도서관(library) => 도서(book) + 관(house))을 보여줍니다. 노리가 "
    "입력을 분석할 때 가능한 모든 격자(lattice)를 만들기 위해 글자마다 이 "
    "사전을 찾아봐야 합니다. 처음 네 컬럼은 MeCab 형식의 공통 필드입니다. 첫 "
    "번째 컬럼은 단어(도서관)의 표층형(surface form)입니다. 마지막 컬럼은 "
    "분해된 모습(도서관(library) => 도서(book) + 관(house))을 보여줍니다. "
    "노리가 입력을 분석할 때 가능한 모든 격자(lattice)를 만들기 위해 글자마다 "
    "이 사전을 찾아봐야 합니다. 처음 네 컬럼은 MeCab 형식의 공통 필드입니다. "
    "첫 번째 컬럼은 단어(도서관)의 표층형(surface form)입니다. 마지막 컬럼은 "
    "분해된 모습(도서관(library) => 도서(book) + 관(house))을 보여줍니다. "
    "노리가 입력을 분석할 때 가능한 모든 격자(lattice)를 만들기 위해 글자마다 "
    "이 사전을 찾아봐야 합니다. 처음 네 컬럼은 MeCab 형식의 공통 필드입니다. "
    "첫 번째 컬럼은 단어(도서관)의 표층형(surface form)입니다. 마지막 컬럼은 "
    "분해된 모습(도서관(library) => 도서(book) + 관(house))을 보여줍니다. "
    "노리가 입력을 분석할 때 가능한 모든 격자(lattice)를 만들기 위해 글자마다 "
    "이 사전을 찾아봐야 합니다. 처음 네 컬럼은 MeCab 형식의 공통 필드입니다. "
    "첫 번째 컬럼은 단어(도서관)의 표층형(surface form)입니다. 마지막 컬럼은 "
    "분해된 모습(도서관(library) => 도서(book) + 관(house))을 보여줍니다. "
    "노리가 입력을 분석할 때 가능한 모든 격자(lattice)를 만들기 위해 글자마다 "
    "이 사전을 찾아봐야 합니다. 처음 네 컬럼은 MeCab 형식의 공통 필드입니다. "
    "첫 번째 컬럼은 단어(도서관)의 표층형(surface form)입니다. 마지막 컬럼은 "
    "분해된 모습(도서관(library) => 도서(book) + 관(house))을 보여줍니다. "
    "노리가 입력을 분석할 때 가능한 모든 격자(lattice)를 만들기 위해 글자마다 "
    "이 사전을 찾아봐야 합니다. 처음 네 컬럼은 MeCab 형식의 공통 필드입니다. "
    "첫 번째 컬럼은 단어(도서관)의 표층형(surface form)입니다. 마지막 컬럼은 "
    "분해된 모습(도서관(library) => 도서(book) + 관(house))을 보여줍니다. "
    "노리가 입력을 분석할 때 가능한 모든 격자(lattice)를 만들기 위해 글자마다 "
    "이 사전을 찾아봐야 합니다.",
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
  auto status = dictionary.load(FLAGS_dictionary);
  CHECK(status.ok()) << status.message();

  nori::NoriTokenizer tokenizer(&dictionary);
  LOG(INFO) << "Input message: " << FLAGS_input;
  nori::Lattice lattice(FLAGS_input);

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

  LOG(INFO) << "Elapsed: " << elapsedMs.count() << "ms. ";

  if (FLAGS_print_output) {
    LOG(INFO) << "Tokenization Results.";
    for (const auto& token : *lattice.getTokens()) {
      std::string posTagStr = "";
      for (const auto& postag : token.morpheme->postag()) {
        absl::StrAppend(&posTagStr, nori::POSTag_Name(postag), "+");
      }
      posTagStr.pop_back();

      LOG(INFO) << token.surface << ", " << posTagStr;
    }
  }

  LOG(INFO) << "Done.";

  google::protobuf::ShutdownProtobufLibrary();
}
