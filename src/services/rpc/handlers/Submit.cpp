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
#include <network/resource/Fees.h>
#include <common/misc/NetworkOPs.h>
#include <transaction/paths/FindPaths.h>
#include <common/base/StringUtilities.h>
#include <common/json/json_reader.h>
#include <common/core/LoadFeeTrack.h>
#include <common/base/StringUtilities.h>
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
#include <services/server/Role.h>
#include <main/Application.h>
//#include <network/rpc/handlers/AccountMerge.cpp>

namespace bessel {

// {
//   tx_json: <object>,
//   secret: <secret>
// }


Json::Value doSubmit (RPC::Context& context)
{
    context.loadType = Resource::feeMediumBurdenRPC;

    if (!context.params.isMember (jss::tx_blob))
    {
        bool bFailHard = context.params.isMember (jss::fail_hard)
                && context.params[jss::fail_hard].asBool ();
    
       return RPC::transactionSign (
            context.params, true, bFailHard, context.netOps, context.role);
    }

    Json::Value jvResult;

    std::pair<Blob, bool> ret(strUnHex (context.params[jss::tx_blob].asString ()));

    if (!ret.second || !ret.first.size ())
        return rpcError (rpcINVALID_PARAMS);

    Serializer sTrans (ret.first);
    SerialIter sitTrans (sTrans);

    STTx::pointer stpTrans;

    try
    {
        stpTrans = std::make_shared<STTx> (std::ref (sitTrans));
    }
    catch (std::exception& e)
    {
        jvResult[jss::error]           = "invalidTransaction";
        jvResult[jss::error_exception] = e.what ();

        return jvResult;
    }

    Transaction::pointer            tpTrans;
    std::string reason;
    tpTrans = std::make_shared<Transaction> (stpTrans, Validate::YES, reason);
    if (tpTrans->getStatus() != NEW)
    {
        jvResult[jss::error]            = "invalidTransaction";
        jvResult[jss::error_exception] = "fails local checks: " + reason;

        return jvResult;
    }

    try
    {
        (void) context.netOps.processTransaction (
            tpTrans, context.role == Role::ADMIN, true,
            context.params.isMember (jss::fail_hard)
            && context.params[jss::fail_hard].asBool ());
    }
    catch (std::exception& e)
    {
        jvResult[jss::error]           = "internalSubmit";
        jvResult[jss::error_exception] = e.what ();

        return jvResult;
    }


    try
    {
        jvResult[jss::tx_json] = tpTrans->getJson (0);
        jvResult[jss::tx_blob] = strHex (
            tpTrans->getSTransaction ()->getSerializer ().peekData ());

        if (temUNCERTAIN != tpTrans->getResult ())
        {
            std::string sToken;
            std::string sHuman;

            transResultInfo (tpTrans->getResult (), sToken, sHuman);

            jvResult[jss::engine_result]           = sToken;
            jvResult[jss::engine_result_code]      = tpTrans->getResult ();
            jvResult[jss::engine_result_message]   = sHuman;
        }

        return jvResult;
    }
    catch (std::exception& e)
    {
        jvResult[jss::error]           = "internalJson";
        jvResult[jss::error_exception] = e.what ();

        return jvResult;
    }
}

} // bessel
