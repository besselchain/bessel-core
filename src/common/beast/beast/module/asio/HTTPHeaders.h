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

#ifndef BEAST_MODULE_ASIO_HTTPHEADERS_H_INCLUDED
#define BEAST_MODULE_ASIO_HTTPHEADERS_H_INCLUDED

#include <beast/module/asio/HTTPField.h>
#include <beast/module/core/text/StringPairArray.h>
#include <map>
#include <beast/strings/String.h>
#include <beast/strings/NewLine.h>

namespace beast {

/** A set of HTTP headers. */
class HTTPHeaders
{
public:
    /** Construct an empty set of headers. */
    HTTPHeaders ();

    /** Construct headers taking ownership of a field array.
        The callers value is overwritten.
    */
    HTTPHeaders (StringPairArray& fields);

    /** Construct a copy of headers from an array.*/
    HTTPHeaders (StringPairArray const& fields);

    /** Construct a copy of headers. */
    HTTPHeaders (HTTPHeaders const& other);

    /** Assign a copy of headers. */
    HTTPHeaders& operator= (HTTPHeaders const& other);

    /** Returns `true` if the container is empty. */
    bool empty () const;

    /** Returns the number of fields in the container. */
    std::size_t size () const;

    /** Random access to fields by index. */
    /** @{ */
    HTTPField at (int index) const;
    HTTPField operator[] (int index) const;
    /** @} */

    /** Associative access to fields by name.
        If the field is not present, an empty string is returned.
    */
    /** @{ */
    String get (String const& field) const;
    String operator[] (String const& field) const;
    /** @} */

    /** Outputs all the headers into one string. */
    String toString () const;

    //  HACK to present the headers in a useful format
    std::map <std::string, std::string>
    build_map() const;

private:
    StringPairArray m_fields;
};

}

#endif
