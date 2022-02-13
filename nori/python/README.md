# `nori-clone` Python binding

## Usage

### Installation

```sh
pip install nori-clone
```

Pre-built pip packages are only provided for x86_64 macOS and linux.
But you can use `nori-clone` in M1 chips if you build this repo from source.

### Build from source

You can build pip package using Bazel.

```sh
$ bazel build //nori/python:build_pip_pkg
INFO: Analyzed target //nori/python:build_pip_pkg (0 packages loaded, 0 targets configured).
INFO: Found 1 target...
Target //nori/python:build_pip_pkg up-to-date:
  bazel-bin/nori/python/build_pip_pkg
...
$ ./bazel-bin/nori/python/build_pip_pkg dist
++ uname -s
++ tr A-Z a-z
...
++ date
+ echo Thu Jan 20 21:59:37 KST 2022 : '=== Output wheel file is in: ./dist'
Thu Jan 20 21:59:37 KST 2022 : === Output wheel file is in: ./dist
```

You can find your own wheel file in `./dist`.

### How to use

```python
import nori

dictionary = nori.Dictionary()
dictionary.load_prebuilt_dictionary("./dictionary/latest-dictionary.nori")
dictionary.load_user_dictionary("./dictionary/latest-userdict.txt")
tokenizer = nori.NoriTokenizer(dictionary)

result = tokenizer.tokenize("이 프로젝트는 nori를 재작성하는 프로젝트입니다.")

for token in result.tokens:
    print(token.surface)
```

You can check the inferface of Python binding [here(`./nori/bind.pyi`)](./nori/bind.pyi).

<!-- TODO(jeongukjae): add description -->
