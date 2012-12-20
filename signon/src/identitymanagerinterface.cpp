/*
 * Copyright (C) 2012 Jolla Ltd. <chris.adams@jollamobile.com>
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

#include "identitymanagerinterface.h"

#include "identityinterface.h"

// TODO: do we need to hook up signals etc?  Do we need to track any data?
class IdentityManagerInterfacePrivate
{
};

/*!
    \qmltype IdentityManager
    \instantiates IdentityManagerInterface
    \inqmlmodule org.nemomobile.signon 1
    \brief An IdentityManager allows identities to be created, removed or referenced

    This type is intended for use by privileged applications and
    account provider plugins.  It allows identities to be created,
    removed or referenced.  Each Identity encapsulates some
    credentials which may be used to sign on to a service.

    See the documentation of the \l Identity type for more information
    on how to use an Identity.

    The IdentityManager provides invokable functions:
    \list
    \li createIdentity() returning a new, unsaved Identity
    \li removeIdentity(Identity)
    \li identity(identityId) returning an existing Identity
    \li addIdentityReference(Identity, string)
    \li removeIdentityReference(Identity, string)
    \endlist
*/

IdentityManagerInterface::IdentityManagerInterface(QObject *parent)
    : QObject(parent), d(0)
{
}

IdentityManagerInterface::~IdentityManagerInterface()
{
}

/*!
    \qmlmethod Identity* IdentityManager::createIdentity()
    Creates a new Identity and returns it.  The IdentityManager will
    own the returned Identity and delete it on destruction.
*/
IdentityInterface *IdentityManagerInterface::createIdentity()
{
    IdentityInterface *retn = new IdentityInterface(0, this); // owned by this manager.
    SignOn::Identity *newIdent = SignOn::Identity::newIdentity(SignOn::IdentityInfo(), retn); // owned by the returned interface.
    if (!newIdent) {
        delete retn;
        return 0; // couldn't create new identity.
    }
    retn->setIdentity(newIdent);
    retn->classBegin();
    retn->componentComplete();
    return retn;
}

/*!
    \qmlmethod void IdentityManager::removeIdentity(Identity *identity)
    Removes the given \a identity from the database.
    The operation may be either synchronous or asynchronous depending
    on the current lock state of the database.  When the remove operation
    completes, the identity will become invalid.
*/
void IdentityManagerInterface::removeIdentity(IdentityInterface *identity)
{
    if (identity && identity->identity())
        identity->identity()->remove();
}

/*!
    \qmlmethod Identity* IdentityManager::identity(const QString &identityIdentifier)
    Returns an existing identity from the database which is identified by the
    given \a identityIdentifier or null if no such identity exists.
    The IdentityManager will own the returned Identity instance and will
    delete it on destruction.
*/
IdentityInterface *IdentityManagerInterface::identity(const QString &identityIdentifier)
{
    bool ok = false;
    quint32 id = identityIdentifier.toUInt(&ok);
    if (!ok)
        return 0;
    
    IdentityInterface *retn = new IdentityInterface(0, this); // owned by this manager.
    SignOn::Identity *newIdent = SignOn::Identity::existingIdentity(id, retn); // owned by the returned interface.
    if (!newIdent) {
        delete retn;
        return 0; // no such identity.
    }

    retn->setIdentity(newIdent);
    retn->classBegin();
    retn->componentComplete();
    return retn;
}

/*!
    \qmlmethod void IdentityManager::addIdentityReference(Identity *identity, const QString &reference)
    Adds a reference \a reference to the given \a identity.
    I have no idea what this function is for or why anyone would call it.
*/
void IdentityManagerInterface::addIdentityReference(IdentityInterface *identity, const QString &reference)
{
    if (identity && identity->identity())
        identity->identity()->addReference(reference);
}

/*!
    \qmlmethod void IdentityManager::removeIdentityReference(Identity *identity, const QString &reference)
    Removes a reference \a reference from the given \a identity.
    I have no idea what this function is for or why anyone would call it.
*/
void IdentityManagerInterface::removeIdentityReference(IdentityInterface *identity, const QString &reference)
{
    if (identity && identity->identity())
        identity->identity()->removeReference(reference);
}

