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

#include <QContactId>

#include <QtDebug>

QTM_USE_NAMESPACE

// Helper utility to synchronize a cached list with some reference list with correct
// QAbstractItemModel signals and filtering.

// If the reference list is populated incrementally this can be called multiple times with the
// same variables c and r to progressively synchronize the lists.  If after the final call either
// the c or r index is not equal to the length of the cache or reference lists respectively then
// the remaining items can be synchronized manually by removing the remaining items from the
// cache list before (filtering and) appending the remaining reference items.

template <typename Agent, typename ReferenceList>
class SynchronizeList
{
public:
    SynchronizeList(
            Agent *agent,
            const QVector<QContactLocalId> &cache,
            int &c,
            const ReferenceList &reference,
            int &r)
        : agent(agent), cache(cache), c(c), reference(reference), r(r)
    {
        while (c < cache.count() && r < reference.count()) {
            if (cache.at(c) == reference.at(r)) {
                ++c;
                ++r;
                continue;
            }

            bool match = false;

            // Iterate through both the reference and cache lists in parallel looking for first
            // point of commonality, when that is found resolve the differences and continue
            // looking.
            int count = 1;
            for (; !match && c + count < cache.count() && r + count < reference.count(); ++count) {
                const QContactLocalId cacheId = cache.at(c + count);
                const QContactLocalId referenceId = reference.at(r + count);

                for (int i = 0; i <= count; ++i) {
                    if (cacheMatch(i, count, referenceId) || referenceMatch(i, count, cacheId)) {
                        match = true;
                        break;
                    }
                }
            }

            // Continue scanning the reference list if the cache has been exhausted.
            for (int re = r + count; !match && re < reference.count(); ++re) {
                const QContactLocalId referenceId = reference.at(re);
                for (int i = 0; i < count; ++i) {
                    if (cacheMatch(i, re - r, referenceId)) {
                        match = true;
                        break;
                    }
                }
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
                c += agent->removeRange(c, i);
            c += agent->insertRange(c, count, reference, r) + 1;
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
        if (reference.at(r + i) == cacheId) {
            c += agent->removeRange(c, count);
            if (i > 0)
                c += agent->insertRange(c, i, reference, r);
            c += 1;
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
};

template <typename Agent, typename ReferenceList>
void synchronizeList(
        Agent *agent,
        const QVector<QContactLocalId> &cache,
        int &c,
        const ReferenceList &reference,
        int &r)
{
    SynchronizeList<Agent, ReferenceList>(agent, cache, c, reference, r);
}

template <typename Agent, typename ReferenceList>
class SynchronizeFilteredList
{
public:
    SynchronizeFilteredList(
            Agent *agent,
            const QVector<QContactLocalId> &cache,
            int &c,
            const ReferenceList &reference,
            int &r)
        : cache(cache)
        , agent(agent)
        , previousIndex(0)
        , removeCount(0)
    {
        synchronizeList(this, cache, c, reference, r);

        if (filteredIds.count() > 0) {
            c += filteredIds.count();
            agent->insertRange(previousIndex, filteredIds.count(), filteredIds, 0);
        } else if (removeCount > 0) {
            c -= removeCount;
            agent->removeRange(previousIndex, removeCount);
        }

        for (; previousIndex < c; ++previousIndex) {
            int filterCount = 0;
            for (int i; (i = previousIndex + filterCount) < c; ++filterCount) {
                if (agent->filterId(cache.at(i)))
                    break;
            }
            if (filterCount > 0) {
                agent->removeRange(previousIndex, filterCount);
                c -= filterCount;
            }
        }
    }

    int insertRange(int index, int count, const QVector<QContactLocalId> &source, int sourceIndex)
    {
        int adjustedIndex = index;

        if (removeCount > 0) {
            adjustedIndex -= removeCount;
            agent->removeRange(previousIndex, removeCount);
            removeCount = 0;
        } else if (filteredIds.count() > 0 && index > previousIndex) {
            agent->insertRange(previousIndex, filteredIds.count(), filteredIds, 0);
            adjustedIndex += filteredIds.count();
            previousIndex += filteredIds.count();
            filteredIds.resize(0);
        }

        if (filteredIds.isEmpty()) {
            for (; previousIndex < adjustedIndex;) {
                int filterCount = 0;
                for (int i; (i = previousIndex + filterCount) < adjustedIndex; ++filterCount) {
                    if (agent->filterId(cache.at(i)))
                        break;
                }
                if (filterCount > 0) {
                    agent->removeRange(previousIndex, filterCount);
                    adjustedIndex -= filterCount;
                } else {
                    ++previousIndex;
                }
            }
        }

        for (int i = 0; i < count; ++i) {
            const QContactLocalId contactId = source.at(sourceIndex + i);
            if (agent->filterId(contactId))
                filteredIds.append(contactId);
        }

        return adjustedIndex - index;
    }

    int removeRange(int index, int count)
    {
        int adjustedIndex = index;
        if (filteredIds.count() > 0) {
            adjustedIndex += filteredIds.count();
            agent->insertRange(previousIndex, filteredIds.count(), filteredIds, 0);
            filteredIds.resize(0);
        } else if (removeCount > 0 && adjustedIndex > previousIndex + removeCount) {
            adjustedIndex -= removeCount;
            agent->removeRange(previousIndex, removeCount);
            removeCount = 0;
        }

        if (removeCount == 0) {
            for (; previousIndex < adjustedIndex;) {
                int filterCount = 0;
                for (int i; (i = previousIndex + filterCount) < adjustedIndex; ++filterCount) {
                    if (agent->filterId(cache.at(i)))
                        break;
                }
                if (previousIndex + filterCount == adjustedIndex) {
                    removeCount += filterCount;
                    break;
                } else if (filterCount > 0) {
                    agent->removeRange(previousIndex, filterCount);
                    adjustedIndex -= filterCount;
                } else {
                    ++previousIndex;
                }
            }
        }

        removeCount += count;

        return adjustedIndex - index + count;
    }

    QVector<QContactLocalId> filteredIds;
    const QVector<QContactLocalId> &cache;
    Agent *agent;
    int previousIndex;
    int removeCount;
};

template <typename Agent, typename ReferenceList>
void synchronizeFilteredList(
        Agent *agent,
        const QVector<QContactLocalId> &cache,
        int &c,
        const ReferenceList &reference,
        int &r)
{
    SynchronizeFilteredList<Agent, ReferenceList>(agent, cache, c, reference, r);
}

#endif
