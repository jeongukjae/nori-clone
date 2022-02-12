# `nori-clone` Golang binding

## Usage

### Installation

First, since you need to install `libnori_c` for `nori-clone` to bind golang codes, you can build like below.

```sh
$ bazel build //nori/c:libnori_c
INFO: Analyzed target //nori/c:libnori_c (0 packages loaded, 0 targets configured).
INFO: Found 1 target...
Target //nori/c:libnori_c up-to-date:
  bazel-bin/nori/c/libnori_c.tar.gz
...
```

Then install library and header.

```sh
sudo tar -C /usr/local -xzf bazel-bin/nori/c/libnori_c.tar.gz
sudo ldconfig /usr/local/lib
```

Or, you can install in non-system directory.

```sh
export LIBRARY_PATH=$LIBRARY_PATH:/path/to/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/lib
# in MacOS
# export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/path/to/lib
```

After the installation of `libnori_c`, you can install golang binding.

```sh
go get https://github.com/jeongukjae/nori-clone/tree/main/nori/go
```

### How to use

```golang
import (
    "fmt"

    nori "github.com/jeongukjae/nori-clone/nori/go"
)

// Pass empty string if you don't want to load user dictionary
tokenizer, err := nori.New("./dictionary/latest-dictionary.nori", "./dictionary/latest-userdict.txt")
if err != nil {
    // ...
}
defer tokenizer.Free()

tokens, err := tokenizer.Tokenize("이 프로젝트는 nori를 재작성하는 프로젝트입니다.")
if err != nil {
    // ...
}

for _, token := range *tokens {
    fmt.Println(token.Surface)
}
```

<!-- TODO(jeongukjae): add description -->
