//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER) && (_MSC_VER < 1600)

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

// Other compilers

#else	// defined(_MSC_VER)

#include <stdint.h>

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------

void MurmurHash3_x86_32  ( const void * key, int len, uint32_t seed, void * out );

void MurmurHash3_x86_128 ( const void * key, int len, uint32_t seed, void * out );

void MurmurHash3_x64_128 ( const void * key, int len, uint32_t seed, void * out );

inline uint64_t murmur3_64(const long key, uint32_t seed = 42) {
    uint64_t hash[2]; // 128-bit result, but only 64 bits used here
    MurmurHash3_x64_128(&key, sizeof(key), seed, hash);
    return hash[0]; // Only take the first 64 bits
}

inline uint64_t hash_in_range(long value, uint64_t d) {
    uint64_t hash_value = murmur3_64(value);
    return hash_value % d;  // Map to range 0..d
}

//-----------------------------------------------------------------------------

#endif // _MURMURHASH3_H_
