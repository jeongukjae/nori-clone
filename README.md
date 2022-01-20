# nori

Standalone Nori (Korean Morphological Analyzer in Apache Lucene) written in C++.

## Introduction

ElasticSearch provides high-quality/performance Korean morphological analyzer `nori`. But `nori`'s code is strongly coupled with the Lucene codebase, and `nori` is written in Java that is the main language in the Lucene project. So, it's hard to use `nori` standalone in Python or Golang with the same performance. Therefore, I re-implemented almost the same algorithms with `nori` in Lucene using C++ for the portability and usability.

## Usage

This project is written in C++, but also provides Python and Golang binding.

### C++

```cpp
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

### Python

See [./nori/python](./nori/python/README.md)

### Golang

```golang
import (
    "fmt"

    nori "github.com/jeongukjae/nori-clone/nori/go"
)

tokenizer, err := nori.New("./dictionary", "./dictionary/userdict.txt")
if err != nil {
    // ...
}
defer tokenizer.Free()

// False means do not normalize sentence
tokens, err := tokenizer.Tokenize("이 프로젝트는 nori를 재작성하는 프로젝트입니다.", false)
if err != nil {
    // ...
}

for _, token := range *tokens {
    fmt.Println(token.Surface)
}
```

## Build and test

```sh
bazel build //...
bazel test //...
```

## References

* <https://www.elastic.co/kr/blog/nori-the-official-elasticsearch-plugin-for-korean-language-analysis>
* <https://github.com/apache/lucene/tree/main/lucene/analysis/nori>
* <https://gritmind.blog/2020/07/22/nori_deep_dive/>
* <https://github.com/gritmind/python-nori>
