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

#ifndef BESSEL_APP_LEDGER_CONSENSUSTRANSSETSF_H_INCLUDED
#define BESSEL_APP_LEDGER_CONSENSUSTRANSSETSF_H_INCLUDED

#include <common/shamap/SHAMapSyncFilter.h>
#include <common/base/TaggedCache.h>

namespace bessel {

// Sync filters allow low-level SHAMapSync code to interact correctly with
// higher-level structures such as caches and transaction stores

// This class is needed on both add and check functions
// sync filter for transaction sets during consensus building
class ConsensusTransSetSF : public SHAMapSyncFilter
{
public:
    typedef TaggedCache <uint256, Blob> NodeCache;

    //  TODO Use a dependency injection to get the temp node cache
    ConsensusTransSetSF (NodeCache& nodeCache);

    // Note that the nodeData is overwritten by this call
    void gotNode (bool fromFilter,
                  SHAMapNodeID const& id,
                  uint256 const& nodeHash,
                  Blob& nodeData,
                  SHAMapTreeNode::TNType) override;

    bool haveNode (SHAMapNodeID const& id,
                   uint256 const& nodeHash,
                   Blob& nodeData) override;

private:
    NodeCache& m_nodeCache;
};

} // bessel

#endif
