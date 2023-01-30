#include <gflags/gflags.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "nori/lib/dictionary/builder.h"
#include "nori/lib/protos/dictionary.pb.h"
#include "nori/lib/utils.h"

DEFINE_string(mecab_dic, "",
              "Path to mecab dictionary. This cli program reads "
              "{matrix,char,unk}.def and all CSV files");
DEFINE_string(output, "./dictionary.nori",
              "output filename for nori dictionary");
DEFINE_string(normalization_form, "NFKC",
              "Unicode normalization form for dictionary of MeCab");
DEFINE_bool(normalize, true, "whether to normalize dictionary of MeCab");

int main(int argc, char** argv) {
  gflags::SetUsageMessage("Build Nori dictionary from MeCab's dictionary.");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  CHECK(nori::utils::isDirectory(FLAGS_mecab_dic))
      << "Cannot find directory " << FLAGS_mecab_dic;
  LOG(INFO) << "MeCab dictionary path: " << FLAGS_mecab_dic;
  LOG(INFO) << "Output path: " << FLAGS_output;

  nori::dictionary::builder::DictionaryBuilder builder(
      FLAGS_normalize, FLAGS_normalization_form);
  auto status = builder.build(FLAGS_mecab_dic);
  CHECK(status.ok()) << status.message();
  status = builder.save(FLAGS_output);
  CHECK(status.ok()) << status.message();

  google::protobuf::ShutdownProtobufLibrary();
  LOG(INFO) << "Done.";
}
