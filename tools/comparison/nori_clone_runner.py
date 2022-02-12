import time
from sys import argv, stdin

import nori

dictionary = nori.Dictionary()
dictionary.load_prebuilt_dictionary("./dictionary/legacy-dictionary.nori")
dictionary.load_user_dictionary("./dictionary/legacy-userdict.txt")
tokenizer = nori.NoriTokenizer(dictionary)

def run_with_iterator(f):
    for line in f:
        # https://github.com/apache/lucene/blob/2e2c4818d10e7db3ba6fabf5c0db630ad2cb57b0/gradle/generation/nori.gradle#L74
        # disable NFKC normalization
        result = tokenizer.tokenize(line)

        print(line.rstrip())
        for token in result.tokens[1:-1]:
            print(f"{token.surface}, {token.postype}, {token.postag[0]}, {token.postag[-1]}")
        print()


if len(argv) != 1:
    # read all lines from the input file
    with open(argv[1]) as f:
        lines = f.readlines()

    start_time = time.time()
    run_with_iterator(lines)
else:
    start_time = time.time()
    run_with_iterator(stdin)

end_time = time.time()
time_diff = (end_time - start_time)
print("Elapsed time:", time_diff * 1000, "ms")
