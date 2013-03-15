/*
 * Copyright (C) 2013 Jolla Ltd. <chris.adams@jollamobile.com>
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

#ifndef FACEBOOKUSERINTERFACE_H
#define FACEBOOKUSERINTERFACE_H

#include "identifiablecontentiteminterface.h"

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QUrl>

class FacebookObjectReferenceInterface;
class FacebookPictureInterface;

/*
 * NOTE: if you construct one of these in C++ directly,
 * you MUST call classBegin() and componentCompleted()
 * directly after construction.
 */

/*
 * Doesn't expose (via API, although it does via data())
 * the following fields from the Facebook API:
 *     age_range
 *     cover
 *     currency
 *     devices
 *     education
 *     languages
 *     payment_pricepoints
 *     security_settings
 *     video_upload_limits
 *     work
 */

class FacebookUserInterfacePrivate;
class FacebookUserInterface : public IdentifiableContentItemInterface
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString firstName READ firstName NOTIFY firstNameChanged)
    Q_PROPERTY(QString middleName READ middleName NOTIFY middleNameChanged)
    Q_PROPERTY(QString lastName READ lastName NOTIFY lastNameChanged)
    Q_PROPERTY(QString gender READ gender NOTIFY genderChanged)
    Q_PROPERTY(QString locale READ locale NOTIFY localeChanged)
    Q_PROPERTY(QUrl link READ link NOTIFY linkChanged)
    Q_PROPERTY(QString userName READ userName NOTIFY userNameChanged)
    Q_PROPERTY(QString thirdPartyIdentifier READ thirdPartyIdentifier NOTIFY thirdPartyIdentifierChanged)
    Q_PROPERTY(bool installed READ installed NOTIFY installedChanged)
    Q_PROPERTY(qreal timezoneOffset READ timezoneOffset NOTIFY timezoneOffsetChanged)
    Q_PROPERTY(QString updatedTime READ updatedTime NOTIFY updatedTimeChanged)
    Q_PROPERTY(bool verified READ verified NOTIFY verifiedChanged)
    Q_PROPERTY(QString bio READ bio NOTIFY bioChanged)
    Q_PROPERTY(QString birthday READ birthday NOTIFY birthdayChanged)
    Q_PROPERTY(QString email READ email NOTIFY emailChanged)
    Q_PROPERTY(FacebookObjectReferenceInterface *hometown READ hometown NOTIFY hometownChanged)
    Q_PROPERTY(QStringList interestedIn READ interestedIn NOTIFY interestedInChanged)
    Q_PROPERTY(FacebookObjectReferenceInterface *location READ location NOTIFY locationChanged)
    Q_PROPERTY(QString political READ political NOTIFY politicalChanged)
    Q_PROPERTY(FacebookPictureInterface *picture READ picture NOTIFY pictureChanged)
    Q_PROPERTY(QString quotes READ quotes NOTIFY quotesChanged)
    Q_PROPERTY(RelationshipStatus relationshipStatus READ relationshipStatus NOTIFY relationshipStatusChanged)
    Q_PROPERTY(QString religion READ religion NOTIFY religionChanged)
    Q_PROPERTY(FacebookObjectReferenceInterface *significantOther READ significantOther NOTIFY significantOtherChanged)
    Q_PROPERTY(QUrl website READ website NOTIFY websiteChanged)

    Q_ENUMS(RelationshipStatus)

public:
    enum RelationshipStatus {
        Unknown = 0,
        Single,
        InARelationship,
        Engaged,
        Married,
        ItsComplicated,
        InAnOpenRelationship,
        Widowed,
        Separated,
        Divorced,
        InACivilUnion,
        InADomesticPartnership
    };


public:
    FacebookUserInterface(QObject *parent = 0);
    ~FacebookUserInterface();

    // overrides.
    int type() const;
    Q_INVOKABLE bool remove();
    Q_INVOKABLE bool reload(const QStringList &whichFields = QStringList());
    void emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData);

    // invokable API
    Q_INVOKABLE bool uploadPhoto(const QUrl &source, const QString &message = QString());
    Q_INVOKABLE bool removePhoto(const QString &photoIdentifier);
    Q_INVOKABLE bool uploadAlbum(const QString &name, const QString &message = QString(), const QVariantMap &privacy = QVariantMap());
    Q_INVOKABLE bool removeAlbum(const QString &albumIdentifier);

public:
    // property accessors.
    QString name() const;
    QString firstName() const;
    QString middleName() const;
    QString lastName() const;
    QString gender() const;
    QString locale() const;
    QUrl link() const;
    QString userName() const;
    QString thirdPartyIdentifier() const;
    bool installed() const;
    qreal timezoneOffset() const;
    QString updatedTime() const;
    bool verified() const;
    QString bio() const;
    QString birthday() const;
    QString email() const;
    FacebookObjectReferenceInterface *hometown() const;
    QStringList interestedIn() const;
    FacebookObjectReferenceInterface *location() const;
    QString political() const;
    FacebookPictureInterface *picture() const;
    QString quotes() const;
    RelationshipStatus relationshipStatus() const;
    QString religion() const;
    FacebookObjectReferenceInterface *significantOther() const;
    QUrl website() const;

Q_SIGNALS:
    void nameChanged();
    void firstNameChanged();
    void middleNameChanged();
    void lastNameChanged();
    void genderChanged();
    void localeChanged();
    void linkChanged();
    void userNameChanged();
    void thirdPartyIdentifierChanged();
    void installedChanged();
    void timezoneOffsetChanged();
    void updatedTimeChanged();
    void verifiedChanged();
    void bioChanged();
    void birthdayChanged();
    void emailChanged();
    void hometownChanged();
    void interestedInChanged();
    void locationChanged();
    void politicalChanged();
    void pictureChanged();
    void quotesChanged();
    void relationshipStatusChanged();
    void religionChanged();
    void significantOtherChanged();
    void websiteChanged();

private:
    Q_DECLARE_PRIVATE(FacebookUserInterface)
    Q_PRIVATE_SLOT(d_func(), void finishedHandler())
};

#endif // FACEBOOKUSERINTERFACE_H
