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
#include <transaction/paths/AccountCurrencies.h>
#include <transaction/paths/FindPaths.h>
#include <transaction/paths/BesselCalc.h>
#include <transaction/paths/PathRequest.h>
#include <transaction/paths/PathRequests.h>
#include <main/Application.h>
#include <common/misc/NetworkOPs.h>
#include <common/base/Log.h>
#include <common/core/Config.h>
#include <common/core/LoadFeeTrack.h>
#include <services/net/RPCErr.h>
#include <protocol/ErrorCodes.h>
#include <protocol/UintTypes.h>
#include <boost/log/trivial.hpp>
#include <tuple>

namespace bessel {

PathRequest::PathRequest (
    const std::shared_ptr<InfoSub>& subscriber,
    int id,
    PathRequests& owner,
    beast::Journal journal)
        : m_journal (journal)
        , mOwner (owner)
        , wpSubscriber (subscriber)
        , jvStatus (Json::objectValue)
        , bValid (false)
        , mLastIndex (0)
        , mInProgress (false)
        , iLastLevel (0)
        , bLastSuccess (false)
        , iIdentifier (id)
{
    if (m_journal.debug)
    {
        m_journal.debug << iIdentifier << " created";
    }

    ptCreated = boost::posix_time::microsec_clock::universal_time ();
}

static std::string const get_milli_diff (
    boost::posix_time::ptime const& after,
    boost::posix_time::ptime
    const& before)
{
    return boost::lexical_cast<std::string> (static_cast<unsigned> ((after - before).total_milliseconds ()));
}

static std::string const get_milli_diff (boost::posix_time::ptime const& before)
{
    return get_milli_diff(boost::posix_time::microsec_clock::universal_time (), before);
}

PathRequest::~PathRequest()
{
    std::string fast, full;
    if (!ptQuickReply.is_not_a_date_time ())
    {
        fast = " fast:";
        fast += get_milli_diff (ptQuickReply, ptCreated);
        fast += "ms";
    }
    if (!ptFullReply.is_not_a_date_time ())
    {
        full = " full:";
        full += get_milli_diff (ptFullReply, ptCreated);
        full += "ms";
    }
    if (m_journal.info)
    {
        m_journal.info << iIdentifier << " complete:" << fast << full << " total:" << get_milli_diff (ptCreated) << "ms";
    }
}

bool PathRequest::isValid ()
{
    ScopedLockType sl (mLock);

    return bValid;
}

bool PathRequest::isNew ()
{
    ScopedLockType sl (mIndexLock);

    // does this path request still need its first full path
    return mLastIndex == 0;
}

bool PathRequest::needsUpdate (bool newOnly, LedgerIndex index)
{
    ScopedLockType sl (mIndexLock);

    if (mInProgress)
    {
        // Another thread is handling this
        return false;
    }

    if (newOnly && (mLastIndex != 0))
    {
        // Only handling new requests, this isn't new
        return false;
    }

    if (mLastIndex >= index)
    {
        return false;
    }

    mInProgress = true;

    return true;
}

void PathRequest::updateComplete ()
{
    ScopedLockType sl (mIndexLock);

    assert (mInProgress);

    mInProgress = false;
}

bool PathRequest::isValid (BesselLineCache::ref crCache)
{
    ScopedLockType sl (mLock);
    bValid = raSrcAccount.isSet () && 
             raDstAccount.isSet () &&
             saDstAmount > zero;

    Ledger::pointer lrLedger = crCache->getLedger ();

    if (bValid)
    {
        auto asSrc = getApp().getOPs ().getAccountState (crCache->getLedger (), raSrcAccount);
        if (!asSrc)
        {
            // no source account
            bValid = false;
            jvStatus = rpcError (rpcSRC_ACT_NOT_FOUND);
        }
        else
        {
            auto asDst = getApp().getOPs ().getAccountState (lrLedger, raDstAccount);

            Json::Value& jvDestCur = (jvStatus[jss::destination_currencies] = Json::arrayValue);

            if (!asDst)
            {
                // no destination account
                jvDestCur.append (Json::Value ("SWT"));

                if (!saDstAmount.isNative ())
                {
                    // only SWT can be send to a non-existent account
                    bValid = false;
                    jvStatus = rpcError (rpcACT_NOT_FOUND);
                }
                else if (saDstAmount < STAmount (lrLedger->getReserve (0)))
                {
                    // payment must meet reserve
                    bValid = false;
                    jvStatus = rpcError (rpcDST_AMT_MALFORMED);
                }
            }
            else
            {
                bool const disallowSWT (asDst->peekSLE ().getFlags () & lsfDisallowSWT);

                auto usDestCurrID = accountDestCurrencies (raDstAccount, crCache, !disallowSWT);

                for (auto const& currency : usDestCurrID)
                {
                    jvDestCur.append (to_string (currency));
                }

                jvStatus["destination_tag"] = (asDst->peekSLE ().getFlags () & lsfRequireDestTag) != 0;
            }
        }
    }

    if (bValid)
    {
        jvStatus[jss::ledger_hash] = to_string (lrLedger->getHash ());
        jvStatus[jss::ledger_index] = lrLedger->getLedgerSeq ();
    }
    return bValid;
}

Json::Value PathRequest::doCreate (
    Ledger::ref lrLedger,
    BesselLineCache::ref& cache,
    Json::Value const& value,
    bool& valid)
{

    Json::Value status;

    if (parseJson (value, true) != PFR_PJ_INVALID)
    {
        bValid = isValid (cache);

        if (bValid)
        {
            status = doUpdate (cache, true);
        }
        else
        {
            status = jvStatus;
        }
    }
    else
    {
        bValid = false;
        status = jvStatus;
    }

    if (m_journal.debug)
    {
        if (bValid)
        {
            m_journal.debug << iIdentifier << " valid: " << raSrcAccount.humanAccountID ();
            m_journal.debug << iIdentifier << " Deliver: " << saDstAmount.getFullText ();
        }
        else
        {
            m_journal.debug << iIdentifier << " invalid";
        }
    }

    valid = bValid;
    return status;
}

int PathRequest::parseJson (Json::Value const& jvParams, bool complete)
{
    int ret = PFR_PJ_NOCHANGE;

    if (jvParams.isMember (jss::source_account))
    {
        if (!raSrcAccount.setAccountID (jvParams[jss::source_account].asString ()))
        {
            jvStatus = rpcError (rpcSRC_ACT_MALFORMED);
            return PFR_PJ_INVALID;
        }
    }
    else if (complete)
    {
        jvStatus = rpcError (rpcSRC_ACT_MISSING);
        return PFR_PJ_INVALID;
    }

    if (jvParams.isMember (jss::destination_account))
    {
        if (!raDstAccount.setAccountID (jvParams[jss::destination_account].asString ()))
        {
            jvStatus = rpcError (rpcDST_ACT_MALFORMED);
            return PFR_PJ_INVALID;
        }
    }
    else if (complete)
    {
        jvStatus = rpcError (rpcDST_ACT_MISSING);
        return PFR_PJ_INVALID;
    }

    if (jvParams.isMember (jss::destination_amount))
    {
        if (! amountFromJsonNoThrow (
                saDstAmount, jvParams[jss::destination_amount]) ||
            (saDstAmount.getCurrency ().isZero () && saDstAmount.getIssuer ().isNonZero ()) ||
            (saDstAmount.getCurrency () == badCurrency ()) ||
            saDstAmount <= zero)
        {
            jvStatus = rpcError (rpcDST_AMT_MALFORMED);
            return PFR_PJ_INVALID;
        }
    }
    else if (complete)
    {
        jvStatus = rpcError (rpcDST_ACT_MISSING);
        return PFR_PJ_INVALID;
    }

    if (jvParams.isMember (jss::source_currencies))
    {
        Json::Value const& jvSrcCur = jvParams[jss::source_currencies];

        if (!jvSrcCur.isArray ())
        {
            jvStatus = rpcError (rpcSRC_CUR_MALFORMED);
            return PFR_PJ_INVALID;
        }

        sciSourceCurrencies.clear ();

        for (unsigned i = 0; i < jvSrcCur.size (); ++i)
        {
            Json::Value const& jvCur = jvSrcCur[i];
            Currency uCur;
            Account uIss;

            if (!jvCur.isObject() || 
                !jvCur.isMember (jss::currency) ||
                !to_currency (uCur, jvCur[jss::currency].asString ()))
            {
                jvStatus = rpcError (rpcSRC_CUR_MALFORMED);
                return PFR_PJ_INVALID;
            }

            if (jvCur.isMember (jss::issuer) &&
                !to_issuer (uIss, jvCur[jss::issuer].asString ()))
            {
                jvStatus = rpcError (rpcSRC_ISR_MALFORMED);
            }

            if (uCur.isZero () && uIss.isNonZero ())
            {
                jvStatus = rpcError (rpcSRC_CUR_MALFORMED);
                return PFR_PJ_INVALID;
            }

            if (uCur.isNonZero() && uIss.isZero())
            {
                uIss = raSrcAccount.getAccountID();
            }

            sciSourceCurrencies.insert ({uCur, uIss});
        }
    }

    if (jvParams.isMember ("id"))
    {
        jvId = jvParams["id"];
    }

    return ret;
}
Json::Value PathRequest::doClose (Json::Value const&)
{
    m_journal.debug << iIdentifier << " closed";

    ScopedLockType sl (mLock);

    return jvStatus;
}

Json::Value PathRequest::doStatus (Json::Value const&)
{
    ScopedLockType sl (mLock);
    return jvStatus;
}

void PathRequest::resetLevel (int l)
{
    if (iLastLevel > l)
    {
        iLastLevel = l;
    }
}

Json::Value PathRequest::doUpdate (BesselLineCache::ref cache, bool fast)
{
    m_journal.debug << iIdentifier << " update " << (fast ? "fast" : "normal");

    ScopedLockType sl (mLock);

    if (!isValid (cache))
    {
        return jvStatus;
    }

    jvStatus = Json::objectValue;

    auto sourceCurrencies = sciSourceCurrencies;

    if (sourceCurrencies.empty ())
    {
        auto usCurrencies = accountSourceCurrencies (raSrcAccount, cache, true);
        bool sameAccount = raSrcAccount == raDstAccount;
        for (auto const& c: usCurrencies)
        {
            if (!sameAccount || (c != saDstAmount.getCurrency ()))
            {
                if (c.isZero ())
                {
                    sourceCurrencies.insert ({c, xrpAccount()});
                }
                else
                {
                    sourceCurrencies.insert ({c, raSrcAccount.getAccountID ()});
                }
            }
        }
    }

    jvStatus[jss::source_account]      = raSrcAccount.humanAccountID ();
    jvStatus[jss::destination_account] = raDstAccount.humanAccountID ();
    jvStatus[jss::destination_amount]  = saDstAmount.getJson (0);

    if (!jvId.isNull ())
    {
        jvStatus["id"] = jvId;
    }

    Json::Value jvArray = Json::arrayValue;

    int iLevel = iLastLevel;
    bool loaded = getApp ().getFeeTrack ().isLoadedLocal ();

    if (iLevel == 0)
    {
        // first pass
        if (loaded || fast)
        {
            iLevel = getConfig ().PATH_SEARCH_FAST;
        }
        else
        {
            iLevel = getConfig ().PATH_SEARCH;
        }
    }
    else if ((iLevel == getConfig ().PATH_SEARCH_FAST) && !fast)
    {
        // leaving fast pathfinding
        iLevel = getConfig().PATH_SEARCH;
        if (loaded && (iLevel > getConfig().PATH_SEARCH_FAST))
        {
            --iLevel;
        }
    }
    else if (bLastSuccess)
    {
        // decrement, if possible
        if (iLevel > getConfig().PATH_SEARCH ||
            (loaded && (iLevel > getConfig().PATH_SEARCH_FAST)))
        {
            --iLevel;
        }
    }
    else
    {
        // adjust as needed
        if (!loaded && (iLevel < getConfig().PATH_SEARCH_MAX))
        {
            ++iLevel;
        }

        if (loaded && (iLevel > getConfig().PATH_SEARCH_FAST))
        {
            --iLevel;
        }
    }

    m_journal.debug << iIdentifier << " processing at level " << iLevel;

    bool found = false;

    FindPaths fp (
        cache,
        raSrcAccount.getAccountID (),
        raDstAccount.getAccountID (),
        saDstAmount,
        iLevel,
        4);  // iMaxPaths

    for (auto const& currIssuer: sourceCurrencies)
    {
        {
            STAmount test (currIssuer, 1);

            if (m_journal.debug)
            {
                m_journal.debug << iIdentifier << " Trying to find paths: " << test.getFullText ();
            }
        }

        STPathSet& spsPaths = mContext[currIssuer];
        STPath fullLiquidityPath;
        auto valid = fp.findPathsForIssue (
            currIssuer,
            spsPaths,
            fullLiquidityPath);

        CondLog (!valid, lsDEBUG, PathRequest) << iIdentifier << " PF request not valid";

        if (valid)
        {
            LedgerEntrySet lesSandbox (cache->getLedger (), tapNONE);
            auto& sourceAccount = !isSWT (currIssuer.account) ? currIssuer.account : isSWT (currIssuer.currency) ? xrpAccount() : raSrcAccount.getAccountID ();
            STAmount saMaxAmount ({currIssuer.currency, sourceAccount}, 1);

            saMaxAmount.negate ();

            m_journal.debug << iIdentifier << " Paths found, calling besselCalc";

            auto rc = path::BesselCalc::besselCalculate (
                lesSandbox,
                saMaxAmount,
                saDstAmount,
                raDstAccount.getAccountID (),
                raSrcAccount.getAccountID (),
                spsPaths);

            if (!fullLiquidityPath.empty () && (rc.result () == terNO_LINE || rc.result () == tecPATH_PARTIAL))
            {
                m_journal.debug << iIdentifier << " Trying with an extra path element";

                spsPaths.push_back (fullLiquidityPath);
                lesSandbox.clear();
                rc = path::BesselCalc::besselCalculate (
                    lesSandbox,
                    saMaxAmount,
                    saDstAmount,
                    raDstAccount.getAccountID (),
                    raSrcAccount.getAccountID (),
                    spsPaths);

                if (rc.result () != tesSUCCESS)
                {
                    m_journal.warning << iIdentifier << " Failed with covering path " << transHuman (rc.result ());
                }
                else
                {
                    m_journal.debug << iIdentifier << " Extra path element gives " << transHuman (rc.result ());
                }
            }

            if (rc.result () == tesSUCCESS)
            {
                Json::Value jvEntry (Json::objectValue);
                rc.actualAmountIn.setIssuer (sourceAccount);

                jvEntry[jss::source_amount] = rc.actualAmountIn.getJson (0);
                jvEntry[jss::paths_computed] = spsPaths.getJson (0);
                found  = true;
                jvArray.append (jvEntry);
            }
            else
            {
                m_journal.debug << iIdentifier << " besselCalc returns " << transHuman (rc.result ());
            }
        }
        else
        {
            m_journal.debug << iIdentifier << " No paths found";
        }
    }

    iLastLevel = iLevel;
    bLastSuccess = found;

    if (fast && ptQuickReply.is_not_a_date_time ())
    {
        ptQuickReply = boost::posix_time::microsec_clock::universal_time ();
        mOwner.reportFast ((ptQuickReply-ptCreated).total_milliseconds ());
    }
    else if (!fast && ptFullReply.is_not_a_date_time ())
    {
        ptFullReply = boost::posix_time::microsec_clock::universal_time ();
        mOwner.reportFull ((ptFullReply-ptCreated).total_milliseconds ());
    }

    jvStatus[jss::alternatives] = jvArray;
    return jvStatus;
}

InfoSub::pointer PathRequest::getSubscriber ()
{
    return wpSubscriber.lock ();
}

} // bessel