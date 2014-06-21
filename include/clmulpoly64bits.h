#ifndef CLMULPOLY64BITS_H_
#define CLMULPOLY64BITS_H_

#include "clmul.h"

// simple 64-bit polynomial hashing, uses only one key
// not expected to be fast!
uint64_t hashGaloisFieldPoly64(const void* rs, const uint64_t * string,
		const size_t length) {
	const uint64_t * randomsource = (const uint64_t *) rs;
	assert(*randomsource != 0); //otherwise silly
	const uint64_t * const endstring = string + length;
	__m128i key = _mm_set_epi64x(0, *(randomsource));
	__m128i acc = _mm_set_epi64x(0, *string);
	++string;
	for (; string < endstring; ++string) {
		const __m128i temp = _mm_set_epi64x(0, *string);
		const __m128i multi = _mm_clmulepi64_si128(acc, key, 0x00);
		acc = barrettWithoutPrecomputation64_si128(multi);
		acc = _mm_xor_si128(acc, temp);
	}
	return _mm_cvtsi128_si64(acc);
}

uint64_t precomphashGaloisFieldPoly64(const void* rs, const uint64_t * string,
		const size_t length) {
	const uint64_t * randomsource = (const uint64_t *) rs;
	assert(*randomsource != 0); //otherwise silly
	const uint64_t * const endstring = string + length;
	__m128i key = _mm_set_epi64x(0, *(randomsource));
	__m128i acc = _mm_set_epi64x(0, *string);
	++string;
	for (; string < endstring; ++string) {
		const __m128i temp = _mm_set_epi64x(0, *string);
		const __m128i multi = _mm_clmulepi64_si128(acc, key, 0x00);
		acc = precompReduction64_si128(multi);
		acc = _mm_xor_si128(acc, temp);
	}
	return _mm_cvtsi128_si64(acc);
}

uint64_t fasthashGaloisFieldPoly64_2_noprecomp(const void* rs,
		const uint64_t * string, const size_t length) {
	const uint64_t * randomsource = (const uint64_t *) rs;
	assert(*randomsource != 0); //otherwise silly
	const uint64_t * const endstring = string + length;
	__m128i tkey1 = _mm_set_epi64x(0, *(randomsource));
	__m128i acc = _mm_set_epi64x(0, *string);
	++string;
	if (string + 1 < endstring) {
		// we start by precomputing the powers of the key
		__m128i tkey2 = barrettWithoutPrecomputation64_si128(
				_mm_clmulepi64_si128(tkey1, tkey1, 0x00));
		__m128i key = _mm_xor_si128(_mm_and_si128(tkey1, _mm_set_epi64x(0, -1)),
				_mm_slli_si128(tkey2, 8));
		for (; string + 1 < endstring; string += 2) {
			__m128i temp = _mm_lddqu_si128((__m128i *) string);
			acc = _mm_clmulepi64_si128(acc, key, 0x10);
			const __m128i clprod1 = _mm_clmulepi64_si128(temp, key, 0x00);
			acc = _mm_xor_si128(clprod1, acc);
			acc = _mm_xor_si128(acc, _mm_srli_si128(temp, 8));
			acc = barrettWithoutPrecomputation64_si128(acc);
		}
	}
	if (string < endstring) {
		const __m128i temp = _mm_set_epi64x(0, *string);
		const __m128i multi = _mm_clmulepi64_si128(acc, tkey1, 0x00);
		acc = barrettWithoutPrecomputation64_si128(multi);
		acc = _mm_xor_si128(acc, temp);
	}
	return _mm_cvtsi128_si64(acc);
}


uint64_t fasthashGaloisFieldPoly64_2(const void* rs, const uint64_t * string,
		const size_t length) {
	const uint64_t * randomsource = (const uint64_t *) rs;
	assert(*randomsource != 0); //otherwise silly
	const uint64_t * const endstring = string + length;
	__m128i tkey1 = _mm_set_epi64x(0, *(randomsource));
	__m128i acc = _mm_set_epi64x(0, *string);
	++string;
	if (string + 1 < endstring) {
		// we start by precomputing the powers of the key
		__m128i tkey2 = precompReduction64_si128(
				_mm_clmulepi64_si128(tkey1, tkey1, 0x00));
		__m128i key = _mm_xor_si128(_mm_and_si128(tkey1, _mm_set_epi64x(0, -1)),
				_mm_slli_si128(tkey2, 8));
		for (; string + 1 < endstring; string += 2) {
			__m128i temp = _mm_lddqu_si128((__m128i *) string);
			acc = _mm_clmulepi64_si128(acc, key, 0x10);
			const __m128i clprod1 = _mm_clmulepi64_si128(temp, key, 0x00);
			acc = _mm_xor_si128(clprod1, acc);
			acc = _mm_xor_si128(acc, _mm_srli_si128(temp, 8));
			acc = precompReduction64_si128(acc);
		}
	}
	if (string < endstring) {
		const __m128i temp = _mm_set_epi64x(0, *string);
		const __m128i multi = _mm_clmulepi64_si128(acc, tkey1, 0x00);
		acc = precompReduction64_si128(multi);
		acc = _mm_xor_si128(acc, temp);
	}
	return _mm_cvtsi128_si64(acc);
}

// fast 64-bit polynomial hashing, uses only one key
// expected to be fast!
//TODO: can use more keys for increased universality
uint64_t fasthashGaloisFieldPoly64_4(const void* rs, const uint64_t * string,
		const size_t length) {
	const uint64_t * randomsource = (const uint64_t *) rs;
	assert(*randomsource != 0); //otherwise silly
	const uint64_t * const endstring = string + length;
	__m128i tkey1 = _mm_set_epi64x(0, *(randomsource));
	__m128i acc = _mm_set_epi64x(0, *string);
	++string;
	if (string + 1 < endstring) {
		// we start by precomputing the powers of the key
		__m128i tkey2 = precompReduction64_si128(
				_mm_clmulepi64_si128(tkey1, tkey1, 0x00));
		// powers of the keys are packed into two registers
		__m128i key = _mm_xor_si128(_mm_and_si128(tkey1, _mm_set_epi64x(0, -1)),
				_mm_slli_si128(tkey2, 8));
		if  (string + 3 < endstring) {
			__m128i tkey3 = precompReduction64_si128(
					_mm_clmulepi64_si128(tkey2, tkey2, 0x00));
			__m128i tkey4 = precompReduction64_si128(
					_mm_clmulepi64_si128(tkey3, tkey3, 0x00));
			__m128i key2 = _mm_xor_si128(
					_mm_and_si128(tkey3, _mm_set_epi64x(0, -1)),
					_mm_slli_si128(tkey4, 8));
			for (; string + 3 < endstring; string += 4) {
				__m128i temp = _mm_lddqu_si128((__m128i *) string); //a1 a2
				__m128i temp2 = _mm_lddqu_si128((__m128i *) (string + 2)); //a3 a4
				const __m128i x1 = _mm_srli_si128(temp2, 8); //a4
				const __m128i clprod1 = _mm_clmulepi64_si128(temp, key2, 0x01); //a1*k^3
				const __m128i clprod2 = _mm_clmulepi64_si128(temp, key, 0x10); //a2*k^2
				const __m128i clprod3 = _mm_clmulepi64_si128(temp2, key, 0x00); //a3*k
				acc = _mm_clmulepi64_si128(acc, key2, 0x10); //k^4
				acc = precompReduction64_si128(
						_mm_xor_si128(acc,
								_mm_xor_si128(_mm_xor_si128(x1, clprod1),
										_mm_xor_si128(clprod2, clprod3))));
			}
		}
		for (; string + 1 < endstring; string += 2) {
			__m128i temp = _mm_lddqu_si128((__m128i *) string);
			acc = _mm_clmulepi64_si128(acc, key, 0x10);
			const __m128i clprod1 = _mm_clmulepi64_si128(temp, key, 0x00);
			acc = _mm_xor_si128(clprod1, acc);
			acc = _mm_xor_si128(acc, _mm_srli_si128(temp, 8));
			acc = precompReduction64_si128(acc);
		}
	}
	if (string < endstring) {
		const __m128i temp = _mm_set_epi64x(0, *string);
		const __m128i multi = _mm_clmulepi64_si128(acc, tkey1, 0x00);
		acc = precompReduction64_si128(multi);
		acc = _mm_xor_si128(acc, temp);
	}
	return _mm_cvtsi128_si64(acc);
}

#endif /* CLMULPOLY64BITS_H_ */