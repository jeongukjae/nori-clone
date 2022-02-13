import time
from sys import argv, stdin

from pynori.korean_analyzer import KoreanAnalyzer

nori = KoreanAnalyzer(
    decompound_mode='DISCARD', # DISCARD or MIXED or NONE
    infl_decompound_mode='DISCARD', # DISCARD or MIXED or NONE
    discard_punctuation=True,
    output_unknown_unigrams=False,
    pos_filter=False, stop_tags=['JKS', 'JKB', 'VV', 'EF'],
    synonym_filter=False, mode_synonym='NORM', # NORM or EXTENSION
)

def run_with_iterator(f):
    for line in f:
        nori.do_analysis(line)


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
