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

#ifndef BESSEL_APP_LEDGER_ORDERBOOKDB_H_INCLUDED
#define BESSEL_APP_LEDGER_ORDERBOOKDB_H_INCLUDED

#include <ledger/AcceptedLedgerTx.h>
#include <ledger/BookListeners.h>
#include <common/misc/OrderBook.h>

namespace bessel {

class OrderBookDB : public beast::Stoppable
{
public:
    explicit OrderBookDB (Stoppable& parent);

    void setup (Ledger::ref ledger);
    void update (Ledger::pointer ledger);
    void invalidate ();

    void addOrderBook(Book const&);

    /** @return a list of all orderbooks that want this issuerID and currencyID.
     */
    OrderBook::List getBooksByTakerPays (Issue const&);

    /** @return a count of all orderbooks that want this issuerID and currencyID.
     */
    int getBookSize(Issue const&);

    bool isBookToSWT (Issue const&);

    BookListeners::pointer getBookListeners (Book const&);
    BookListeners::pointer makeBookListeners (Book const&);

    // see if this txn effects any orderbook
    void processTxn (
        Ledger::ref ledger, const AcceptedLedgerTx& alTx,
        Json::Value const& jvObj);

    typedef hash_map<Issue, OrderBook::List> IssueToOrderBook;

private:
    void rawAddBook(Book const&);

    // by ci/ii
    IssueToOrderBook mSourceMap;

    // by co/io
    IssueToOrderBook mDestMap;

    // does an order book to SWT exist
    hash_set<Issue> mSWTBooks;

    typedef BesselRecursiveMutex LockType;
    typedef std::lock_guard<LockType> ScopedLockType;
    LockType mLock;

    typedef hash_map<Book, BookListeners::pointer>
    BookToListenersMap;

    BookToListenersMap mListeners;

    std::uint32_t mSeq;
};

} // bessel

#endif
