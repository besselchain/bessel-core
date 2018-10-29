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

#ifndef BESSEL_APP_TX_TRANSACTIONMETA_H_INCLUDED
#define BESSEL_APP_TX_TRANSACTIONMETA_H_INCLUDED

#include <protocol/STLedgerEntry.h>
#include <protocol/STArray.h>
#include <protocol/TER.h>
#include <boost/optional.hpp>

namespace bessel {

class TransactionMetaSet
{
public:
    typedef std::shared_ptr<TransactionMetaSet> pointer;
    typedef const pointer& ref;

private:
    struct CtorHelper{};
    template<class T>
    TransactionMetaSet (uint256 const& txID, std::uint32_t ledger, T const& data,
                        CtorHelper);
public:
    TransactionMetaSet ()
        : mLedger (0)
        , mIndex (static_cast<std::uint32_t> (-1))
        , mResult (255)
    {
    }

    TransactionMetaSet (uint256 const& txID, std::uint32_t ledger, std::uint32_t index)
        : mTransactionID (txID)
        , mLedger (ledger)
        , mIndex (static_cast<std::uint32_t> (-1))
        , mResult (255)
    {
    }

    TransactionMetaSet (uint256 const& txID, std::uint32_t ledger, Blob const&);
    TransactionMetaSet (uint256 const& txID, std::uint32_t ledger, std::string const&);

    void init (uint256 const& transactionID, std::uint32_t ledger);
    void clear ()
    {
        mNodes.clear ();
    }
    void swap (TransactionMetaSet&) noexcept;

    uint256 const& getTxID ()
    {
        return mTransactionID;
    }
    std::uint32_t getLgrSeq ()
    {
        return mLedger;
    }
    int getResult () const
    {
        return mResult;
    }
    TER getResultTER () const
    {
        return static_cast<TER> (mResult);
    }
    std::uint32_t getIndex () const
    {
        return mIndex;
    }

    bool isNodeAffected (uint256 const& ) const;
    void setAffectedNode (uint256 const& , SField const& type,
                          std::uint16_t nodeType);
    STObject& getAffectedNode (SLE::ref node, SField const& type); // create if needed
    STObject& getAffectedNode (uint256 const& );
    const STObject& peekAffectedNode (uint256 const& ) const;
    std::vector<BesselAddress> getAffectedAccounts ();

    Json::Value getJson (int p) const
    {
        return getAsObject ().getJson (p);
    }
    void addRaw (Serializer&, TER, std::uint32_t index);

    STObject getAsObject () const;

    STArray& getNodes ()
    {
        return (mNodes);
    }

	void setNodes(STArray const& nodes)
	{
		mNodes = nodes;
	}

    void setDeliveredAmount (STAmount const& delivered)
    {
        mDelivered.reset (delivered);
    }

    STAmount getDeliveredAmount () const
    {
        assert (hasDeliveredAmount ());
        return *mDelivered;
    }

    bool hasDeliveredAmount () const
    {
        return static_cast <bool> (mDelivered);
    }

    static bool thread (STObject& node, uint256 const& prevTxID, std::uint32_t prevLgrID);

private:
    uint256       mTransactionID;
    std::uint32_t mLedger;
    std::uint32_t mIndex;
    int           mResult;

    boost::optional <STAmount> mDelivered;

    STArray mNodes;
};

} // bessel

#endif