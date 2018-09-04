//------------------------------------------------------------------------------
/*
    This file is part of skywelld: https://github.com/skywell/skywelld
    Copyright (c) 2014 Skywell Labs Inc.

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

#ifndef SKYWELL_CRYPTO_EC_KEY_H_INCLUDED
#define SKYWELL_CRYPTO_EC_KEY_H_INCLUDED

#include <common/base/base_uint.h>
#include <cstdint>

namespace skywell {
namespace openssl {

class ec_key
{
public:
    typedef struct opaque_EC_KEY* pointer_t;

private:
    pointer_t ptr;

    void destroy();

    ec_key (pointer_t raw) : ptr(raw)
    {
    }

public:
    static const ec_key invalid;

    static ec_key acquire (pointer_t raw)  { return ec_key (raw); }

    //ec_key() : ptr() {}

    ec_key            (const ec_key&);
    ec_key& operator= (const ec_key&) = delete;

    ~ec_key()
    {
        destroy();
    }

    pointer_t get() const  { return ptr; }

    pointer_t release()
    {
        pointer_t released = ptr;

        ptr = nullptr;

        return released;
    }

    bool valid() const  { return ptr != nullptr; }
};

} // openssl
} // skywell

#endif
