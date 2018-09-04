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

#include <protocol/AnySecretKey.h>
#include <protocol/SkywellAddress.h>
#include <protocol/Serializer.h>
#include <crypto/RandomNumbers.h>
#include <crypto/ed25519-donna/ed25519.h>
#include <algorithm>
#include <cassert>
#include <cstring>

namespace skywell {

AnySecretKey::~AnySecretKey()
{
    // secure erase
    std::fill(p_.data(), p_.data() + p_.size(), 0);
}

AnySecretKey::AnySecretKey (AnySecretKey&& other)
    : p_ (std::move(other.p_))
    , type_ (other.type_)
{
    other.type_ = KeyType::unknown;
}

AnySecretKey&
AnySecretKey::operator= (AnySecretKey&& other)
{
    p_ = std::move(other.p_);
    type_ = other.type_;
    other.type_ = KeyType::unknown;
    return *this;
}

AnySecretKey::AnySecretKey (KeyType type,
        void const* data, std::size_t size)
    : p_ (data, size)
    , type_ (type)
{
    if (type_ == KeyType::unknown)
        throw std::runtime_error(
            "AnySecretKey: unknown type");
    if (type_ == KeyType::ed25519 &&
            size != 32)
        throw std::runtime_error(
            "AnySecretKey: wrong ed25519 size");
    if (type_ == KeyType::secp256k1 &&
            size != 32)
        throw std::runtime_error(
            "AnySecretKey: wrong secp256k1 size");
}

AnyPublicKey
AnySecretKey::publicKey() const
{
    switch (type())
    {
    case KeyType::ed25519:
    {
        unsigned char buf[33];
        buf[0] = 0xED;
        ed25519_publickey(p_.data() + 1, &buf[1]);
        return AnyPublicKey(buf, sizeof(buf));
    }
    default:
        throw std::runtime_error(
            "AnySecretKey: unknown type");
    };
}

Buffer
AnySecretKey::sign (
    void const* msg, std::size_t msg_len) const
{
    switch(type_)
    {
        case KeyType::ed25519:
        {
            auto const sk = p_.data() + 1;
            ed25519_public_key pk;
            ed25519_publickey(sk, pk);
            Buffer b(64);
            ed25519_sign(reinterpret_cast<
                unsigned char const*>(msg), msg_len,
                    sk, pk, b.data());
            return b;
        }
        default:
            break;
    }
    throw std::runtime_error(
        "AnySecretKey: unknown type");
}

AnySecretKey
AnySecretKey::make_ed25519()
{
    std::uint8_t buf[32];
    random_fill(&buf[0], sizeof(buf));
    AnySecretKey ask(KeyType::ed25519,
        buf, sizeof(buf));
    // secure erase
    std::fill(buf, buf + sizeof(buf), 0);
    return ask;
}

std::pair<AnySecretKey, AnyPublicKey>
AnySecretKey::make_secp256k1_pair()
{
    //  What a pile
    SkywellAddress s;
    s.setSeedRandom();
    SkywellAddress const g =
        SkywellAddress::createGeneratorPublic(s);
    SkywellAddress sk;
    sk.setAccountPrivate (g, s, 0);
    SkywellAddress pk;
    pk.setAccountPublic (g, 0);
    return std::pair<AnySecretKey, AnyPublicKey>(
        std::piecewise_construct, std::make_tuple(
            KeyType::secp256k1, sk.data(), sk.size()),
                std::make_tuple(pk.data(), pk.size()));
}

} // skywell
