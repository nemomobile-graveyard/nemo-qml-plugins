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

#include "facebookuserinterface.h"
#include "facebookuserinterface_p.h"

#include "facebookontology_p.h"
#include "facebookpictureinterface.h"
#include "facebookobjectreferenceinterface.h"

#include "identifiablecontentiteminterface_p.h"
#include "contentiteminterface_p.h"

#include <QtDebug>

FacebookUserInterfacePrivate::FacebookUserInterfacePrivate(FacebookUserInterface *parent, IdentifiableContentItemInterfacePrivate *parentData)
    : QObject(parent)
    , q(parent)
    , dd(parentData)
    , hometown(new FacebookObjectReferenceInterface(this))
    , location(new FacebookObjectReferenceInterface(this))
    , significantOther(new FacebookObjectReferenceInterface(this))
    , picture(new FacebookPictureInterface(this))
    , action(FacebookInterfacePrivate::NoAction)
{
}

FacebookUserInterfacePrivate::~FacebookUserInterfacePrivate()
{
    dd->deleteReply();
}

/*! \internal */
void FacebookUserInterfacePrivate::finishedHandler()
{
    if (!dd->reply()) {
        // if an error occurred, it might have been deleted by the error handler.
        qWarning() << Q_FUNC_INFO << "network request finished but no reply";
        return;
    }

    QByteArray replyData = dd->reply()->readAll();
    dd->deleteReply();
    bool ok = false;
    QVariantMap responseData = ContentItemInterface::parseReplyData(replyData, &ok);
    if (!ok)
        responseData.insert("response", replyData);

    switch (action) {
        case FacebookInterfacePrivate::DeletePhotoAction: // flow down.
        case FacebookInterfacePrivate::DeleteAlbumAction: {
            if (replyData == QString(QLatin1String("true"))) {
                dd->status = SocialNetworkInterface::Idle;
                emit q->statusChanged();
                emit q->responseReceived(responseData);
            } else {
                dd->error = SocialNetworkInterface::RequestError;
                dd->errorMessage = QLatin1String("User: request failed");
                dd->status = SocialNetworkInterface::Error;
                emit q->statusChanged();
                emit q->errorChanged();
                emit q->errorMessageChanged();
                emit q->responseReceived(responseData);
            }
        }
        break;

        case FacebookInterfacePrivate::UploadPhotoAction: // flow down.
        case FacebookInterfacePrivate::UploadAlbumAction: {
            if (!ok || responseData.value("id").toString().isEmpty()) {
                // failed.
                dd->error = SocialNetworkInterface::RequestError;
                dd->errorMessage = action == FacebookInterfacePrivate::UploadAlbumAction
                    ? QLatin1String("Album: add album request failed")
                    : QLatin1String("Album: add photo request failed");
                dd->status = SocialNetworkInterface::Error;
                emit q->statusChanged();
                emit q->errorChanged();
                emit q->errorMessageChanged();
                emit q->responseReceived(responseData);
            } else {
                // succeeded.
                dd->status = SocialNetworkInterface::Idle;
                emit q->statusChanged();
                emit q->responseReceived(responseData);
            }
        }
        break;

        default: {
            dd->error = SocialNetworkInterface::OtherError;
            dd->errorMessage = QLatin1String("Request finished but no action currently in progress");
            dd->status = SocialNetworkInterface::Error;
            emit q->statusChanged();
            emit q->errorChanged();
            emit q->errorMessageChanged();
            emit q->responseReceived(responseData);
        }
        break;
    }
}

//-------------------------------------------

FacebookUserInterface::FacebookUserInterface(QObject *parent)
    : IdentifiableContentItemInterface(parent), f(new FacebookUserInterfacePrivate(this, dd))
{
}

FacebookUserInterface::~FacebookUserInterface()
{
}

/*! \reimp */
int FacebookUserInterface::type() const
{
    return FacebookInterface::User;
}

/*! \reimp */
bool FacebookUserInterface::remove()
{
    return IdentifiableContentItemInterface::remove();
}

/*! \reimp */
bool FacebookUserInterface::reload(const QStringList &whichFields)
{
    return IdentifiableContentItemInterface::reload(whichFields);
}

/*! \reimp */
void FacebookUserInterface::emitPropertyChangeSignals(const QVariantMap &oldData, const QVariantMap &newData)
{
    QString nameStr = newData.value(FACEBOOK_ONTOLOGY_USER_NAME).toString();
    QString fnStr = newData.value(FACEBOOK_ONTOLOGY_USER_FIRSTNAME).toString();
    QString mnStr = newData.value(FACEBOOK_ONTOLOGY_USER_MIDDLENAME).toString();
    QString lnStr = newData.value(FACEBOOK_ONTOLOGY_USER_LASTNAME).toString();
    QString gnStr = newData.value(FACEBOOK_ONTOLOGY_USER_GENDER).toString();
    QString locStr = newData.value(FACEBOOK_ONTOLOGY_USER_LOCALE).toString();
    QString linkStr = newData.value(FACEBOOK_ONTOLOGY_USER_LINK).toString();
    QString unStr = newData.value(FACEBOOK_ONTOLOGY_USER_USERNAME).toString();
    QString tpiStr = newData.value(FACEBOOK_ONTOLOGY_USER_THIRDPARTYIDENTIFIER).toString();
    QString insStr = newData.value(FACEBOOK_ONTOLOGY_USER_INSTALLED).toString();
    QString tzoStr = newData.value(FACEBOOK_ONTOLOGY_USER_TIMEZONEOFFSET).toString();
    QString verStr = newData.value(FACEBOOK_ONTOLOGY_USER_VERIFIED).toString();
    QString udtStr = newData.value(FACEBOOK_ONTOLOGY_USER_UPDATEDTIME).toString();
    QString bioStr = newData.value(FACEBOOK_ONTOLOGY_USER_BIO).toString();
    QString bdayStr = newData.value(FACEBOOK_ONTOLOGY_USER_BIRTHDAY).toString();
    QString emailStr = newData.value(FACEBOOK_ONTOLOGY_USER_EMAIL).toString();
    QStringList iinStr = newData.value(FACEBOOK_ONTOLOGY_USER_INTERESTEDIN).toStringList();
    QString polStr = newData.value(FACEBOOK_ONTOLOGY_USER_POLITICAL).toString();
    QString quoStr = newData.value(FACEBOOK_ONTOLOGY_USER_QUOTES).toString();
    QString rssStr = newData.value(FACEBOOK_ONTOLOGY_USER_RELATIONSHIPSTATUS).toString();
    QString relStr = newData.value(FACEBOOK_ONTOLOGY_USER_RELIGION).toString();
    QString webStr = newData.value(FACEBOOK_ONTOLOGY_USER_WEBSITE).toString();
    QVariantMap htMap = newData.value(FACEBOOK_ONTOLOGY_USER_HOMETOWN).toMap();
    QVariantMap locMap = newData.value(FACEBOOK_ONTOLOGY_USER_LOCATION).toMap();
    QVariantMap picMap = newData.value(FACEBOOK_ONTOLOGY_USER_PICTURE).toMap();
    QVariantMap sigMap = newData.value(FACEBOOK_ONTOLOGY_USER_SIGNIFICANTOTHER).toMap();

    QString oldNameStr = oldData.value(FACEBOOK_ONTOLOGY_USER_NAME).toString();
    QString oldFnStr = oldData.value(FACEBOOK_ONTOLOGY_USER_FIRSTNAME).toString();
    QString oldMnStr = oldData.value(FACEBOOK_ONTOLOGY_USER_MIDDLENAME).toString();
    QString oldLnStr = oldData.value(FACEBOOK_ONTOLOGY_USER_LASTNAME).toString();
    QString oldGnStr = oldData.value(FACEBOOK_ONTOLOGY_USER_GENDER).toString();
    QString oldLocStr = oldData.value(FACEBOOK_ONTOLOGY_USER_LOCALE).toString();
    QString oldLinkStr = oldData.value(FACEBOOK_ONTOLOGY_USER_LINK).toString();
    QString oldUnStr = oldData.value(FACEBOOK_ONTOLOGY_USER_USERNAME).toString();
    QString oldTpiStr = oldData.value(FACEBOOK_ONTOLOGY_USER_THIRDPARTYIDENTIFIER).toString();
    QString oldInsStr = oldData.value(FACEBOOK_ONTOLOGY_USER_INSTALLED).toString();
    QString oldTzoStr = newData.value(FACEBOOK_ONTOLOGY_USER_TIMEZONEOFFSET).toString();
    QString oldVerStr = newData.value(FACEBOOK_ONTOLOGY_USER_VERIFIED).toString();
    QString oldUdtStr = newData.value(FACEBOOK_ONTOLOGY_USER_UPDATEDTIME).toString();
    QString oldBioStr = oldData.value(FACEBOOK_ONTOLOGY_USER_BIO).toString();
    QString oldBdayStr = oldData.value(FACEBOOK_ONTOLOGY_USER_BIRTHDAY).toString();
    QString oldEmailStr = oldData.value(FACEBOOK_ONTOLOGY_USER_EMAIL).toString();
    QStringList oldIinStr = oldData.value(FACEBOOK_ONTOLOGY_USER_INTERESTEDIN).toStringList();
    QString oldPolStr = oldData.value(FACEBOOK_ONTOLOGY_USER_POLITICAL).toString();
    QString oldQuoStr = oldData.value(FACEBOOK_ONTOLOGY_USER_QUOTES).toString();
    QString oldRssStr = oldData.value(FACEBOOK_ONTOLOGY_USER_RELATIONSHIPSTATUS).toString();
    QString oldRelStr = oldData.value(FACEBOOK_ONTOLOGY_USER_RELIGION).toString();
    QString oldWebStr = oldData.value(FACEBOOK_ONTOLOGY_USER_WEBSITE).toString();
    QVariantMap oldHtMap = oldData.value(FACEBOOK_ONTOLOGY_USER_HOMETOWN).toMap();
    QVariantMap oldLocMap = oldData.value(FACEBOOK_ONTOLOGY_USER_LOCATION).toMap();
    QVariantMap oldPicMap = oldData.value(FACEBOOK_ONTOLOGY_USER_PICTURE).toMap();
    QVariantMap oldSigMap = oldData.value(FACEBOOK_ONTOLOGY_USER_SIGNIFICANTOTHER).toMap();

    // standard properties
    if (nameStr != oldNameStr)
        emit nameChanged();
    if (fnStr != oldFnStr)
        emit firstNameChanged();
    if (mnStr != oldMnStr)
        emit middleNameChanged();
    if (lnStr != oldLnStr)
        emit lastNameChanged();
    if (gnStr != oldGnStr)
        emit genderChanged();
    if (locStr != oldLocStr)
        emit locationChanged();
    if (linkStr != oldLinkStr)
        emit linkChanged();
    if (unStr != oldUnStr)
        emit userNameChanged();
    if (tpiStr != oldTpiStr)
        emit thirdPartyIdentifierChanged();
    if (insStr != oldInsStr)
        emit installedChanged();
    if (tzoStr != oldTzoStr)
        emit timezoneOffsetChanged();
    if (verStr != oldVerStr)
        emit verifiedChanged();
    if (udtStr != oldUdtStr)
        emit updatedTimeChanged();
    if (bioStr != oldBioStr)
        emit bioChanged();
    if (bdayStr != oldBdayStr)
        emit birthdayChanged();
    if (emailStr != oldEmailStr)
        emit emailChanged();
    if (iinStr != oldIinStr)
        emit interestedInChanged();
    if (polStr != oldPolStr)
        emit politicalChanged();
    if (quoStr != oldQuoStr)
        emit quotesChanged();
    if (rssStr != oldRssStr)
        emit relationshipStatusChanged();
    if (relStr != oldRelStr)
        emit religionChanged();
    if (webStr != oldWebStr)
        emit websiteChanged();

    // variantmap / object reference properties
    if (htMap != oldHtMap) {
        QVariantMap newHtData;
        newHtData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::Location);
        newHtData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, newHtData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER));
        newHtData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, newHtData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME));
        qobject_cast<FacebookInterface*>(socialNetwork())->setFacebookContentItemData(f->hometown, newHtData);
        emit hometownChanged();
    }

    if (locMap != oldLocMap) {
        QVariantMap newLocData;
        newLocData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::Location);
        newLocData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, newLocData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER));
        newLocData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, newLocData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME));
        qobject_cast<FacebookInterface*>(socialNetwork())->setFacebookContentItemData(f->location, newLocData);
        emit locationChanged();
    }

    if (picMap != oldPicMap) {
        qobject_cast<FacebookInterface*>(socialNetwork())->setFacebookContentItemData(f->picture, picMap);
        emit pictureChanged();
    }

    if (sigMap != oldSigMap) {
        QVariantMap newSigData;
        newSigData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTTYPE, FacebookInterface::User);
        newSigData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER, newSigData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTIDENTIFIER));
        newSigData.insert(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME, newSigData.value(FACEBOOK_ONTOLOGY_OBJECTREFERENCE_OBJECTNAME));
        qobject_cast<FacebookInterface*>(socialNetwork())->setFacebookContentItemData(f->significantOther, newSigData);
        emit significantOtherChanged();
    }

    // call the super class implementation
    IdentifiableContentItemInterface::emitPropertyChangeSignals(oldData, newData);
}


/*!
    \qmlmethod bool FacebookUser::uploadPhoto(const QUrl &source, const QString &message)
    Initiates a "post photo" operation on the user.  The photo will
    be loaded from the local filesystem and uploaded to Facebook with
    its caption set to the given \a message.  It will be uploaded to
    the default album of the user.

    If the network request was started successfully, the function
    will return true and the status of the user will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.

    Once the network request completes, the \c responseReceived()
    signal will be emitted.  The \c data parameter of the signal
    will contain the \c id of the newly uploaded photo.
*/
bool FacebookUserInterface::uploadPhoto(const QUrl &source, const QString &message)
{
    // XXX TODO: privacy parameter?

    QVariantMap extraData; // image upload is handled specially by the facebook adapter.
    extraData.insert("isImageUpload", true);

    QVariantMap postData;
    postData.insert("source", source);
    if (!message.isEmpty())
        postData.insert("message", message);

    bool requestMade = request(IdentifiableContentItemInterface::Post,
                               identifier(), QLatin1String("photos"),
                               QStringList(), postData, extraData);

    if (!requestMade)
        return false;

    f->action = FacebookInterfacePrivate::UploadPhotoAction;
    connect(f->dd->reply(), SIGNAL(finished()), f, SLOT(finishedHandler()));
    connect(f->dd->reply(), SIGNAL(error(QNetworkReply::NetworkError)), f->dd, SLOT(defaultErrorHandler(QNetworkReply::NetworkError)));
    connect(f->dd->reply(), SIGNAL(sslErrors(QList<QSslError>)), f->dd, SLOT(defaultSslErrorsHandler(QList<QSslError>)));
    return true;
}

/*!
    \qmlmethod bool FacebookUser::removePhoto(const QString &identifier)
    Initiates a "delete photo" operation on the photo specified by
    the given \a identifier.

    If the network request was started successfully, the function
    will return true and the status of the user will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookUserInterface::removePhoto(const QString &photoIdentifier)
{
    bool requestMade = request(IdentifiableContentItemInterface::Delete, photoIdentifier);

    if (!requestMade)
        return false;

    f->action = FacebookInterfacePrivate::DeletePhotoAction;
    connect(f->dd->reply(), SIGNAL(finished()), f, SLOT(finishedHandler()));
    connect(f->dd->reply(), SIGNAL(error(QNetworkReply::NetworkError)), f->dd, SLOT(defaultErrorHandler(QNetworkReply::NetworkError)));
    connect(f->dd->reply(), SIGNAL(sslErrors(QList<QSslError>)), f->dd, SLOT(defaultSslErrorsHandler(QList<QSslError>)));
    return true;
}

/*!
    \qmlmethod bool FacebookUser::uploadAlbum(const QString &name, const QString &message, const QVariantMap &privacy)
    Initiates a "post album" operation on the user.  The album
    will be created with the given \a name and be described by the
    given \a message, and will have the specified \a privacy.

    If the network request was started successfully, the function
    will return true and the status of the user will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.

    Once the network request completes, the \c responseReceived()
    signal will be emitted.  The \c data parameter of the signal
    will contain the \c id of the newly uploaded album.
*/
bool FacebookUserInterface::uploadAlbum(const QString &name, const QString &message, const QVariantMap &privacy)
{
    QVariantMap postData;
    postData.insert("name", name);
    if (!message.isEmpty())
        postData.insert("message", message);
    if (privacy != QVariantMap())
        postData.insert("privacy", privacy);

    bool requestMade = request(IdentifiableContentItemInterface::Post,
                               identifier(), QLatin1String("albums"),
                               QStringList(), postData);

    if (!requestMade)
        return false;

    f->action = FacebookInterfacePrivate::UploadAlbumAction;
    connect(f->dd->reply(), SIGNAL(finished()), f, SLOT(finishedHandler()));
    connect(f->dd->reply(), SIGNAL(error(QNetworkReply::NetworkError)), f->dd, SLOT(defaultErrorHandler(QNetworkReply::NetworkError)));
    connect(f->dd->reply(), SIGNAL(sslErrors(QList<QSslError>)), f->dd, SLOT(defaultSslErrorsHandler(QList<QSslError>)));
    return true;
}

/*!
    \qmlmethod bool FacebookUser::removeAlbum(const QString &identifier)
    Initiates a "delete album" operation on the album specified by
    the given \a identifier.

    If the network request was started successfully, the function
    will return true and the status of the user will change to
    \c SocialNetwork::Busy.  Otherwise, the function will return
    false.
*/
bool FacebookUserInterface::removeAlbum(const QString &albumIdentifier)
{
    bool requestMade = request(IdentifiableContentItemInterface::Delete, albumIdentifier);

    if (!requestMade)
        return false;

    f->action = FacebookInterfacePrivate::DeleteAlbumAction;
    connect(f->dd->reply(), SIGNAL(finished()), f, SLOT(finishedHandler()));
    connect(f->dd->reply(), SIGNAL(error(QNetworkReply::NetworkError)), f->dd, SLOT(defaultErrorHandler(QNetworkReply::NetworkError)));
    connect(f->dd->reply(), SIGNAL(sslErrors(QList<QSslError>)), f->dd, SLOT(defaultSslErrorsHandler(QList<QSslError>)));
    return true;
}


/*!
    \qmlproperty QString FacebookUser::name
    Holds the full name of the user
*/
QString FacebookUserInterface::name() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_NAME).toString();
}

/*!
    \qmlproperty QString FacebookUser::firstName
    Holds the first name of the user
*/
QString FacebookUserInterface::firstName() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_FIRSTNAME).toString();
}

/*!
    \qmlproperty QString FacebookUser::middleName
    Holds the middle name of the user
*/
QString FacebookUserInterface::middleName() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_MIDDLENAME).toString();
}

/*!
    \qmlproperty QString FacebookUser::lastName
    Holds the last name of the user
*/
QString FacebookUserInterface::lastName() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_LASTNAME).toString();
}

/*!
    \qmlproperty QString FacebookUser::gender
    Holds the gender of the user
*/
QString FacebookUserInterface::gender() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_GENDER).toString();
}

/*!
    \qmlproperty QString FacebookUser::locale
    Holds the locale of the user
*/
QString FacebookUserInterface::locale() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_LOCALE).toString();
}

/*!
    \qmlproperty QUrl FacebookUser::link
    Holds a link to the profile of the user
*/
QUrl FacebookUserInterface::link() const
{
    return QUrl(d->data().value(FACEBOOK_ONTOLOGY_USER_LINK).toString());
}

/*!
    \qmlproperty QString FacebookUser::userName
    Holds the username of the user
*/
QString FacebookUserInterface::userName() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_USERNAME).toString();
}

/*!
    \qmlproperty QString FacebookUser::thirdPartyIdentifier
    Holds the third party identifier of the user
*/
QString FacebookUserInterface::thirdPartyIdentifier() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_THIRDPARTYIDENTIFIER).toString();
}

/*!
    \qmlproperty bool FacebookUser::installed
    Whether the user has installed the application associated with
    the access token which made the request to the Facebook service
*/
bool FacebookUserInterface::installed() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_INSTALLED).toString() == QLatin1String("true");
}

/*!
    \qmlproperty qreal FacebookUser::timezoneOffset
    Holds the timezone offset of the user
*/
qreal FacebookUserInterface::timezoneOffset() const
{
    QString tzoStr = d->data().value(FACEBOOK_ONTOLOGY_USER_TIMEZONEOFFSET).toString();
    bool ok = false;
    qreal tzo = tzoStr.toDouble(&ok);
    if (!ok)
        return 0.0;
    return tzo;
}

/*!
    \qmlproperty QString FacebookUser::updatedTime
    Holds the last-update time of the user as an ISO8601-formatted string
*/
QString FacebookUserInterface::updatedTime() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_UPDATEDTIME).toString();
}

/*!
    \qmlproperty bool FacebookUser::verified
    Whether the user has been verified
*/
bool FacebookUserInterface::verified() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_VERIFIED).toString() == QLatin1String("true");
}

/*!
    \qmlproperty QString FacebookUser::bio
    Holds the biographical details of the user
*/
QString FacebookUserInterface::bio() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_BIO).toString();
}

/*!
    \qmlproperty QString FacebookUser::birthday
    Holds the birthday of the user in MM/dd/YYYY format
*/
QString FacebookUserInterface::birthday() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_BIRTHDAY).toString();
}

/*!
    \qmlproperty QString FacebookUser::email
    Holds the email address of the user
*/
QString FacebookUserInterface::email() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_EMAIL).toString();
}

/*!
    \qmlproperty FacebookObjectReference *FacebookUser::hometown
    Holds a reference to the place object which is the user's home town
*/
FacebookObjectReferenceInterface *FacebookUserInterface::hometown() const
{
    return f->hometown;
}

/*!
    \qmlproperty QStringList FacebookUser::interestedIn
    Holds a list of the user's personal gender preferences in the
    context of relationships
*/
QStringList FacebookUserInterface::interestedIn() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_INTERESTEDIN).toStringList();
}

/*!
    \qmlproperty FacebookObjectReference *FacebookUser::location
    Holds a reference to the place object which is the user's current domicile location
*/
FacebookObjectReferenceInterface *FacebookUserInterface::location() const
{
    return f->location;
}

/*!
    \qmlproperty QString FacebookUser::political
    Holds the political views which the user identifies with
*/
QString FacebookUserInterface::political() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_POLITICAL).toString();
}

/*!
    \qmlproperty FacebookPicture *FacebookUser::picture
    Holds a reference to the picture associated with the user
*/
FacebookPictureInterface *FacebookUserInterface::picture() const
{
    return f->picture;
}

/*!
    \qmlproperty QString FacebookUser::quotes
    Holds some of the user's favourite quotes
*/
QString FacebookUserInterface::quotes() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_QUOTES).toString();
}

/*!
    \qmlproperty Facebookuser::RelationshipStatus FacebookUser::relationshipStatus
    Holds the current relationship status of the user
*/
FacebookUserInterface::RelationshipStatus FacebookUserInterface::relationshipStatus() const
{
    QString statusStr = d->data().value(FACEBOOK_ONTOLOGY_USER_RELATIONSHIPSTATUS).toString();
    if (statusStr.toLower() == QLatin1String("single"))
        return FacebookUserInterface::Single;
    if (statusStr.toLower() == QLatin1String("in a relationship"))
        return FacebookUserInterface::InARelationship;
    if (statusStr.toLower() == QLatin1String("engaged"))
        return FacebookUserInterface::Engaged;
    if (statusStr.toLower() == QLatin1String("married"))
        return FacebookUserInterface::Married;
    if (statusStr.toLower() == QLatin1String("it's complicated"))
        return FacebookUserInterface::ItsComplicated;
    if (statusStr.toLower() == QLatin1String("in an open relationship"))
        return FacebookUserInterface::InAnOpenRelationship;
    if (statusStr.toLower() == QLatin1String("widowed"))
        return FacebookUserInterface::Widowed;
    if (statusStr.toLower() == QLatin1String("separated"))
        return FacebookUserInterface::Separated;
    if (statusStr.toLower() == QLatin1String("divorced"))
        return FacebookUserInterface::Divorced;
    if (statusStr.toLower() == QLatin1String("in a civil union"))
        return FacebookUserInterface::InACivilUnion;
    if (statusStr.toLower() == QLatin1String("in a domestic partnership"))
        return FacebookUserInterface::InADomesticPartnership;
    return FacebookUserInterface::Unknown;
}

/*!
    \qmlproperty QString FacebookUser::religion
    Holds the religious views which the user identifies with
*/
QString FacebookUserInterface::religion() const
{
    return d->data().value(FACEBOOK_ONTOLOGY_USER_RELIGION).toString();
}

/*!
    \qmlproperty FacebookObjectReference *FacebookUser::significantOther
    Holds a reference to the person object which is listed as the user's
    significant other (spouse or partner)
*/
FacebookObjectReferenceInterface *FacebookUserInterface::significantOther() const
{
    return f->significantOther;
}

/*!
    \qmlproperty QUrl FacebookUser::website
    Holds a link the user's website
*/
QUrl FacebookUserInterface::website() const
{
    return QUrl(d->data().value(FACEBOOK_ONTOLOGY_USER_WEBSITE).toString());
}

