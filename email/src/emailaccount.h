/*
 * Copyright 2011 Intel Corporation.
 * Copyright (C) 2012 Jolla Ltd.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 	
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef EMAILACCOUNT_H
#define EMAILACCOUNT_H

#include <qmailaccount.h>
#include <qmailserviceconfiguration.h>
#include <qmailserviceaction.h>

#ifdef HAS_MLITE
#include <mgconfitem.h>
#endif

class EmailAccount : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString address READ address WRITE setAddress)
    // read-only property, returns username part of email address
    Q_PROPERTY(QString username READ username)
    // read-only property, returns server part of email address
    Q_PROPERTY(QString server READ server)
    Q_PROPERTY(QString password READ password WRITE setPassword)

    Q_PROPERTY(QString recvType READ recvType WRITE setRecvType)
    Q_PROPERTY(QString recvServer READ recvServer WRITE setRecvServer)
    Q_PROPERTY(QString recvPort READ recvPort WRITE setRecvPort)
    Q_PROPERTY(QString recvSecurity READ recvSecurity WRITE setRecvSecurity)
    Q_PROPERTY(QString recvUsername READ recvUsername WRITE setRecvUsername)
    Q_PROPERTY(QString recvPassword READ recvPassword WRITE setRecvPassword)

    Q_PROPERTY(QString sendServer READ sendServer WRITE setSendServer)
    Q_PROPERTY(QString sendPort READ sendPort WRITE setSendPort)
    Q_PROPERTY(QString sendAuth READ sendAuth WRITE setSendAuth)
    Q_PROPERTY(QString sendSecurity READ sendSecurity WRITE setSendSecurity)
    Q_PROPERTY(QString sendUsername READ sendUsername WRITE setSendUsername)
    Q_PROPERTY(QString sendPassword READ sendPassword WRITE setSendPassword)

    Q_PROPERTY(int preset READ preset WRITE setPreset)

    // error message and code from configuration test
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY testFailed)
    Q_PROPERTY(int errorCode READ errorCode NOTIFY testFailed)

public:
    EmailAccount();
    EmailAccount(const QMailAccount &other);
    ~EmailAccount();

    Q_INVOKABLE bool save();
    Q_INVOKABLE bool remove();
    Q_INVOKABLE void test();
    Q_INVOKABLE void clear();
    Q_INVOKABLE void applyPreset();

    QString description() const;
    void setDescription(QString val);
    bool enabled() const;
    void setEnabled(bool val);
    QString name() const;
    void setName(QString val);
    QString address() const;
    void setAddress(QString val);
    QString username() const;
    QString server() const;
    QString password() const;
    void setPassword(QString val);

    QString recvType() const;
    void setRecvType(QString val);
    QString recvServer() const;
    void setRecvServer(QString val);
    QString recvPort() const;
    void setRecvPort(QString val);
    QString recvSecurity() const;
    void setRecvSecurity(QString val);
    QString recvUsername() const;
    void setRecvUsername(QString val);
    QString recvPassword() const;
    void setRecvPassword(QString val);

    QString sendServer() const;
    void setSendServer(QString val);
    QString sendPort() const;
    void setSendPort(QString val);
    QString sendAuth() const;
    void setSendAuth(QString val);
    QString sendSecurity() const;
    void setSendSecurity(QString val);
    QString sendUsername() const;
    void setSendUsername(QString val);
    QString sendPassword() const;
    void setSendPassword(QString val);

    int preset() const;
    void setPreset(int val);

    QString errorMessage() const;
    int errorCode() const;

    enum PresetType {
        noPreset = 0,
        mobilemePreset,
        gmailPreset,
        yahooPreset,
        aolPreset,
        mslivePreset
    };

signals:
    void testSucceeded();
    void testFailed();

private slots:
    void testConfiguration();
    void activityChanged(QMailServiceAction::Activity activity);

private:
    QMailAccount *mAccount;
    QMailAccountConfiguration *mAccountConfig;
    QMailServiceConfiguration *mRecvCfg;
    QMailServiceConfiguration *mSendCfg;
    QMailRetrievalAction *mRetrievalAction;
    QMailTransmitAction *mTransmitAction;
    QString mRecvType;
    QString mPassword;
    QString mErrorMessage;
    int mErrorCode;

#ifdef HAS_MLITE
    MGConfItem *mUpdateIntervalConf;
    MGConfItem *mSignatureConf;
#endif

    void init();

    // workaround to QMF hiding its base64 password encoder in
    // protected methods
    class Base64 : public QMailServiceConfiguration {
    public:
        static QString decode(const QString &value)
            { return decodeValue(value); }
        static QString encode(const QString &value)
            { return encodeValue(value); }
    };
};


#endif // EMAILACCOUNT_H

