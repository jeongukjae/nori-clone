# `nori-clone` C++ API

## Dependency setting for `nori-clone`

In `WORKSPACE`, you can fetch nori-clone like below.

```python
http_archive(
    name = "com_github_jeongukjae_nori_clone",
    strip_prefix = "nori-clone-<COMMIT_HASH>",
    url = "https://github.com/jeongukjae/nori-clone/archive/<COMMIT_HASH>.zip",
)

load("@com_github_jeongukjae_nori_clone//nori:workspace.bzl", "nori_workspace")

nori_workspace()

# Register protobuf
http_archive(
    name = "rules_proto",
    sha256 = "66bfdf8782796239d3875d37e7de19b1d94301e8972b3cbd2446b332429b4df1",
    strip_prefix = "rules_proto-4.0.0",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_proto/archive/refs/tags/4.0.0.tar.gz",
        "https://github.com/bazelbuild/rules_proto/archive/refs/tags/4.0.0.tar.gz",
    ],
)

load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")

rules_proto_dependencies()

rules_proto_toolchains()
```

After that, you can use `nori-clone` c++ lib in any `BUILD` files.

```python
cc_binary(
    name = "example_cc_main",
    srcs = ['example_cc_main.cc'],
    deps =['@com_github_jeongukjae_nori_clone//nori/lib:nori']
)
```

## Usage

```c++
#include "nori/lib/tokenizer.h"
#include "nori/lib/dictionary/dictionary.h"

nori::dictionary::Dictionary dictionary;
auto status = dictionary.loadPrebuilt("./dictionary");
CHECK(status.ok()) << status.message();
status = dictionary.loadUser("./dictionary/userdict.txt");
CHECK(status.ok()) << status.message();

const nori::NoriTokenizer tokenizer(&dictionary);
nori::Lattice lattice("이 프로젝트는 nori를 재작성하는 프로젝트입니다.");

status = tokenizer.tokenize(lattice);
ASSERT_TRUE(status.ok());

for (int i = 0; i < lattice.getTokens()->size(); i++) {
    LOG(INFO) << lattice.getTokens()->at(i).surface;
}
```
