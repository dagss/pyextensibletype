#include <stdint.h>
#include <stdlib.h>

typedef struct {
  uint64_t hash;
  uint32_t key_length; /* length of key */
  char *key_data;
} lookup_table_key_t;

typedef struct {
  uint16_t entry_count, slot_count, bin_count, m_f, m_g;
  uint8_t r;
  uint8_t flags;
  uint16_t d[0];
} lookup_table_header_t;


uint64_t PyCustomSlots_roundup_2pow(uint64_t x) {
  x--; x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8;
  x |= x >> 16; x |= x >> 32;
  return x + 1;
}

#define BIN_LIMIT 12

  
void _PyCustomSlots_bucket_argsort(uint16_t *p, uint8_t *binsizes,
                                   uint16_t *number_of_bins_by_size) {
  uint16_t *sort_bins[BIN_LIMIT];
  int binsize, ibin, nbins;
  nbins = 0;
  /* We know how many bins there are of each size, so place pointers
     for each size along on the output array p */
  for (binsize = BIN_LIMIT - 1; binsize >= 0; --binsize) {
    sort_bins[binsize] = p;
    p += number_of_bins_by_size[binsize];
    nbins += number_of_bins_by_size[binsize];
  }
  /* Then simply write indices to the bins */
  for (ibin = 0; ibin != nbins; ++ibin) {
    binsize = binsizes[ibin];
    sort_bins[binsize][0] = ibin;
    sort_bins[binsize]++;
  }
}

int _PyCustomSlots_FindDisplacements(lookup_table_header_t *table,
                                     uint64_t *hashes,
                                     uint8_t *binsizes,
                                     uint16_t *bins,
                                     uint16_t *binsizes_ordering,
                                     uint16_t *out_permutation,
                                     uint8_t *taken) {
  uint16_t *d = &table->d[0];
  uint16_t slot_count = table->slot_count;
  uint16_t bin_count = table->bin_count;
  uint64_t m_f = table->m_f;
  uint8_t r = table->r;
  int i, j, bin;

  /* Step 1: Validate that f is 1:1 in each bin */
  for (j = 0; j != bin_count; ++j) {
    int k, t;
    bin = binsizes_ordering[j];
    for (k = 0; k != binsizes[bin]; ++k) {
      for (t = k + 1; t < binsizes[bin]; ++t) {
        if (((hashes[bins[BIN_LIMIT * bin + k]] >> r) & m_f) ==
            ((hashes[bins[BIN_LIMIT * bin + t]] >> r) & m_f)) {
          return -1;
        }
      }
    }
  }

  /* Step 2: Attempt to assign displacements d[bin], starting with
     the largest bin */
  for (i = 0; i != slot_count; ++i) {
    taken[i] = 0;
  }
  for (j = 0; j != bin_count; ++j) {
    uint16_t dval;
    bin = binsizes_ordering[j];
    if (binsizes[bin] == 0) {
      d[bin] = 0;
    } else {
      for (dval = 0; dval != slot_count; ++dval) {
        int k;
        int collides = 0;
        for (k = 0; k != binsizes[bin]; ++k) {
          uint16_t slot = (((hashes[bins[BIN_LIMIT * bin + k]] >> r) & m_f) ^
                           dval);
          if (taken[slot]) {
            collides = 1;
            break;
          }
        }
        if (!collides) break;
      }
      if (dval == slot_count) {
        /* no appropriate dval found */
        return -2;
      } else {
        int k;
        /* mark slots as taken and shuffle in table elements */
        for (k = 0; k != binsizes[bin]; ++k) {
          uint16_t slot = (((hashes[bins[BIN_LIMIT * bin + k]] >> r) & m_f) ^
                           dval);
          taken[slot] = 1;
          out_permutation[bins[BIN_LIMIT * bin + k]] = slot;
        }
        /* record dval */
        d[bin] = dval;
      }
    }
  }
  return 0;
}

int PyCustomSlots_PerfectHash(lookup_table_header_t *table,
                              uint16_t entry_count,
                              uint16_t slot_count,
                              uint16_t bin_count,
                              uint64_t *hashes,
                              uint16_t *out_permutation) {
  uint16_t bin;
  uint8_t binsize;
  uint16_t i;
  uint16_t m_f = slot_count - 1;
  uint16_t m_g = bin_count - 1;
  uint16_t *bins = malloc(sizeof(uint16_t) * bin_count * BIN_LIMIT);
  uint8_t *binsizes = malloc(sizeof(uint8_t) * bin_count);
  uint16_t *binsizes_ordering = malloc(sizeof(uint16_t) * bin_count);
  uint8_t *taken = malloc(sizeof(uint8_t) * slot_count);
  uint16_t number_of_bins_by_size[BIN_LIMIT];

  table->entry_count = entry_count;
  table->bin_count = bin_count;
  table->slot_count = slot_count;
  table->m_f = m_f;
  table->m_g = m_g;
  

  /* Bin the n hashes into b bins based on the g hash. Also count the
     number of bins of each size. */
  for (bin = 0; bin != bin_count; ++bin) {
    binsizes[bin] = 0;
  }
  number_of_bins_by_size[0] = bin_count;
  for (binsize = 1; binsize != BIN_LIMIT; ++binsize) {
    number_of_bins_by_size[binsize] = 0;
  }
  for (i = 0; i != entry_count; ++i) {
    bin = hashes[i] & m_g;
    binsize = ++binsizes[bin];
    if (binsize == BIN_LIMIT) {
      printf("ERROR 1\n");
      return -1;
    }
    bins[BIN_LIMIT * bin + binsize - 1] = i;
    number_of_bins_by_size[binsize - 1]--;
    number_of_bins_by_size[binsize]++;
  }

  /* argsort the bins (p stores permutation) from largest to
     smallest, using binsort */
  _PyCustomSlots_bucket_argsort(binsizes_ordering, binsizes,
                                number_of_bins_by_size);

  /* Find perfect table -- try again for each choice of r */
  table->m_f = m_f;
  table->m_g = m_g;
  int r, retcode;
  for (r = 64; r != -1; --r) {
    table->r = r;
    retcode = _PyCustomSlots_FindDisplacements(table, hashes, binsizes, bins,
                                               binsizes_ordering,
                                               out_permutation, taken);
    if (retcode == 0) {
      break;
    }
  }

  if (table->r == 0) {
    return -1;
  }

  /*TODO does not free on error... */
  free(bins);
  free(binsizes);
  free(binsizes_ordering);
  free(taken);

  return 0;
}
