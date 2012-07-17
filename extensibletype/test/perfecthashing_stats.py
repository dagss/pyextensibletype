from __future__ import division
from extensibletype import extensibletype
from extensibletype.test.common import draw_hashes
import numpy as np
import multiprocessing


def try_perfect_hash(entry_count, slot_count, ntrials):
    failures = 0
    for i in xrange(ntrials):
        prehashes = draw_hashes(np.random, entry_count)
        try:
            p, r, m_f, m_g, d = extensibletype.perfect_hash(prehashes, slot_count,
                                                            repeat=1)
        except RuntimeError:
            failures += 1
    return failures
    
if __name__ == '__main__':
    ntrials = 10**4
    ns = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
              19, 20, 30, 31, 32, 33, 34, 62, 63, 64, 65, 66, 126, 127, 128,
              129]
#    ns = [33]
    if 0:
        for n in ns:
            first_nslots = extensibletype.roundup_2pow(n)
            for nslots in [first_nslots, 2 * first_nslots]:
                failure_count = try_perfect_hash(n, nslots, ntrials)
                print '%3d %3d   %.2e' % (n, nslots, failure_count / ntrials)

    failure_count = try_perfect_hash(32, 32, 10**7)
    print '%3d %3d   %.2e' % (n, nslots, failure_count / ntrials)
            
