/* Copyright (C) 2012 John Brooks <john.brooks@dereferenced.net>
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

#include "messagesmanager.h"
#include <QDBusInterface>
#include <QDBusPendingCall>

MessagesManager::MessagesManager(QObject *parent)
    : QObject(parent)
{
    interface = new QDBusInterface("org.nemomobile.qmlmessages", "/", "org.nemomobile.qmlmessages",
            QDBusConnection::sessionBus(), this);
}

void MessagesManager::showGroupsWindow()
{
    interface->asyncCall("showGroupsWindow");
}

void MessagesManager::startConversation(const QString &localAccount, const QString &remoteUid)
{
    // 0 is ChatTypeP2P; this API doesn't support multiple contacts currently.
    interface->asyncCall("startConversation", localAccount, remoteUid, (unsigned)0);
}

void MessagesManager::startSMS(const QString &phoneNumber)
{
    interface->asyncCall("startConversation", "/org/freedesktop/Telepathy/Account/ring/tel/account0",
            phoneNumber, (unsigned)0);
}

