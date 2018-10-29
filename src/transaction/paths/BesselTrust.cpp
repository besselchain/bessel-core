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
#include <transaction/book/Types.h>
#include <transaction/paths/BesselTrust.h>
#include <protocol/STAmount.h>
#include <cstdint>

namespace bessel {

BesselTrust::pointer BesselTrust::makeItem (
    Account const& accountID, STLedgerEntry::ref ledgerEntry)
{
    //  Does this ever happen in practice?
    if (!ledgerEntry || ledgerEntry->getType () != ltTrust_STATE)
        return pointer ();

    return pointer (new BesselTrust (ledgerEntry, accountID));
}

BesselTrust::BesselTrust (
        STLedgerEntry::ref ledgerEntry,
        Account const& viewAccount)
    : mLedgerEntry (ledgerEntry)
    , mLowLimit (ledgerEntry->getFieldAmount (sfLowLimit))
    , mHighLimit (ledgerEntry->getFieldAmount (sfHighLimit))
    , mLowID (mLowLimit.getIssuer ())
    , mHighID (mHighLimit.getIssuer ())
    , mBalance (ledgerEntry->getFieldAmount (sfBalance))
{
    mFlags          = mLedgerEntry->getFieldU32 (sfFlags);
    mRelationType          = mLedgerEntry->getFieldU32 (sfRelationType);

    mLowQualityIn   = mLedgerEntry->getFieldU32 (sfLowQualityIn);
    mLowQualityOut  = mLedgerEntry->getFieldU32 (sfLowQualityOut);

    mHighQualityIn  = mLedgerEntry->getFieldU32 (sfHighQualityIn);
    mHighQualityOut = mLedgerEntry->getFieldU32 (sfHighQualityOut);

    mViewLowest = (mLowID == viewAccount);

    if (!mViewLowest)
        mBalance.negate ();
}

Json::Value BesselTrust::getJson (int)
{
    Json::Value ret (Json::objectValue);
    ret["low_id"] = to_string (mLowID);
    ret["high_id"] = to_string (mHighID);
    return ret;
}

std::vector <BesselTrust::pointer>
getBesselTrustItems (
    Account const& accountID,
    Ledger::ref ledger)
{
    std::vector <BesselTrust::pointer> items;

    ledger->visitAccountItems (accountID,
        [&items,&accountID](SLE::ref sleCur)
        {
             auto ret = BesselTrust::makeItem (accountID, sleCur);

             if (ret)
                items.push_back (ret);
        });

    return items;
}

} // bessel
