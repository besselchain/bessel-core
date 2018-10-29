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

#ifndef BESSEL_PROTOCOL_UINTTYPES_H_INCLUDED
#define BESSEL_PROTOCOL_UINTTYPES_H_INCLUDED

#include <common/base/UnorderedContainers.h>
#include <common/base/base_uint.h>

namespace bessel {
namespace detail {

class AccountTag {};
class CurrencyTag {};
class DirectoryTag {};
class NodeIDTag {};

} // detail

/** Directory is an index into the directory of offer books.
    The last 64 bits of this are the quality. */
typedef base_uint<256, detail::DirectoryTag> Directory;

/** Account is a hash representing a specific account. */
typedef base_uint<160, detail::AccountTag> Account;

/** Currency is a hash representing a specific currency. */
typedef base_uint<160, detail::CurrencyTag> Currency;

/** NodeID is a 160-bit hash representing one node. */
typedef base_uint<160, detail::NodeIDTag> NodeID;

typedef hash_set<Currency> CurrencySet;
typedef hash_set<NodeID> NodeIDSet;

/** A special account that's used as the "issuer" for SWT. */
Account const& xrpAccount();

/** SWT currency. */
Currency const& xrpCurrency();

/** A placeholder for empty accounts. */
Account const& noAccount();

/** A placeholder for empty currencies. */
Currency const& noCurrency();

/** We deliberately disallow the currency that looks like "SWT" because too
    many people were using it instead of the correct SWT currency. */


Currency const& badCurrency();

inline bool isSWT(Currency const& c)
{
    return c == zero;
}

inline bool isSWT(Account const& c)
{
    return c == zero;
}

/** Returns a human-readable form of the account. */
std::string to_string(Account const&);

/** Returns "", "SWT", or three letter ISO code. */
std::string to_string(Currency const& c);

/** Tries to convert a string to a Currency, returns true on success. */
bool to_currency(Currency&, std::string const&);

/** Tries to convert a string to a Currency, returns noCurrency() on failure. */
Currency to_currency(std::string const&);

/** Tries to convert a string to an Account representing an issuer, returns true
 * on success. */
bool to_issuer(Account&, std::string const&);

inline std::ostream& operator<< (std::ostream& os, Account const& x)
{
    os << to_string (x);
    return os;
}

inline std::ostream& operator<< (std::ostream& os, Currency const& x)
{
    os << to_string (x);
    return os;
}

} // bessel

namespace std {

template <>
struct hash <bessel::Account> : bessel::Account::hasher
{
};

template <>
struct hash <bessel::Currency> : bessel::Currency::hasher
{
};

template <>
struct hash <bessel::NodeID> : bessel::NodeID::hasher
{
};

template <>
struct hash <bessel::Directory> : bessel::Directory::hasher
{
};

} // std

#endif
