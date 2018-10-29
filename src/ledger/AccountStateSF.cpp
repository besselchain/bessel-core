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
#include <ledger/AccountStateSF.h>
#include <main/Application.h>
#include <common/misc/NetworkOPs.h>
#include <transaction/tx/TransactionMaster.h>
#include <data/nodestore/Database.h>
#include <protocol/HashPrefix.h>

namespace bessel {

AccountStateSF::AccountStateSF()
{
}

void AccountStateSF::gotNode (bool fromFilter,
                              SHAMapNodeID const& id,
                              uint256 const& nodeHash,
                              Blob& nodeData,
                              SHAMapTreeNode::TNType)
{
    //  SHAMapSync filters should be passed the SHAMap, the
    //        SHAMap should provide an accessor to get the injected Database,
    //        and this should use that Database instad of getNodeStore
    getApp().getNodeStore ().store (hotACCOUNT_NODE, std::move (nodeData), nodeHash);
}

bool AccountStateSF::haveNode (SHAMapNodeID const& id,
                               uint256 const& nodeHash,
                               Blob& nodeData)
{
    return getApp().getOPs ().getFetchPack (nodeHash, nodeData);
}

} // bessel