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

#include <BeastConfig.h>
#include <transaction/book/Types.h>
#include <protocol/STAmount.h>
#include <protocol/Indexes.h>

namespace skywell {

STAmount creditLimit (
    LedgerEntrySet& ledger,
    Account const& account,
    Account const& issuer,
    Currency const& currency)
{
    STAmount result ({currency, account});

    auto sleSkywellState = ledger.entryCache (ltSKYWELL_STATE,
        getSkywellStateIndex (account, issuer, currency));

    if (sleSkywellState)
    {
        result = sleSkywellState->getFieldAmount (
            account < issuer ? sfLowLimit : sfHighLimit);
        result.setIssuer (account);
    }

    assert (result.getIssuer () == account);
    assert (result.getCurrency () == currency);
    return result;
}

STAmount creditBalance (
    LedgerEntrySet& ledger,
    Account const& account,
    Account const& issuer,
    Currency const& currency)
{
    STAmount result ({currency, account});

    auto sleSkywellState = ledger.entryCache (ltSKYWELL_STATE,
        getSkywellStateIndex (account, issuer, currency));

    if (sleSkywellState)
    {
        result = sleSkywellState->getFieldAmount (sfBalance);
        if (account < issuer)
            result.negate ();
        result.setIssuer (account);
    }

    assert (result.getIssuer () == account);
    assert (result.getCurrency () == currency);
    return result;
}

} // skywell
