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

#ifndef SKYWELL_RPC_ACCOUNT_OBJECTS_H_INCLUDED
#define SKYWELL_RPC_ACCOUNT_OBJECTS_H_INCLUDED

#include <ledger/Ledger.h>

namespace skywell {
namespace RPC {

/** Gathers all objects for an account in a ledger.
    @param ledger Ledger to search account objects.
    @param account Account to find objects for.
    @param type Gathers objects of this type. ltINVALID gathers all types.
    @param dirIndex Begin gathering account objects from this directory.
    @param entryIndex Begin gathering objects from this directory node.
    @param limit Maximum number of objects to find.
    @param jvResult A JSON result that holds the request objects.
*/
bool
getAccountObjects (Ledger const& ledger, Account const& account,
    LedgerEntryType const type, uint256 dirIndex, uint256 const& entryIndex,
    std::uint32_t const limit, Json::Value& jvResult);

} // RPC
} // skywell

#endif
