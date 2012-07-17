import numpy as np

def draw_hashes(rng, nitems): 
    hashes = np.random.randint(-2**63, 2**63-1, size=nitems).astype(np.uint64)
    return hashes


