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

#ifndef BESSEL_PROTOCOL_STACCOUNT_H_INCLUDED
#define BESSEL_PROTOCOL_STACCOUNT_H_INCLUDED

#include <protocol/BesselAddress.h>
#include <protocol/STBlob.h>
#include <string>

namespace bessel {

class STAccount final
    : public STBlob
{
public:
    STAccount (SField const& n, Buffer&& v)
            : STBlob (n, std::move(v))
    {
        ;
    }
    STAccount (SField const& n, Account const& v);
    STAccount (SField const& n) : STBlob (n)
    {
        ;
    }
    STAccount ()
    {
        ;
    }

    STAccount (SerialIter& sit, SField const& name);

    STBase*
    copy (std::size_t n, void* buf) const override
    {
        return emplace(n, buf, *this);
    }

    STBase*
    move (std::size_t n, void* buf) override
    {
        return emplace(n, buf, std::move(*this));
    }

    SerializedTypeID getSType () const override
    {
        return STI_ACCOUNT;
    }
    std::string getText () const override;

    BesselAddress getValueNCA () const;
    void setValueNCA (BesselAddress const& nca);

    template <typename Tag>
    void setValueH160 (base_uint<160, Tag> const& v)
    {
        peekValue () = Buffer (v.data (), v.size ());
        assert (peekValue ().size () == (160 / 8));
    }

    template <typename Tag>
    bool getValueH160 (base_uint<160, Tag>& v) const
    {
        auto success = isValueH160 ();
        if (success)
            memcpy (v.begin (), peekValue ().data (), (160 / 8));
        return success;
    }

    bool isValueH160 () const;

private:
    static STAccount* construct (SerialIter&, SField const&);
};

} // bessel

#endif
