/*
 * Copyright (C) 2012 Jolla Mobile <chris.adams@jollamobile.com>
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

#ifndef IDENTITYINTERFACE_H
#define IDENTITYINTERFACE_H

#include <QtCore/QObject>

//libsignon-qt
#include <SignOn/Identity>
#include <SignOn/identityinfo.h>

class IdentityInterfacePrivate;
class IdentityManagerInterface;

/*
 * IdentityInterface
 *
 * Provides an interface to an Identity in QML.
 * Identities are created and removed via the IdentityManager.
 * Identity is intended for use by accounts plugin authors,
 * not 3rd party application developers (they should use
 * ServiceAccountIdentityInterface instead).
 *
 * On retrieving the identity from the manager, the identity
 * will be in the RefreshInProgress state.  Once the information
 * associated with the identity has been retrieved from the
 * database, it will transition to the Initialized state.
 *
 * Please note that if an error occurs during first-time
 * initialization, the client must manually call refresh()
 * on the identity in order to initialize the identity.
 */

class IdentityInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int identifier READ identifier NOTIFY identifierChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(ErrorType error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    Q_PROPERTY(QString secret READ secret WRITE setSecret NOTIFY secretChanged)
    Q_PROPERTY(QString caption READ caption WRITE setCaption NOTIFY captionChanged)
    Q_PROPERTY(QStringList realms READ realms WRITE setRealms NOTIFY realmsChanged)
    Q_PROPERTY(QString owner READ owner WRITE setOwner NOTIFY ownerChanged)
    Q_PROPERTY(QStringList accessControlList READ accessControlList WRITE setAccessControlList NOTIFY accessControlListChanged)
    Q_PROPERTY(QStringList methods READ methods NOTIFY methodsChanged)

    Q_ENUMS(Status)
    Q_ENUMS(SyncMode)
    Q_ENUMS(ErrorType)

public:
    enum Status {
        Initialized = 0,
        Synced,
        SyncInProgress,
        RefreshInProgress,
        Modified,
        Error,
        Invalid
    };

    enum SyncMode {
        OverwriteSecret = 0,
        NoOverwriteSecret = 1
    };

    enum ErrorType {
        UnknownError                = SignOn::Identity::UnknownError,
        InternalServerError         = SignOn::Identity::InternalServerError,
        InternalCommunicationError  = SignOn::Identity::InternalCommunicationError,
        PermissionDeniedError       = SignOn::Identity::PermissionDeniedError,
        IdentityError               = SignOn::Identity::IdentityErr,
        MethodNotAvailableError     = SignOn::Identity::MethodNotAvailableError,
        NotFoundError               = SignOn::Identity::NotFoundError,
        StoreFailedError            = SignOn::Identity::StoreFailedError,
        RemoveFailedError           = SignOn::Identity::RemoveFailedError,
        SignOutFailedError          = SignOn::Identity::SignOutFailedError,
        CanceledError               = SignOn::Identity::CanceledError,
        CredentialsNotAvailableError= SignOn::Identity::CredentialsNotAvailableError,
        UserError                   = SignOn::Error::UserErr,
        NoError                     = UserError+1
    };

public:
    IdentityInterface(SignOn::Identity *ident, QObject *parent = 0);
    ~IdentityInterface();

    // property accessors and mutators
    int identifier() const;
    Status status() const;
    ErrorType error() const;
    QString errorMessage() const;

    QString userName() const;
    void setUserName(const QString &uname);
    QString secret() const;
    void setSecret(const QString &sec);
    QString caption() const;
    void setCaption(const QString &cap);
    QStringList realms() const;
    void setRealms(const QStringList &rlms);
    QString owner() const;
    void setOwner(const QString &own);
    QStringList accessControlList() const;
    void setAccessControlList(const QStringList& acl);
    QStringList methods() const;

    // invokable api
    Q_INVOKABLE void setMethodMechanisms(const QString &methodName, const QStringList &mechanisms);
    Q_INVOKABLE QStringList methodMechanisms(const QString &methodName);
    Q_INVOKABLE void removeMethod(const QString &methodName);

    Q_INVOKABLE void sync(SyncMode mode = OverwriteSecret);
    Q_INVOKABLE void refresh(); // sync but without writing any changes.

Q_SIGNALS:
    void identifierChanged();
    void statusChanged();
    void errorChanged();
    void errorMessageChanged();
    void userNameChanged();
    void secretChanged();
    void captionChanged();
    void realmsChanged();
    void ownerChanged();
    void accessControlListChanged();
    void methodsChanged();

private:
    void setIdentity(SignOn::Identity *ident);
    SignOn::Identity *identity() const;
    friend class IdentityManagerInterface;

private:
    IdentityInterfacePrivate *d;
    friend class IdentityInterfacePrivate;
};

#endif
