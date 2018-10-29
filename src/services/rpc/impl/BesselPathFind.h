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

#ifndef BESSEL_RPC_BESSELPATHFIND_H_INCLUDED
#define BESSEL_RPC_BESSELPATHFIND_H_INCLUDED

#include <transaction/paths/BesselLineCache.h>
#include <ledger/Ledger.h>

namespace bessel {

class BesselAddress;

Json::Value
buildSrcCurrencies(BesselAddress const& raSrc, BesselLineCache::pointer const& cache);

std::pair<bool, Json::Value>
besselPathFind(BesselLineCache::pointer const& cache, BesselAddress const& raSrc, BesselAddress const& raDst,
    STAmount const& saDstAmount, Ledger::pointer const& lpLedger, Json::Value const& jvSrcCurrencies, boost::optional<Json::Value> const& contextPaths, int const& level);

}

#endif