import time
from sys import argv, stdin

import nori

tokenizer = nori.NoriTokenizer()
tokenizer.load_prebuilt_dictionary("./dictionary/latest-dictionary.nori")
tokenizer.load_user_dictionary("./dictionary/latest-userdict.txt")

def run_with_iterator(f):
    for line in f:
        tokenizer.tokenize(line)


if len(argv) != 1:
    # read all lines from the input file
    with open(argv[1]) as f:
        lines = f.readlines()[:int(argv[2])]

    start_time = time.time()
    run_with_iterator(lines)
else:
    start_time = time.time()
    run_with_iterator(stdin)

end_time = time.time()
time_diff = (end_time - start_time)
print(int(time_diff * 1000))
