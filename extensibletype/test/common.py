import numpy as np

def draw_hashes(rng, nitems):
    hashes = rng.randint(2**32, size=nitems).astype(np.uint64)
    hashes <<= 32
    hashes |= rng.randint(2**32, size=nitems).astype(np.uint64)
    return hashes


