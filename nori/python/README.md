# `nori-clone` Python binding

## Usage

### Installation

```sh
pip install nori-clone
```

### Tutorial

```python
import nori

dictionary = nori.Dictionary()
dictionary.load_prebuilt_dictionary("./dictionary")
dictionary.load_user_dictionary("./dictionary/userdict.txt")
tokenizer = nori.NoriTokenizer(dictionary)

# False means do not normalize sentence
result = tokenizer.tokenize("이 프로젝트는 nori를 재작성하는 프로젝트입니다.", False)

for token in result.tokens:
    print(token.surface)
```

<!-- TODO(jeongukjae): add description -->
