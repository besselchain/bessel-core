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
#include <transaction/book/Quality.h>
#include <common/base/Log.h>
#include <common/base/StringUtilities.h>
#include <common/json/to_string.h>
#include <ledger/LedgerEntrySet.h>
#include <ledger/DeferredCredits.h>
#include <protocol/JsonFields.h>
#include <protocol/Indexes.h>

namespace bessel {

// #define META_DEBUG

//  TODO Replace this macro with a documented language constant
//        NOTE Is this part of the protocol?
//
#define DIR_NODE_MAX        32

void LedgerEntrySet::init (Ledger::ref ledger, 
                           uint256 const& transactionID,
                           std::uint32_t ledgerID, 
                           TransactionEngineParams params)
{
    mEntries.clear ();
    if (mDeferredCredits)
        mDeferredCredits->clear ();

    mLedger = ledger;
    mSet.init (transactionID, ledgerID);
    mParams = params;
    mSeq    = 0;
}

void LedgerEntrySet::clear ()
{
    mEntries.clear ();
    mSet.clear ();

    if (mDeferredCredits)
        mDeferredCredits->clear ();
}

LedgerEntrySet LedgerEntrySet::duplicate () const
{
    return LedgerEntrySet (mLedger, mEntries, mSet, mSeq + 1, mDeferredCredits);
}

void LedgerEntrySet::swapWith (LedgerEntrySet& e)
{
    using std::swap;
    swap (mLedger, e.mLedger);
    mEntries.swap (e.mEntries);
    mSet.swap (e.mSet);
    swap (mParams, e.mParams);
    swap (mSeq, e.mSeq);
    swap (mDeferredCredits, e.mDeferredCredits);
}

// Find an entry in the set.  If it has the wrong sequence number, copy it and update the sequence number.
// This is basically: copy-on-read.
SLE::pointer LedgerEntrySet::getEntry (uint256 const& index, LedgerEntryAction& action)
{
    auto it = mEntries.find (index);

    if (it == mEntries.end ())
    {
        action = taaNONE;
        return SLE::pointer ();
    }

    if (it->second.mSeq != mSeq)
    {
        assert (it->second.mSeq < mSeq);
        it->second.mEntry = std::make_shared<STLedgerEntry> (*it->second.mEntry);
        it->second.mSeq = mSeq;
    }

    action = it->second.mAction;

    return it->second.mEntry;
}

SLE::pointer LedgerEntrySet::entryCreate (LedgerEntryType letType, uint256 const& index)
{
    assert (index.isNonZero ());
    SLE::pointer sleNew = std::make_shared<SLE> (letType, index);
    entryCreate (sleNew);

    return sleNew;
}

SLE::pointer LedgerEntrySet::entryCache (LedgerEntryType letType, uint256 const& index)
{
    assert (mLedger);
    SLE::pointer sleEntry;

    if (index.isNonZero ())
    {
        LedgerEntryAction action;
        sleEntry = getEntry (index, action);

        if (!sleEntry)
        {
            assert (action != taaDELETE);
            sleEntry = mImmutable ? mLedger->getSLEi (index) : mLedger->getSLE (index);

            if (sleEntry)
                entryCache (sleEntry);
        }
        else if (action == taaDELETE)
        {
            sleEntry.reset ();
        }
    }

    return sleEntry;
}

void LedgerEntrySet::entryCache (SLE::ref sle)
{
    assert (mLedger);
    assert (sle->isMutable () || mImmutable); // Don't put an immutable SLE in a mutable LES
    auto it = mEntries.find (sle->getIndex ());

    if (it == mEntries.end ())
    {
        mEntries.insert (std::make_pair (sle->getIndex (), LedgerEntrySetEntry (sle, taaCACHED, mSeq)));
        return;
    }

    switch (it->second.mAction)
    {
    case taaCACHED:
        assert (sle == it->second.mEntry);
        it->second.mSeq     = mSeq;
        it->second.mEntry   = sle;
        return;

    default:
        throw std::runtime_error ("Cache after modify/delete/create");
    }
}

void LedgerEntrySet::entryCreate (SLE::ref sle)
{
    assert (mLedger && !mImmutable);
    assert (sle->isMutable ());

    auto it = mEntries.find (sle->getIndex ());

    if (it == mEntries.end ())
    {
        mEntries.insert (std::make_pair (sle->getIndex (), LedgerEntrySetEntry (sle, taaCREATE, mSeq)));
        return;
    }

    switch (it->second.mAction)
    {

    case taaDELETE:
        WriteLog (lsDEBUG, LedgerEntrySet) << "Create after Delete = Modify";
        it->second.mEntry = sle;
        it->second.mAction = taaMODIFY;
        it->second.mSeq = mSeq;
        break;

    case taaMODIFY:
        throw std::runtime_error ("Create after modify");

    case taaCREATE:
        throw std::runtime_error ("Create after create"); // This could be made to work

    case taaCACHED:
        throw std::runtime_error ("Create after cache");

    default:
        throw std::runtime_error ("Unknown taa");
    }

    assert (it->second.mSeq == mSeq);
}

void LedgerEntrySet::entryModify (SLE::ref sle)
{
    assert (sle->isMutable () && !mImmutable);
    assert (mLedger);
    auto it = mEntries.find (sle->getIndex ());

    if (it == mEntries.end ())
    {
        mEntries.insert (std::make_pair (sle->getIndex (), LedgerEntrySetEntry (sle, taaMODIFY, mSeq)));
        return;
    }

    assert (it->second.mSeq == mSeq);
    assert (it->second.mEntry == sle);

    switch (it->second.mAction)
    {
    case taaCACHED:
        it->second.mAction  = taaMODIFY;

        // Fall through

    case taaCREATE:
    case taaMODIFY:
        it->second.mSeq     = mSeq;
        it->second.mEntry   = sle;
        break;

    case taaDELETE:
        throw std::runtime_error ("Modify after delete");

    default:
        throw std::runtime_error ("Unknown taa");
    }
}

void LedgerEntrySet::entryDelete (SLE::ref sle)
{
    assert (sle->isMutable () && !mImmutable);
    assert (mLedger);
    auto it = mEntries.find (sle->getIndex ());

    if (it == mEntries.end ())
    {
        assert (false); // deleting an entry not cached?

        mEntries.insert (std::make_pair (sle->getIndex (), LedgerEntrySetEntry (sle, taaDELETE, mSeq)));

        return;
    }

    assert (it->second.mSeq == mSeq);
    assert (it->second.mEntry == sle);

    switch (it->second.mAction)
    {
    case taaCACHED:
    case taaMODIFY:
        it->second.mSeq     = mSeq;
        it->second.mEntry   = sle;
        it->second.mAction  = taaDELETE;
        break;

    case taaCREATE:
        mEntries.erase (it);
        break;

    case taaDELETE:
        break;

    default:
        throw std::runtime_error ("Unknown taa");
    }
}

Json::Value LedgerEntrySet::getJson (int) const
{
    Json::Value ret (Json::objectValue);

    Json::Value nodes (Json::arrayValue);

    for (auto it = mEntries.begin (), end = mEntries.end (); it != end; ++it)
    {
        Json::Value entry (Json::objectValue);
        entry[jss::node] = to_string (it->first);

        switch (it->second.mEntry->getType ())
        {
        case ltINVALID:
            entry[jss::type] = "invalid";
            break;

        case ltACCOUNT_ROOT:
            entry[jss::type] = "acccount_root";
            break;

        case ltDIR_NODE:
            entry[jss::type] = "dir_node";
            break;

        case ltBESSEL_STATE:
            entry[jss::type] = "bessel_state";
            break;

        case ltNICKNAME:
            entry[jss::type] = "nickname";
            break;

        case ltOFFER:
            entry[jss::type] = "offer";
            break;

        default:
            assert (false);
        }

        switch (it->second.mAction)
        {
        case taaCACHED:
            entry[jss::action] = "cache";
            break;

        case taaMODIFY:
            entry[jss::action] = "modify";
            break;

        case taaDELETE:
            entry[jss::action] = "delete";
            break;

        case taaCREATE:
            entry[jss::action] = "create";
            break;

        default:
            assert (false);
        }

        nodes.append (entry);
    }

    ret[jss::nodes] = nodes;

    ret[jss::metaData] = mSet.getJson (0);

    return ret;
}

SLE::pointer LedgerEntrySet::getForMod (uint256 const& node, Ledger::ref ledger,
                                        NodeToLedgerEntry& newMods)
{
    auto it = mEntries.find (node);

    if (it != mEntries.end ())
    {
        if (it->second.mAction == taaDELETE)
        {
            WriteLog (lsFATAL, LedgerEntrySet) << "Trying to thread to deleted node";

            return SLE::pointer ();
        }

        if (it->second.mAction == taaCACHED)
            it->second.mAction = taaMODIFY;

        if (it->second.mSeq != mSeq)
        {
            it->second.mEntry = std::make_shared<STLedgerEntry> (*it->second.mEntry);
            it->second.mSeq = mSeq;
        }

        return it->second.mEntry;
    }

    auto me = newMods.find (node);

    if (me != newMods.end ())
    {
        assert (me->second);

        return me->second;
    }

    SLE::pointer ret = ledger->getSLE (node);

    if (ret)
        newMods.insert (std::make_pair (node, ret));

    return ret;
}

bool LedgerEntrySet::threadTx (BesselAddress const& threadTo, 
                               Ledger::ref ledger,
                               NodeToLedgerEntry& newMods)
{
    SLE::pointer sle = getForMod (getAccountRootIndex (threadTo.getAccountID ()), ledger, newMods);

#ifdef META_DEBUG
    WriteLog (lsTRACE, LedgerEntrySet) << "Thread to " << threadTo.getAccountID ();
#endif

    if (!sle)
    {
        WriteLog (lsFATAL, LedgerEntrySet) << "Threading to non-existent account: " << threadTo.humanAccountID ();

        assert (false);

        return false;
    }

    return threadTx (sle, ledger, newMods);
}

bool LedgerEntrySet::threadTx (SLE::ref threadTo, 
                               Ledger::ref ledger,
                               NodeToLedgerEntry& newMods)
{
    // node = the node that was modified/deleted/created
    // threadTo = the node that needs to know
    uint256 prevTxID;
    std::uint32_t prevLgrID;

    if (!threadTo->thread (mSet.getTxID (), mSet.getLgrSeq (), prevTxID, prevLgrID))
        return false;

    if (prevTxID.isZero () ||
            TransactionMetaSet::thread (mSet.getAffectedNode (threadTo, sfModifiedNode), prevTxID, prevLgrID))
    {
        return true;
    }

    assert (false);

    return false;
}

bool LedgerEntrySet::threadOwners (SLE::ref node, Ledger::ref ledger,
                                   NodeToLedgerEntry& newMods)
{
    // thread new or modified node to owner or owners
    if (node->hasOneOwner ()) // thread to owner's account
    {
#ifdef META_DEBUG
        WriteLog (lsTRACE, LedgerEntrySet) << "Thread to single owner";
#endif
        return threadTx (node->getOwner (), ledger, newMods);
    }
    else if (node->hasTwoOwners ()) // thread to owner's accounts
    {
#ifdef META_DEBUG
        WriteLog (lsTRACE, LedgerEntrySet) << "Thread to two owners";
#endif
        return
            threadTx (node->getFirstOwner (), ledger, newMods) &&
            threadTx (node->getSecondOwner (), ledger, newMods);
    }
    else
    {
        return false;
    }
}

void LedgerEntrySet::calcRawMeta (Serializer& s, TER result, std::uint32_t index)
{
    // calculate the raw meta data and return it. This must be called before the set is committed

    // Entries modified only as a result of building the transaction metadata
    NodeToLedgerEntry newMod;

    for (auto& it : mEntries)
    {
        auto type = &sfGeneric;

        switch (it.second.mAction)
        {
        case taaMODIFY:
#ifdef META_DEBUG
            WriteLog (lsTRACE, LedgerEntrySet) << "Modified Node " << it.first;
#endif
            type = &sfModifiedNode;
            break;

        case taaDELETE:
#ifdef META_DEBUG
            WriteLog (lsTRACE, LedgerEntrySet) << "Deleted Node " << it.first;
#endif
            type = &sfDeletedNode;
            break;

        case taaCREATE:
#ifdef META_DEBUG
            WriteLog (lsTRACE, LedgerEntrySet) << "Created Node " << it.first;
#endif
            type = &sfCreatedNode;
            break;

        default: // ignore these
            break;
        }

        if (type == &sfGeneric)
            continue;

        SLE::pointer origNode = mLedger->getSLEi (it.first);
        SLE::pointer curNode = it.second.mEntry;

        if ((type == &sfModifiedNode) && (*curNode == *origNode))
            continue;

        std::uint16_t nodeType = curNode
            ? curNode->getFieldU16 (sfLedgerEntryType)
            : origNode->getFieldU16 (sfLedgerEntryType);

        mSet.setAffectedNode (it.first, *type, nodeType);

        if (type == &sfDeletedNode)
        {
            assert (origNode && curNode);
            threadOwners (origNode, mLedger, newMod); // thread transaction to owners

            STObject prevs (sfPreviousFields);
            for (auto const& obj : *origNode)
            {
                // go through the original node for modified fields saved on modification
                if (obj.getFName ().shouldMeta (SField::sMD_ChangeOrig) && !curNode->hasMatchingEntry (obj))
                    prevs.emplace_back (obj);
            }

            if (!prevs.empty ())
                mSet.getAffectedNode (it.first).emplace_back (std::move(prevs));

            STObject finals (sfFinalFields);
            for (auto const& obj : *curNode)
            {
                // go through the final node for final fields
                if (obj.getFName ().shouldMeta (SField::sMD_Always | SField::sMD_DeleteFinal))
                    finals.emplace_back (obj);
            }

            if (!finals.empty ())
                mSet.getAffectedNode (it.first).emplace_back (std::move(finals));
        }
        else if (type == &sfModifiedNode)
        {
            assert (curNode && origNode);

            if (curNode->isThreadedType ()) // thread transaction to node it modified
                threadTx (curNode, mLedger, newMod);

            STObject prevs (sfPreviousFields);
            for (auto const& obj : *origNode)
            {
                // search the original node for values saved on modify
                if (obj.getFName().shouldMeta(SField::sMD_ChangeOrig) && !curNode->hasMatchingEntry(obj))
                    prevs.emplace_back (obj);
            }

            if (!prevs.empty ())
                mSet.getAffectedNode (it.first).emplace_back (std::move(prevs));

            STObject finals (sfFinalFields);
            for (auto const& obj : *curNode)
            {
                // search the final node for values saved always
                if (obj.getFName ().shouldMeta (SField::sMD_Always | SField::sMD_ChangeNew))
                    finals.emplace_back (obj);
            }

            if (!finals.empty ())
                mSet.getAffectedNode (it.first).emplace_back (std::move(finals));
        }
        else if (type == &sfCreatedNode) // if created, thread to owner(s)
        {
            assert (curNode && !origNode);
            threadOwners (curNode, mLedger, newMod);

            if (curNode->isThreadedType ()) // always thread to self
                threadTx (curNode, mLedger, newMod);

            STObject news (sfNewFields);
            for (auto const& obj : *curNode)
            {
                // save non-default values
                if (!obj.isDefault () && obj.getFName ().shouldMeta (SField::sMD_Create | SField::sMD_Always))
                    news.emplace_back (obj);
            }

            if (!news.empty ())
                mSet.getAffectedNode (it.first).emplace_back (std::move(news));
        }
        else 
        {
            assert (false);
        }
    }

    // add any new modified nodes to the modification set
    for (auto& it : newMod)
        entryModify (it.second);

    mSet.addRaw (s, result, index);
    WriteLog (lsTRACE, LedgerEntrySet) << "Metadata:" << mSet.getJson (0);
}

TER LedgerEntrySet::dirCount (uint256 const& uRootIndex, std::uint32_t& uCount)
{
    std::uint64_t  uNodeDir    = 0;

    uCount  = 0;

    do
    {
        SLE::pointer    sleNode = entryCache (ltDIR_NODE, getDirNodeIndex (uRootIndex, uNodeDir));

        if (sleNode)
        {
            uCount      += sleNode->getFieldV256 (sfIndexes).size ();

            uNodeDir    = sleNode->getFieldU64 (sfIndexNext);       // Get next node.
        }
        else if (uNodeDir)
        {
            WriteLog (lsWARNING, LedgerEntrySet) << "dirCount: no such node";

            assert (false);

            return tefBAD_LEDGER;
        }
    }while (uNodeDir);

    return tesSUCCESS;
}

bool LedgerEntrySet::dirIsEmpty (uint256 const& uRootIndex)
{
    std::uint64_t  uNodeDir = 0;

    SLE::pointer sleNode = entryCache (ltDIR_NODE, getDirNodeIndex (uRootIndex, uNodeDir));

    if (!sleNode)
        return true;

    if (!sleNode->getFieldV256 (sfIndexes).empty ())
        return false;

    // If there's another page, it must be non-empty
    return sleNode->getFieldU64 (sfIndexNext) == 0;
}

// <--     uNodeDir: For deletion, present to make dirDelete efficient.
// -->   uRootIndex: The index of the base of the directory.  Nodes are based off of this.
// --> uLedgerIndex: Value to add to directory.
// Only append. This allow for things that watch append only structure to just monitor from the last node on ward.
// Within a node with no deletions order of elements is sequential.  Otherwise, order of elements is random.
TER LedgerEntrySet::dirAdd (
    std::uint64_t&                          uNodeDir,
    uint256 const&                          uRootIndex,
    uint256 const&                          uLedgerIndex,
    std::function<void (SLE::ref, bool)>    fDescriber)
{
    WriteLog (lsTRACE, LedgerEntrySet) << "dirAdd:" 
                                       << " uRootIndex=" 
                                       << to_string (uRootIndex) 
                                       << " uLedgerIndex=" 
                                       << to_string (uLedgerIndex);

    SLE::pointer        sleNode;
    STVector256         svIndexes;
    SLE::pointer        sleRoot     = entryCache (ltDIR_NODE, uRootIndex);

    if (!sleRoot)
    {
        // No root, make it.
        sleRoot     = entryCreate (ltDIR_NODE, uRootIndex);
        sleRoot->setFieldH256 (sfRootIndex, uRootIndex);
        fDescriber (sleRoot, true);

        sleNode     = sleRoot;
        uNodeDir    = 0;
    }
    else
    {
        uNodeDir    = sleRoot->getFieldU64 (sfIndexPrevious);       // Get index to last directory node.

        if (uNodeDir)
        {
            // Try adding to last node.
            sleNode     = entryCache (ltDIR_NODE, getDirNodeIndex (uRootIndex, uNodeDir));

            assert (sleNode);
        }
        else
        {
            // Try adding to root.  Didn't have a previous set to the last node.
            sleNode     = sleRoot;
        }

        svIndexes   = sleNode->getFieldV256 (sfIndexes);

        if (DIR_NODE_MAX != svIndexes.size ())
        {
            // Add to current node.
            entryModify (sleNode);
        }
        // Add to new node.
        else if (!++uNodeDir)
        {
            return tecDIR_FULL;
        }
        else
        {
            // Have old last point to new node
            sleNode->setFieldU64 (sfIndexNext, uNodeDir);
            entryModify (sleNode);

            // Have root point to new node.
            sleRoot->setFieldU64 (sfIndexPrevious, uNodeDir);
            entryModify (sleRoot);

            // Create the new node.
            sleNode     = entryCreate (ltDIR_NODE, getDirNodeIndex (uRootIndex, uNodeDir));
            sleNode->setFieldH256 (sfRootIndex, uRootIndex);

            if (uNodeDir != 1)
                sleNode->setFieldU64 (sfIndexPrevious, uNodeDir - 1);

            fDescriber (sleNode, false);

            svIndexes   = STVector256 ();
        }
    }

    svIndexes.push_back (uLedgerIndex); // Append entry.
    sleNode->setFieldV256 (sfIndexes, svIndexes);   // Save entry.

    WriteLog (lsTRACE, LedgerEntrySet) <<
        "dirAdd:   creating: root: " << to_string (uRootIndex);
    WriteLog (lsTRACE, LedgerEntrySet) <<
        "dirAdd:  appending: Entry: " << to_string (uLedgerIndex);
    WriteLog (lsTRACE, LedgerEntrySet) <<
        "dirAdd:  appending: Node: " << strHex (uNodeDir);

    return tesSUCCESS;
}

// Ledger must be in a state for this to work.
TER LedgerEntrySet::dirDelete (
    const bool                      bKeepRoot,      // --> True, if we never completely clean up, after we overflow the root node.
    const std::uint64_t&            uNodeDir,       // --> Node containing entry.
    uint256 const&                  uRootIndex,     // --> The index of the base of the directory.  Nodes are based off of this.
    uint256 const&                  uLedgerIndex,   // --> Value to remove from directory.
    const bool                      bStable,        // --> True, not to change relative order of entries.
    const bool                      bSoft)          // --> True, uNodeDir is not hard and fast (pass uNodeDir=0).
{
    std::uint64_t       uNodeCur    = uNodeDir;
    SLE::pointer        sleNode     = entryCache (ltDIR_NODE, getDirNodeIndex (uRootIndex, uNodeCur));

    if (!sleNode)
    {
        WriteLog (lsWARNING, LedgerEntrySet) << "dirDelete: no such node:" 
                                             << " uRootIndex=" 
                                             << to_string (uRootIndex)
                                             << " uNodeDir=" 
                                             << strHex (uNodeDir) 
                                             << " uLedgerIndex=" 
                                             << to_string (uLedgerIndex);

        if (!bSoft)
        {
            assert (false);

            return tefBAD_LEDGER;
        }
        else if (uNodeDir < 20)
        {
            // Go the extra mile. Even if node doesn't exist, try the next node.

            return dirDelete (bKeepRoot, uNodeDir + 1, uRootIndex, uLedgerIndex, bStable, true);
        }
        else
        {
            return tefBAD_LEDGER;
        }
    }

    STVector256 svIndexes = sleNode->getFieldV256 (sfIndexes);

    auto it = std::find (svIndexes.begin (), svIndexes.end (), uLedgerIndex);

    if (svIndexes.end () == it)
    {
        if (!bSoft)
        {
            assert (false);
            WriteLog (lsWARNING, LedgerEntrySet) << "dirDelete: no such entry";

            return tefBAD_LEDGER;
        }

        if (uNodeDir < 20)
        {
            // Go the extra mile. Even if entry not in node, try the next node.
            return dirDelete (bKeepRoot, uNodeDir + 1, uRootIndex, uLedgerIndex, bStable, true);
        }

        return tefBAD_LEDGER;
    }

    // Remove the element.
    if (svIndexes.size () > 1)
    {
        if (bStable)
        {
            svIndexes.erase (it);
        }
        else
        {
            *it = svIndexes[svIndexes.size () - 1];
            svIndexes.resize (svIndexes.size () - 1);
        }
    }
    else
    {
        svIndexes.clear ();
    }

    sleNode->setFieldV256 (sfIndexes, svIndexes);
    entryModify (sleNode);

    if (svIndexes.empty ())
    {
        // May be able to delete nodes.
        std::uint64_t       uNodePrevious   = sleNode->getFieldU64 (sfIndexPrevious);
        std::uint64_t       uNodeNext       = sleNode->getFieldU64 (sfIndexNext);

        if (!uNodeCur)
        {
            // Just emptied root node.

            if (!uNodePrevious)
            {
                // Never overflowed the root node.  Delete it.
                entryDelete (sleNode);
            }
            // Root overflowed.
            else if (bKeepRoot)
            {
                // If root overflowed and not allowed to delete overflowed root node.
            }
            else if (uNodePrevious != uNodeNext)
            {
                // Have more than 2 nodes.  Can't delete root node.
            }
            else
            {
                // Have only a root node and a last node.
                SLE::pointer        sleLast = entryCache (ltDIR_NODE, getDirNodeIndex (uRootIndex, uNodeNext));

                assert (sleLast);

                if (sleLast->getFieldV256 (sfIndexes).empty ())
                {
                    // Both nodes are empty.

                    entryDelete (sleNode);  // Delete root.
                    entryDelete (sleLast);  // Delete last.
                }
                else
                {
                    // Have an entry, can't delete root node.
                }
            }
        }
        // Just emptied a non-root node.
        else if (uNodeNext)
        {
            // Not root and not last node. Can delete node.

            SLE::pointer        slePrevious = entryCache (ltDIR_NODE, getDirNodeIndex (uRootIndex, uNodePrevious));

            assert (slePrevious);

            SLE::pointer        sleNext     = entryCache (ltDIR_NODE, getDirNodeIndex (uRootIndex, uNodeNext));

            assert (slePrevious);
            assert (sleNext);

            if (!slePrevious)
            {
                WriteLog (lsWARNING, LedgerEntrySet) << "dirDelete: previous node is missing";

                return tefBAD_LEDGER;
            }

            if (!sleNext)
            {
                WriteLog (lsWARNING, LedgerEntrySet) << "dirDelete: next node is missing";

                return tefBAD_LEDGER;
            }

            // Fix previous to point to its new next.
            slePrevious->setFieldU64 (sfIndexNext, uNodeNext);
            entryModify (slePrevious);

            // Fix next to point to its new previous.
            sleNext->setFieldU64 (sfIndexPrevious, uNodePrevious);
            entryModify (sleNext);

            entryDelete(sleNode);
        }
        // Last node.
        else if (bKeepRoot || uNodePrevious)
        {
            // Not allowed to delete last node as root was overflowed.
            // Or, have pervious entries preventing complete delete.
        }
        else
        {
            // Last and only node besides the root.
            SLE::pointer            sleRoot = entryCache (ltDIR_NODE, uRootIndex);

            assert (sleRoot);

            if (sleRoot->getFieldV256 (sfIndexes).empty ())
            {
                // Both nodes are empty.

                entryDelete (sleRoot);  // Delete root.
                entryDelete (sleNode);  // Delete last.
            }
            else
            {
                // Root has an entry, can't delete.
            }
        }
    }

    return tesSUCCESS;
}

// Return the first entry and advance uDirEntry.
// <-- true, if had a next entry.
bool LedgerEntrySet::dirFirst (
    uint256 const& uRootIndex,  // --> Root of directory.
    SLE::pointer& sleNode,      // <-- current node
    unsigned int& uDirEntry,    // <-- next entry
    uint256& uEntryIndex)       // <-- The entry, if available. Otherwise, zero.
{
    sleNode     = entryCache (ltDIR_NODE, uRootIndex);
    uDirEntry   = 0;

    assert (sleNode);           // Never probe for directories.

    return LedgerEntrySet::dirNext (uRootIndex, sleNode, uDirEntry, uEntryIndex);
}

// Return the current entry and advance uDirEntry.
// <-- true, if had a next entry.
bool LedgerEntrySet::dirNext (
    uint256 const& uRootIndex,  // --> Root of directory
    SLE::pointer& sleNode,      // <-> current node
    unsigned int& uDirEntry,    // <-> next entry
    uint256& uEntryIndex)       // <-- The entry, if available. Otherwise, zero.
{
    STVector256             svIndexes   = sleNode->getFieldV256 (sfIndexes);

    assert (uDirEntry <= svIndexes.size ());

    if (uDirEntry >= svIndexes.size ())
    {
        std::uint64_t         uNodeNext   = sleNode->getFieldU64 (sfIndexNext);

        if (!uNodeNext)
        {
            uEntryIndex.zero ();

            return false;
        }
        else
        {
            SLE::pointer sleNext = entryCache (ltDIR_NODE, getDirNodeIndex (uRootIndex, uNodeNext));
            uDirEntry   = 0;

            if (!sleNext)
            { // This should never happen
                WriteLog (lsFATAL, LedgerEntrySet)
                        << "Corrupt directory: index:"
                        << uRootIndex << " next:" << uNodeNext;

                return false;
            }

            sleNode = sleNext;
            // TODO(tom): make this iterative.
            return dirNext (uRootIndex, sleNode, uDirEntry, uEntryIndex);
        }
    }

    uEntryIndex = svIndexes[uDirEntry++];

    WriteLog (lsTRACE, LedgerEntrySet) << "dirNext:" 
                                       << " uDirEntry=" 
                                       << uDirEntry 
                                       << " uEntryIndex=" 
                                       << uEntryIndex;

    return true;
}

uint256 LedgerEntrySet::getNextLedgerIndex (uint256 const& uHash)
{
    // find next node in ledger that isn't deleted by LES
    uint256 ledgerNext = uHash;
    std::map<uint256, LedgerEntrySetEntry>::const_iterator it;

    do
    {
        ledgerNext = mLedger->getNextLedgerIndex (ledgerNext);
        it  = mEntries.find (ledgerNext);
    }
    while ((it != mEntries.end ()) && (it->second.mAction == taaDELETE));

    // find next node in LES that isn't deleted
    for (it = mEntries.upper_bound (uHash); it != mEntries.end (); ++it)
    {
        // node found in LES, node found in ledger, return earliest
        if (it->second.mAction != taaDELETE)
            return (ledgerNext.isNonZero () && (ledgerNext < it->first)) ?
                    ledgerNext : it->first;
    }

    // nothing next in LES, return next ledger node
    return ledgerNext;
}

uint256 LedgerEntrySet::getNextLedgerIndex (
    uint256 const& uHash, uint256 const& uEnd)
{
    uint256 next = getNextLedgerIndex (uHash);

    if (next > uEnd)
        return uint256 ();

    return next;
}

void LedgerEntrySet::incrementOwnerCount (SLE::ref sleAccount)
{
    assert (sleAccount);

    std::uint32_t const current_count = sleAccount->getFieldU32 (sfOwnerCount);

    if (current_count == std::numeric_limits<std::uint32_t>::max ())
    {
        WriteLog (lsFATAL, LedgerEntrySet) << "Account " 
                                           << sleAccount->getFieldAccount160 (sfAccount) 
                                           << " owner count exceeds max!";
        return;
    }

    sleAccount->setFieldU32 (sfOwnerCount, current_count + 1);
    entryModify (sleAccount);
}

void LedgerEntrySet::incrementOwnerCount (Account const& owner)
{
    incrementOwnerCount(entryCache (ltACCOUNT_ROOT,
        getAccountRootIndex (owner)));
}

void LedgerEntrySet::decrementOwnerCount (SLE::ref sleAccount)
{
    assert (sleAccount);

    std::uint32_t const current_count = sleAccount->getFieldU32 (sfOwnerCount);

    if (current_count == 0)
    {
        WriteLog (lsFATAL, LedgerEntrySet) << "Account " 
                                           << sleAccount->getFieldAccount160 (sfAccount) 
                                           << " owner count is already 0!";
        return;
    }

    sleAccount->setFieldU32 (sfOwnerCount, current_count - 1);
    entryModify (sleAccount);
}

void LedgerEntrySet::decrementOwnerCount (Account const& owner)
{
    decrementOwnerCount(entryCache (ltACCOUNT_ROOT,
        getAccountRootIndex (owner)));
}


// Return how much of issuer's currency IOUs that account holds.  May be
// negative.
// <-- IOU's account has of issuer.
STAmount LedgerEntrySet::besselHolds (
    Account const& account,
    Currency const& currency,
    Account const& issuer,
    FreezeHandling zeroIfFrozen)
{
    STAmount saBalance;
    SLE::pointer sleBesselState = entryCache (ltBESSEL_STATE,
        getBesselStateIndex (account, issuer, currency));

    if (!sleBesselState)
    {
        saBalance.clear ({currency, issuer});
    }
    else if ((zeroIfFrozen == fhZERO_IF_FROZEN) && isFrozen (account, currency, issuer))
    {
        saBalance.clear (IssueRef (currency, issuer));
    }
    else if (account > issuer)
    {
        saBalance   = sleBesselState->getFieldAmount (sfBalance);
        saBalance.negate ();    // Put balance in account terms.

        saBalance.setIssuer (issuer);
    }
    else
    {
        saBalance   = sleBesselState->getFieldAmount (sfBalance);

        saBalance.setIssuer (issuer);
    }

    return adjustedBalance(account, issuer, saBalance);
}

// Returns the amount an account can spend without going into debt.
//
// <-- saAmount: amount of currency held by account. May be negative.
STAmount LedgerEntrySet::accountHolds (
    Account const& account,
    Currency const& currency,
    Account const& issuer,
    FreezeHandling zeroIfFrozen)
{
    STAmount    saAmount;

    if (!currency)
    {
        SLE::pointer sleAccount = entryCache (ltACCOUNT_ROOT,
            getAccountRootIndex (account));
        std::uint64_t uReserve = mLedger->getReserve (
            sleAccount->getFieldU32 (sfOwnerCount));

        STAmount saBalance   = sleAccount->getFieldAmount (sfBalance);

        if (saBalance < uReserve)
        {
            saAmount.clear ();
        }
        else
        {
            saAmount = saBalance - uReserve;
        }

        WriteLog (lsTRACE, LedgerEntrySet) << "accountHolds:" <<
            " account="    << to_string (account)      <<
            " saAmount="   << saAmount.getFullText ()  <<
            " saBalance="  << saBalance.getFullText () <<
            " uReserve="   << uReserve;

        return adjustedBalance(account, issuer, saAmount);
    }
    else
    {
        saAmount = besselHolds (account, currency, issuer, zeroIfFrozen);

        WriteLog (lsTRACE, LedgerEntrySet) << "accountHolds:" <<
            " account="  << to_string (account) <<
            " saAmount=" << saAmount.getFullText ();

        return saAmount;
    }

}


// Can the specified account spend the specified currency issued by
// the specified issuer or does the freeze flag prohibit it?

// Returns the funds available for account for a currency/issuer.
// Use when you need a default for rippling account's currency.
// XXX Should take into account quality?
// --> saDefault/currency/issuer
// <-- saFunds: Funds available. May be negative.
//
// If the issuer is the same as account, funds are unlimited, use result is
// saDefault.
STAmount LedgerEntrySet::accountFunds (
    Account const& account, STAmount const& saDefault, FreezeHandling zeroIfFrozen)
{
    STAmount    saFunds;

    if (!saDefault.isNative () && saDefault.getIssuer () == account)
    {
        saFunds = saDefault;

        WriteLog (lsTRACE, LedgerEntrySet) << "accountFunds:" <<
            " account="   << to_string (account)      <<
            " saDefault=" << saDefault.getFullText () <<
            " SELF-FUNDED";
    }
    else
    {
        saFunds = accountHolds (
            account, saDefault.getCurrency (), saDefault.getIssuer (),
            zeroIfFrozen);

        WriteLog (lsTRACE, LedgerEntrySet) << "accountFunds:" <<
            " account="   << to_string (account) <<
            " saDefault=" << saDefault.getFullText () <<
            " saFunds="   << saFunds.getFullText ();
    }

    return saFunds;
}


// Calculate the fee needed to transfer IOU assets between two parties.
STAmount LedgerEntrySet::besselTransferFee (
    Account const& from,
    Account const& to,
    Account const& issuer,
    STAmount const& saAmount)
{
    if (from != issuer && to != issuer)
    {
        std::uint32_t uTransitRate = besselTransferRate (*this, issuer,saAmount.getCurrency());
       
        if (QUALITY_ONE != uTransitRate)
        {
            STAmount saTransferTotal = multiply (saAmount, amountFromRate (uTransitRate), saAmount.issue ());
            STAmount saTransferFee = saTransferTotal - saAmount;

            WriteLog (lsDEBUG, LedgerEntrySet) << "besselTransferFee:" 
                                               << " saTransferFee=" 
                                               << saTransferFee.getFullText ();

            return saTransferFee;
        }
    }

    return saAmount.zeroed();
}


bool LedgerEntrySet::checkState (
    SLE::pointer state,
    bool bSenderHigh,
    Account const& sender,
    STAmount const& before,
    STAmount const& after)
{
    std::uint32_t const flags (state->getFieldU32 (sfFlags));

    auto sender_account = entryCache (ltACCOUNT_ROOT,
        getAccountRootIndex (sender));
    assert (sender_account);

    // YYY Could skip this if rippling in reverse.
    if (before > zero
        // Sender balance was positive.
        && after <= zero
        // Sender is zero or negative.
        && (flags & (!bSenderHigh ? lsfLowReserve : lsfHighReserve))
        // Sender reserve is set.
        && static_cast <bool> (flags & (!bSenderHigh ? lsfLowNoBessel : lsfHighNoBessel)) ==
           static_cast <bool> (sender_account->getFlags() & lsfDefaultBessel)
        && !(flags & (!bSenderHigh ? lsfLowFreeze : lsfHighFreeze))
        && !state->getFieldAmount (
            !bSenderHigh ? sfLowLimit : sfHighLimit)
        // Sender trust limit is 0.
        && !state->getFieldU32 (
            !bSenderHigh ? sfLowQualityIn : sfHighQualityIn)
        // Sender quality in is 0.
        && !state->getFieldU32 (
            !bSenderHigh ? sfLowQualityOut : sfHighQualityOut))
        // Sender quality out is 0.
    {
        // Clear the reserve of the sender, possibly delete the line!
        decrementOwnerCount (sender_account);

        // Clear reserve flag.
        state->setFieldU32 (sfFlags,
            flags & (!bSenderHigh ? ~lsfLowReserve : ~lsfHighReserve));

        // Balance is zero, receiver reserve is clear.
        if (!after        // Balance is zero.
            && !(flags & (bSenderHigh ? lsfLowReserve : lsfHighReserve)))
            return true;
    }

    return false;
}


std::uint32_t
besselTransferRate (LedgerEntrySet& ledger, Account const& issuer,Currency const& currency)
{
    SLE::pointer sleAccount(ledger.entryCache(
        ltACCOUNT_ROOT, getAccountRootIndex(issuer)));

    std::uint32_t quality = QUALITY_ONE;
    
    if (currency == noCurrency())
    {
        if (sleAccount && sleAccount->isFieldPresent(sfTransferRate))
            quality = sleAccount->getFieldU32(sfTransferRate);
    }
    else
    {
        bool flag = true;
        if (sleAccount && sleAccount->isFieldPresent(sfCurrencyRates))
        {
            STArray const& CurrencyRates = sleAccount->getFieldArray(sfCurrencyRates);
            for (auto const& rate : CurrencyRates)
            {
                STAmount const& fee = rate.getFieldAmount(sfAmount);
                if (currency == fee.getCurrency())
                {
                    quality = boost::lexical_cast<std::uint32_t>(fee.getText());
                    flag = false;
                    break;
                }
            }
        } 
        if (flag)
        {
            if (sleAccount && sleAccount->isFieldPresent(sfTransferRate))
                quality = sleAccount->getFieldU32(sfTransferRate);
        }
    }
    return quality;
}

std::uint32_t
besselTransferRate (LedgerEntrySet& ledger, Account const& uSenderID,
Account const& uReceiverID, Account const& issuer, Currency const& currency)
{
    // If calculating the transfer rate from or to the issuer of the currency
    // no fees are assessed.
    return (uSenderID == issuer || uReceiverID == issuer)
           ? QUALITY_ONE
           : besselTransferRate (ledger, issuer,currency);
}

ScopedDeferCredits::ScopedDeferCredits (LedgerEntrySet& l)
    : les_ (l), enabled_ (false)
{
    if (!les_.areCreditsDeferred ())
    {
        WriteLog (lsTRACE, DeferredCredits) << "Enable";
        les_.enableDeferredCredits (true);
        enabled_ = true;
    }
}

ScopedDeferCredits::~ScopedDeferCredits ()
{
    if (enabled_)
    {
        WriteLog (lsTRACE, DeferredCredits) << "Disable";
        les_.enableDeferredCredits (false);
    }
}

} // bessel
