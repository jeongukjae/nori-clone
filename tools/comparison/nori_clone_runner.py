import time
from sys import argv, stdin

import nori

tokenizer = nori.NoriTokenizer()
tokenizer.load_prebuilt_dictionary("./dictionary/legacy-dictionary.nori")
tokenizer.load_user_dictionary("./dictionary/legacy-userdict.txt")

def run_with_iterator(f):
    for line in f:
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
