//------------------------------------------------------------------------------
/*
    This file is part of skywelld: https://github.com/skywell/skywelld
    Copyright (c) 2012, 2013 Skywell Labs Inc.

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

#ifndef SKYWELL_PROTOCOL_SIGN_H_INCLUDED
#define SKYWELL_PROTOCOL_SIGN_H_INCLUDED

#include <protocol/AnyPublicKey.h>
#include <protocol/AnySecretKey.h>
#include <protocol/HashPrefix.h>
#include <protocol/STObject.h>
#include <utility>

namespace skywell {

/** Sign a STObject using any secret key.
    The signature is placed in sfSignature. If
    a signature already exists, it is overwritten.
*/
void
sign (STObject& st,
    HashPrefix const& prefix,
        AnySecretKey const& sk);

/** Verify the signature on a STObject.
    The signature must be contained in sfSignature.
*/
bool
verify (STObject const& st,
    HashPrefix const& prefix,
        AnyPublicKeySlice const& pk);

} // skywell

#endif
