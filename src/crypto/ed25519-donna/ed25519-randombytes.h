//------------------------------------------------------------------------------
//*
    This file is part of Bessel Chain Project: https://github.com/Besselfoundation/bessel-core
    Copyright (c) 2018 BESSEL.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

	b = (x + a) ^ mm[(y >> 10) & 0xff]; \
	U32TO8_LE(out + (i + offset) * 4, b);

static void
isaacp_mix(isaacp_state *st) {
	uint32_t i, x, y;
	uint32_t a = st->a, b = st->b, c = st->c;
	uint32_t *mm = st->state;
	unsigned char *out = st->buffer;

	c = c + 1;
	b = b + c;

	for (i = 0; i < 256; i += 4) {
		isaacp_step(0, ROTL32(a,13))
		isaacp_step(1, ROTR32(a, 6))
		isaacp_step(2, ROTL32(a, 2))
		isaacp_step(3, ROTR32(a,16))
	}

	st->a = a;
	st->b = b;
	st->c = c;
	st->left = 1024;
}

static void
isaacp_random(isaacp_state *st, void *p, size_t len) {
	size_t use;
	unsigned char *c = (unsigned char *)p;
	while (len) {
		use = (len > st->left) ? st->left : len;
		memcpy(c, st->buffer + (sizeof(st->buffer) - st->left), use);

		st->left -= use;
		c += use;
		len -= use;

		if (!st->left)
			isaacp_mix(st);
	}
}

void
ED25519_FN(ed25519_randombytes_unsafe) (void *p, size_t len) {
	static int initialized = 0;
	static isaacp_state rng;

	if (!initialized) {
		memset(&rng, 0, sizeof(rng));
		isaacp_mix(&rng);
		isaacp_mix(&rng);
		initialized = 1;
	}

	isaacp_random(&rng, p, len);
}
#elif defined(ED25519_CUSTOMRNG)

#include "ed25519-randombytes-custom.h"

#else

#include <openssl/rand.h>

void
ED25519_FN(ed25519_randombytes_unsafe) (void *p, size_t len) {

  RAND_bytes(p, (int) len);

}
#endif
