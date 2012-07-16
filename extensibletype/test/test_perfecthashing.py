from nose.tools import eq_, ok_
import numpy as np
from .. import extensibletype
from .common import *


def draw_hashes(rng, nitems):
    hashes = rng.randint(2**32, size=nitems).astype(np.uint64)
    hashes <<= 32
    hashes |= rng.randint(2**32, size=nitems).astype(np.uint64)
    return hashes

def roundup(x):
    x -= 1
    x |= x >> 1
    x |= x >> 2
    x |= x >> 4
    x |= x >> 8
    x |= x >> 16
    x |= x >> 32
    x += 1
    return x

def test_binsort():
    nbins = 64
    p = np.zeros(nbins, dtype=np.uint16)
    binsizes = np.random.randint(0, 7, size=nbins).astype(np.uint8)
    num_by_size = np.zeros(8, dtype=np.uint8)
    x = np.bincount(binsizes).astype(np.uint8)
    num_by_size[:x.shape[0]] = x
    extensibletype.bucket_argsort(p, binsizes, num_by_size)
    assert np.all(sorted(binsizes) == binsizes[p][::-1])

def test_basic():
    n=64
    prehashes = draw_hashes(np.random, n)
    p, r, m_f, m_g, d = extensibletype.perfect_hash(prehashes, repeat=1)
    print r
    hashes = ((prehashes >> r) & m_f) ^ d[prehashes & m_g]
    print p
    print d
    hashes.sort()
    print hashes
    assert len(hashes) == len(np.unique(hashes))
    
