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

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTOR(S) ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTOR(S) BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: sha2.h,v 1.1 2001/11/08 00:02:01 adg Exp adg $
 */

#ifndef BEAST_CRYPTO_SHA2_SHA2_H_INCLUDED
#define BEAST_CRYPTO_SHA2_SHA2_H_INCLUDED

//#ifdef __cplusplus
//extern "C" {
//#endif

/*
 * Import u_intXX_t size_t type definitions from system headers.  You
 * may need to change this, or define these things yourself in this
 * file.
 */
#include <sys/types.h>


/*** SHA-256/384/512 Various Length Definitions ***********************/
#define SHA256_DIGEST_STRING_LENGTH	(Sha256::digestLength * 2 + 1)
#define SHA384_BLOCK_LENGTH   128
#define SHA384_DIGEST_LENGTH   48
#define SHA384_DIGEST_STRING_LENGTH	(SHA384_DIGEST_LENGTH * 2 + 1)
#define SHA512_BLOCK_LENGTH	 128
#define SHA512_DIGEST_LENGTH  64
#define SHA512_DIGEST_STRING_LENGTH	(SHA512_DIGEST_LENGTH * 2 + 1)

/*** SHA-256/384/512 Context Structures *******************************/
typedef struct _SHA512_CTX {
	std::uint64_t	state[8];
	std::uint64_t	bitcount[2];
	std::uint8_t	buffer[SHA512_BLOCK_LENGTH];
} SHA512_CTX;

typedef SHA512_CTX SHA384_CTX;


/*** SHA-256/384/512 Function Prototypes ******************************/
#ifndef NOPROTO

void SHA256_Init(Sha256::detail::Context *);
void SHA256_Update(Sha256::detail::Context*, const std::uint8_t*, size_t);
void SHA256_Final(std::uint8_t[Sha256::digestLength], Sha256::detail::Context*);
char* SHA256_End(Sha256::detail::Context*, char[SHA256_DIGEST_STRING_LENGTH]);
char* SHA256_Data(const std::uint8_t*, size_t, char[SHA256_DIGEST_STRING_LENGTH]);

void SHA384_Init(SHA384_CTX*);
void SHA384_Update(SHA384_CTX*, const std::uint8_t*, size_t);
void SHA384_Final(std::uint8_t[SHA384_DIGEST_LENGTH], SHA384_CTX*);
char* SHA384_End(SHA384_CTX*, char[SHA384_DIGEST_STRING_LENGTH]);
char* SHA384_Data(const std::uint8_t*, size_t, char[SHA384_DIGEST_STRING_LENGTH]);

void SHA512_Init(SHA512_CTX*);
void SHA512_Update(SHA512_CTX*, const std::uint8_t*, size_t);
void SHA512_Final(std::uint8_t[SHA512_DIGEST_LENGTH], SHA512_CTX*);
char* SHA512_End(SHA512_CTX*, char[SHA512_DIGEST_STRING_LENGTH]);
char* SHA512_Data(const std::uint8_t*, size_t, char[SHA512_DIGEST_STRING_LENGTH]);

#else /* NOPROTO */

void SHA256_Init();
void SHA256_Update();
void SHA256_Final();
char* SHA256_End();
char* SHA256_Data();

void SHA384_Init();
void SHA384_Update();
void SHA384_Final();
char* SHA384_End();
char* SHA384_Data();

void SHA512_Init();
void SHA512_Update();
void SHA512_Final();
char* SHA512_End();
char* SHA512_Data();

#endif /* NOPROTO */

//#ifdef	__cplusplus
//}
//#endif /* __cplusplus */

#endif /* __SHA2_H__ */

