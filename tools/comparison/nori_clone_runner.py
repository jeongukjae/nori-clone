import time
from sys import argv, stdin

import nori

dictionary = nori.Dictionary()
dictionary.load_prebuilt_dictionary("./dictionary/legacy")
dictionary.load_user_dictionary("./dictionary/legacy/userdict.txt")
tokenizer = nori.NoriTokenizer(dictionary)

def run_with_iterator(f):
    for line in f:
        result = tokenizer.tokenize(line)

        print(line.rstrip())
        for token in result.tokens[1:-1]:
            print(f"{token.surface}, {token.postype}, {token.postag[0]}, {token.postag[-1]}")
        print()

start_time = time.time()

if len(argv) != 1:
    with open(argv[1]) as f:
        run_with_iterator(f)
else:
    run_with_iterator(stdin)

end_time = time.time()
time_diff = (end_time - start_time)
print("Elapsed time:", time_diff * 1000, "ms")
