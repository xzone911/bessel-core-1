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

#ifndef SKYWELL_APP_LEDGER_BOOKLISTENERS_H_INCLUDED
#define SKYWELL_APP_LEDGER_BOOKLISTENERS_H_INCLUDED

#include <services/net/InfoSub.h>
#include <memory>

namespace skywell {

/** Listen to public/subscribe messages from a book. */
class BookListeners
{
public:
    typedef std::shared_ptr<BookListeners> pointer;

    BookListeners () {}

    void addSubscriber (InfoSub::ref sub);
    void removeSubscriber (std::uint64_t sub);
    void publish (Json::Value const& jvObj);

private:
    typedef SkywellRecursiveMutex LockType;
    typedef std::lock_guard <LockType> ScopedLockType;
    LockType mLock;

    hash_map<std::uint64_t, InfoSub::wptr> mListeners;
};

} // skywell

#endif
