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
#include <ledger/ConsensusTransSetSF.h>
#include <main/Application.h>
#include <transaction/tx/TransactionMaster.h>
#include <common/misc/NetworkOPs.h>
#include <common/base/Log.h>
#include <common/core/JobQueue.h>
#include <data/nodestore/Database.h>
#include <protocol/HashPrefix.h>

namespace bessel {

ConsensusTransSetSF::ConsensusTransSetSF (NodeCache& nodeCache)
    : m_nodeCache (nodeCache)
{
}

void ConsensusTransSetSF::gotNode (bool fromFilter, 
                                   const SHAMapNodeID& id, 
                                   uint256 const& nodeHash,
                                   Blob& nodeData, 
                                   SHAMapTreeNode::TNType type)
{
    if (fromFilter)
        return;

    m_nodeCache.insert (nodeHash, nodeData);

    if ((type == SHAMapTreeNode::tnTRANSACTION_NM) && (nodeData.size () > 16))
    {
        // this is a transaction, and we didn't have it
        WriteLog (lsDEBUG, TransactionAcquire) << "Node on our acquiring TX set is TXN we may not have";

        try
        {
            Serializer s (nodeData.begin () + 4, nodeData.end ()); // skip prefix
            SerialIter sit (s);
            STTx::pointer stx = std::make_shared<STTx> (std::ref (sit));

            assert (stx->getTransactionID () == nodeHash);

            getApp().getJobQueue ().addJob (
                                            jtTRANSACTION, "TXS->TXN",
                                            std::bind (&NetworkOPs::submitTransaction, 
                                                       &getApp().getOPs (),
                                                       std::placeholders::_1, 
                                                       stx,
                                                       NetworkOPs::stCallback ()));
        }
        catch (...)
        {
            WriteLog (lsWARNING, TransactionAcquire) << "Fetched invalid transaction in proposed set";
        }
    }
}

bool ConsensusTransSetSF::haveNode (const SHAMapNodeID& id, uint256 const& nodeHash, Blob& nodeData)
{
    if (m_nodeCache.retrieve (nodeHash, nodeData))
        return true;

    //  TODO Use a dependency injection here
    Transaction::pointer txn = getApp ().getMasterTransaction ().fetch (nodeHash, false);

    if (txn)
    {
        // this is a transaction, and we have it
        WriteLog (lsTRACE, TransactionAcquire) << "Node in our acquiring TX set is TXN we have";

        Serializer s;
        s.add32 (HashPrefix::transactionID);
        txn->getSTransaction ()->add (s, true);
        assert (s.getSHA512Half () == nodeHash);
        nodeData = s.peekData ();

        return true;
    }

    return false;
}

} // bessel
