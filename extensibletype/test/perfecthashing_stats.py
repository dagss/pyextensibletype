from __future__ import division
from extensibletype import extensibletype
from extensibletype.test.common import draw_hashes
import numpy as np

ntrials = 10**6

for n in [7]:#2, 4, 8, 16, 32, 64, 128]: 
    failures = 0
    for j in range(ntrials):
        prehashes = draw_hashes(np.random, n)
        try:
            p, r, m_f, m_g, d = extensibletype.perfect_hash(prehashes, repeat=1)
        except RuntimeError:
            failures += 1
    print n, failures / ntrials
