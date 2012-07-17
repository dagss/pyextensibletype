from libc.stdlib cimport malloc, free

cimport numpy as cnp
import numpy as np

cdef extern from "stdint.h":
    ctypedef unsigned int uint32_t
    ctypedef unsigned long long uint64_t
    ctypedef unsigned short uint16_t
    ctypedef unsigned char uint8_t
    ctypedef uint64_t uintptr_t

cdef extern from "perfecthash.h":

    ctypedef struct lookup_table_header_t:
        uint16_t entry_count, slot_count, bin_count, m_f, m_g
        uint8_t r
        uint8_t flags
        uint16_t d[0]

    int PyCustomSlots_PerfectHash(lookup_table_header_t *table,
                                  uint16_t entry_count,
                                  uint16_t slot_count,
                                  uint16_t bin_count,
                                  uint64_t *hashes,
                                  uint16_t *out_permutation)

    void _PyCustomSlots_bucket_argsort(uint16_t *p, uint8_t *binsizes,
                                       uint16_t *number_of_bins_by_size)

    uint64_t PyCustomSlots_roundup_2pow(uint64_t x)

def bucket_argsort(cnp.ndarray[uint16_t, mode='c'] p,
                   cnp.ndarray[uint8_t, mode='c'] binsizes,
                   cnp.ndarray[uint16_t, mode='c'] number_of_bins_by_size):
    _PyCustomSlots_bucket_argsort(&p[0], &binsizes[0],
                                  &number_of_bins_by_size[0])

def roundup_2pow(i):
    return PyCustomSlots_roundup_2pow(i)

def perfect_hash(cnp.ndarray[uint64_t] hashes,
                 int slot_count=-1, int bin_count=-1, int repeat=1):
    """Used for testing. Takes the hashes as input, and returns
       a permutation array and hash parameters:

       (p, r, m_f, m_g, d)
    """
    if slot_count == -1:
        slot_count = PyCustomSlots_roundup_2pow(len(hashes))
    if bin_count == -1:
        bin_count = slot_count
    if slot_count < len(hashes):
        raise ValueError('slot_count < len(hashes)')
    
    cdef cnp.ndarray[uint16_t] out = np.zeros(slot_count, np.uint16)
    cdef lookup_table_header_t *table = <lookup_table_header_t*>(
        malloc(sizeof(lookup_table_header_t)
               + sizeof(uint16_t) * bin_count))
    cdef int r
    for r in range(repeat):
        retcode = PyCustomSlots_PerfectHash(table, len(hashes), slot_count, bin_count,
                                            &hashes[0], &out[0])
    if retcode != 0:
        raise RuntimeError("No perfect hash found")

    cdef int i
    cdef cnp.ndarray[uint16_t] d = np.zeros(bin_count, np.uint16)
    for i in range(bin_count):
        d[i] = table.d[i]

    result = (out, table.r, table.m_f, table.m_g, d)
    free(table)
    return result

cdef extern from "md5sum.h":
    ctypedef struct MD5_CTX:
        uint32_t i[2]
        uint32_t buf[4]
        unsigned char in_ "in"[64]
        unsigned char digest[16]

    void MD5Init(MD5_CTX *mdContext)
    void MD5Update(MD5_CTX *mdContext, unsigned char *inBuf,
                   unsigned int inLen)
    void MD5Final(MD5_CTX *mdContext)
    
cdef extern from "hash.h":
    uint64_t hash_crapwow64(unsigned char *buf, uint64_t len, uint64_t seed)

cdef extern from "sph_md5.h":
    ctypedef struct sph_md5_context:
        pass
    void sph_md5_init(void *cc)
    void sph_md5(void *cc, void *data, size_t len)
    void sph_md5_close(void *cc, void *dst)

    

def crapwowbench(int repeat=1):
    cdef int r
    cdef MD5_CTX ctx
    for r in range(repeat):
        hash_crapwow64("asdf", 4, 0xf123456781234567)


def md5bench(int repeat=1):
    cdef int r
    cdef MD5_CTX ctx
    for r in range(repeat):
        MD5Init(&ctx)
        MD5Update(&ctx, "asdf", 4)
        MD5Final(&ctx)


def md5bench2(int repeat=1):
    cdef sph_md5_context ctx
    cdef char out[16]
    for r in range(repeat):
        sph_md5_init(&ctx)
        sph_md5(&ctx, "asdf", 4)
        sph_md5_close(&ctx, out)

        
