import time
from sys import argv, stdin

from kiwipiepy import Kiwi

kiwi = Kiwi()

def run_with_iterator(f):
    for line in f:
        kiwi.tokenize(line)


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
