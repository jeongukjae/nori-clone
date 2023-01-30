#include <sys/stat.h>
#include <sys/types.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "nori/lib/dictionary/builder.h"
#include "nori/lib/protos/dictionary.pb.h"
#include "nori/lib/utils.h"

ABSL_FLAG(std::string, mecab_dic, "",
          "Path to mecab dictionary. This cli program reads "
          "{matrix,char,unk}.def and all CSV files");
ABSL_FLAG(std::string, output, "./dictionary.nori",
          "output filename for nori dictionary");
ABSL_FLAG(std::string, normalization_form, "NFKC",
          "Unicode normalization form for dictionary of MeCab");
ABSL_FLAG(bool, normalize, true, "whether to normalize dictionary of MeCab");

int main(int argc, char** argv) {
  absl::SetProgramUsageMessage(
      "Build Nori dictionary from MeCab's dictionary.");
  absl::ParseCommandLine(argc, argv);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  auto mecabDicFlag = absl::GetFlag(FLAGS_mecab_dic);
  auto outputFlag = absl::GetFlag(FLAGS_output);

  CHECK(nori::utils::isDirectory(mecabDicFlag))
      << "Cannot find directory " << mecabDicFlag;
  LOG(INFO) << "MeCab dictionary path: " << mecabDicFlag;
  LOG(INFO) << "Output path: " << outputFlag;

  nori::dictionary::builder::DictionaryBuilder builder(
      absl::GetFlag(FLAGS_normalize), absl::GetFlag(FLAGS_normalization_form));
  auto status = builder.build(mecabDicFlag);
  CHECK(status.ok()) << status.message();
  status = builder.save(outputFlag);
  CHECK(status.ok()) << status.message();

  google::protobuf::ShutdownProtobufLibrary();
  LOG(INFO) << "Done.";
}
