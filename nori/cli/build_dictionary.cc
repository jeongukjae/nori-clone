#include <gflags/gflags.h>
#include <glog/logging.h>

#include "absl/strings/str_cat.h"
#include "nori/dictionary/builder.h"
#include "nori/protos/dictionary.pb.h"
#include "nori/utils.h"

DEFINE_string(mecab_dic, "",
              "Path to mecab dictionary. This cli program reads "
              "{matrix,char,unk}.def and all CSV files");
DEFINE_string(output, "./output", "output directory for nori dictionary");
DEFINE_string(normalization_form, "NFKC",
              "Unicode normalization form for dictionary of MeCab");
DEFINE_bool(normalize, true, "whether to normalize dictionary of MeCab");

int main(int argc, char** argv) {
  FLAGS_alsologtostderr = 1;
  google::InitGoogleLogging(argv[0]);
  gflags::SetUsageMessage("Build Nori dictionary from MeCab's dictionary.");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  CHECK(nori::utils::isDirectory(FLAGS_mecab_dic))
      << "Cannot find directory " << FLAGS_mecab_dic;
  LOG(INFO) << "MeCab dictionary path: " << FLAGS_mecab_dic;
  // TODO(jeongukjae) mkdir FLAGS_output

  nori::dictionary::builder::MeCabDictionaryBuilder builder(
      FLAGS_normalize, FLAGS_normalization_form);
  builder.build(FLAGS_mecab_dic, FLAGS_output);

  google::protobuf::ShutdownProtobufLibrary();
}
