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

#elif defined(CPU_SPARC) && !defined(OS_OPENBSD)
	uint64_t t;
	__asm__ __volatile__("rd %%tick, %0" : "=r" (t));
	return t;
#elif defined(CPU_PPC)
	uint32_t lo = 0, hi = 0;
	__asm__ __volatile__("mftbu %0; mftb %1" : "=r" (hi), "=r" (lo));
	return ((uint64_t)lo | ((uint64_t)hi << 32));
#elif defined(CPU_IA64)
	uint64_t t;
	__asm__ __volatile__("mov %0=ar.itc" : "=r" (t));
	return t;
#elif defined(OS_NIX)
	timeval t2;
	gettimeofday(&t2, NULL);
	t = ((uint64_t)t2.tv_usec << 32) | (uint64_t)t2.tv_sec;
	return t;
#else
	need ticks for this platform
#endif
}

#define timeit(x,minvar)         \
	ticks = get_ticks();         \
 	x;                           \
	ticks = get_ticks() - ticks; \
	if (ticks < minvar)          \
		minvar = ticks;

#define maxticks 0xffffffffffffffffull

