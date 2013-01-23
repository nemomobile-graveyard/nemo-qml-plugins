/*
 * Copyright (C) 2013 Jolla Mobile <andrew.den.exter@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#ifndef SYNCHRONIZELISTS_P_H
#define SYNCHRONIZELISTS_P_H

//#include <QContactId>

//QTM_USE_NAMESPACE

typedef quint32 QContactLocalId;

// Helper utility to synchronize a cached list with some reference list with correct
// QAbstractItemModel signals and filtering.

// If the reference list is populated incrementally this can be called multiple times with the
// same variables c and r to progressively synchronize the lists.  If after the final call either
// the c or r index is not equal to the length of the cache or reference lists respectively then
// the remaining items can be synchronized manually by removing the remaining items from the
// cache list before (filtering and) appending the remaining reference items.

template <typename Agent, typename ReferenceList>
class SynchronizeLists
{
public:
    SynchronizeLists(
            Agent *agent,
            const QVector<QContactLocalId> &cache,
            int &c,
            const ReferenceList &reference,
            int &r)
        : agent(agent), cache(cache), c(c), reference(reference), r(r)
    {
        while (c < cache.count() && r < reference.count()) {
            if (cache.at(c) == reference.at(r) && agent->filterId(reference.at(r))) {
                ++c;
                ++r;
                continue;
            }

            bool match = false;

            if (agent->filterId(reference.at(r)))
                insertIds.append(reference.at(r));

            // Iterate through both the reference and cache lists in parallel looking for first
            // point of commonality, when that is found resolve the differences and continue
            // looking.
            int count = 1;
            for (; !match && c + count < cache.count() && r + count < reference.count(); ++count) {
                const QContactLocalId cacheId = cache.at(c + count);
                const QContactLocalId referenceId = reference.at(r + count);

                const bool filterRe = agent->filterId(referenceId);

                for (int i = 0; i < count; ++i) {
                    if ((filterRe && cacheMatch(i, count, referenceId))
                            || referenceMatch(i, count, cacheId)) {
                        match = true;
                        break;
                    }
                }
                if (filterRe)
                    insertIds.append(referenceId);
            }

            // Continue scanning the reference list if the cache has been exhausted.
            for (int re = r + count; !match && re < reference.count(); ++re) {
                const QContactLocalId referenceId = reference.at(re);
                if (!agent->filterId(referenceId))
                    continue;
                for (int i = 0; i < count; ++i) {
                    if (cacheMatch(i, re - r, referenceId)) {
                        match = true;
                        break;
                    }
                }
                insertIds.append(referenceId);
            }

            // Continue scanning the cache if the reference list has been exhausted.
            for (int ce = c + count; !match && ce < cache.count(); ++ce) {
                const QContactLocalId cacheId = cache.at(ce);
                for (int i = 0; i < count; ++i) {
                    if (referenceMatch(i, ce - c, cacheId)) {
                        match = true;
                        break;
                    }
                }
            }

            if (!match)
                return;
            insertIds.clear();
        }
    }

private:
    // Tests if the cached contact id at i matches a referenceId.
    // If there is a match removes all items traversed in the cache since the previous match
    // and inserts any items in the reference set found to to not be in the cache.
    bool cacheMatch(int i, int count, QContactLocalId referenceId)
    {
        if (cache.at(c + i) == referenceId) {
            if (i > 0)
                agent->removeRange(c, i);
            if (insertIds.count() > 0)
                agent->insertRange(c, insertIds.count(), insertIds, 0);
            c += insertIds.count() + 1;
            r += count + 1;
            return true;
        } else {
            return false;
        }
    }

    // Tests if the reference contact id at i matches a cacheId.
    // If there is a match inserts all items traversed in the reference set since the
    // previous match and removes any items from the cache that were not found in the
    // reference list.
    bool referenceMatch(int i, int count, QContactLocalId cacheId)
    {
        const QContactLocalId referenceId = reference.at(r + i);
        if (referenceId == cacheId && agent->filterId(referenceId)) {
            // The insertIds list contains all the items matching the filter between r and
            // r + count but we're only interested in those between r and r + i, so the
            // additional items have to be removed.
            for (int j = r + count - 1; j >= r + i && !insertIds.isEmpty(); --j) {
                if (reference.at(j) == insertIds.last())
                    insertIds.pop_back();
            }

            if (count > 0)
                agent->removeRange(c, count);
            if (insertIds.count() > 0)
                agent->insertRange(c, insertIds.count(), insertIds, 0);

            c += insertIds.count() + 1;
            r += i + 1;
            return true;
        } else {
            return false;
        }
    }

    Agent * const agent;
    const QVector<QContactLocalId> &cache;
    int &c;
    const ReferenceList &reference;
    int &r;
    ReferenceList insertIds;
};

template <typename Agent, typename ReferenceList>
void synchronizeLists(
        Agent *agent,
        const QVector<QContactLocalId> &cache,
        int &c,
        const ReferenceList &reference,
        int &r)
{
    SynchronizeLists<Agent, ReferenceList>(agent, cache, c, reference, r);
}

#endif
