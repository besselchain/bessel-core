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

#include <BeastConfig.h>
#include <common/misc/NetworkOPs.h>
#include <common/base/StringUtilities.h>
#include <common/json/json_reader.h>
#include <common/core/LoadFeeTrack.h>
#include <protocol/TxFlags.h>
#include <protocol/Indexes.h>
#include <protocol/JsonFields.h>
#include <protocol/ErrorCodes.h>
#include <ledger/LedgerMaster.h>
#include <services/rpc/impl/KeypairForSignature.h>
#include <services/rpc/impl/TransactionSign.h>
#include <services/rpc/impl/Tuning.h>
#include <services/rpc/impl/LegacyPathFind.h>
#include <services/net/RPCErr.h>
#include <transaction/paths/FindPaths.h>
#include <main/Application.h>

namespace bessel {
namespace RPC {

KeyPair keypairForSignature (Json::Value const& params, Json::Value& error)
{
    bool const has_key_type   = params.isMember (jss::key_type);
    bool const has_passphrase = params.isMember (jss::passphrase);
    bool const has_secret     = params.isMember (jss::secret);
    bool const has_seed       = params.isMember (jss::seed);
    bool const has_seed_hex   = params.isMember (jss::seed_hex);

    int const n_secrets = has_passphrase + has_secret + has_seed + has_seed_hex;

    if (n_secrets == 0)
    {
        error = RPC::missing_field_error (jss::secret);
        return KeyPair();
    }

    if (n_secrets > 1)
    {
        // `passphrase`, `secret`, `seed`, and `seed_hex` are mutually exclusive.
        error = rpcError (rpcBAD_SECRET);
        return KeyPair();
    }

    if (has_key_type  &&  has_secret)
    {
        // `secret` is deprecated.
        error = rpcError (rpcBAD_SECRET);
        return KeyPair();
    }

    KeyType type = KeyType::secp256k1;

    BesselAddress seed;

    if (has_key_type)
    {
        // `key_type` must be valid if present.

        type = keyTypeFromString (params[jss::key_type].asString());

        if (type == KeyType::invalid)
        {
            error = rpcError (rpcBAD_SEED);
            return KeyPair();
        }

        seed = getSeedFromRPC (params);
    }
    else
    if (! seed.setSeedGeneric (params[jss::secret].asString ()))
    {
        error = RPC::make_error (rpcBAD_SEED,
            RPC::invalid_field_message (jss::secret));
    }

    return generateKeysFromSeed (type, seed);
}

} // RPC
} // bessel
