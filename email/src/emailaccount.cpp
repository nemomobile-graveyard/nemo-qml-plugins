/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 	
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QNetworkConfigurationManager>
#include <QTimer>

#include <qmailstore.h>
#include <qmailmessage.h>

#include "emailaccount.h"

EmailAccount::EmailAccount()
    : mAccount(new QMailAccount())
    , mAccountConfig(new QMailAccountConfiguration())
    , mRecvCfg(0)
    , mSendCfg(0)
    , mRetrievalAction(new QMailRetrievalAction(this))
    , mTransmitAction(new QMailTransmitAction(this))
    , mErrorCode(0)
#ifdef HAS_MLITE
    , mUpdateIntervalConf(new MGConfItem("/apps/meego-app-email/updateinterval"))
    , mSignatureConf(new MGConfItem("/apps/meego-app-email/signature"))
#endif
{ 
    mAccount->setMessageType(QMailMessage::Email);
    init();
}

EmailAccount::EmailAccount(const QMailAccount &other)
    : mAccount(new QMailAccount(other))
    , mAccountConfig(new QMailAccountConfiguration())
    , mRecvCfg(0)
    , mSendCfg(0)
    , mRetrievalAction(new QMailRetrievalAction(this))
    , mTransmitAction(new QMailTransmitAction(this))
    , mErrorCode(0)
#ifdef HAS_MLITE
    , mUpdateIntervalConf(new MGConfItem("/apps/meego-app-email/updateinterval"))
    , mSignatureConf(new MGConfItem("/apps/meego-app-email/signature"))
#endif
{
    *mAccountConfig = QMailStore::instance()->accountConfiguration(mAccount->id());
    init();
}

EmailAccount::~EmailAccount()
{
    delete mRecvCfg;
    delete mSendCfg;
    delete mAccount;
}

void EmailAccount::init()
{
    QStringList services = mAccountConfig->services();
    if (!services.contains("qmfstoragemanager")) {
        // add qmfstoragemanager configuration
        mAccountConfig->addServiceConfiguration("qmfstoragemanager");
        QMailServiceConfiguration storageCfg(mAccountConfig, "qmfstoragemanager");
        storageCfg.setType(QMailServiceConfiguration::Storage);
        storageCfg.setVersion(101);
        storageCfg.setValue("basePath", "");
    }
    if (!services.contains("smtp")) {
        // add SMTP configuration
        mAccountConfig->addServiceConfiguration("smtp");
    }
    if (services.contains("imap4")) {
        mRecvType = "imap4";
    } else if (services.contains("pop3")) {
        mRecvType = "pop3";
    } else {
        // add POP configuration
        mRecvType = "pop3";
        mAccountConfig->addServiceConfiguration(mRecvType);
    }
    mSendCfg = new QMailServiceConfiguration(mAccountConfig, "smtp");
    mRecvCfg = new QMailServiceConfiguration(mAccountConfig, mRecvType);
    mSendCfg->setType(QMailServiceConfiguration::Sink);
    mSendCfg->setVersion(100);
    mRecvCfg->setType(QMailServiceConfiguration::Source);
    mRecvCfg->setVersion(100);

    /*
    serviceCode["IMAP"] = "imap4";
    serviceCode["POP"] = "pop3";

    encryptionCode["None"] = "0";
    encryptionCode["SSL"] = "1";
    encryptionCode["TLS"] = "2";

    authenticationCode["None"] = "0";
    authenticationCode["Login"] = "1";
    authenticationCode["Plain"] = "2";
    authenticationCode["Cram MD5"] = "3";
    */

    connect(mRetrievalAction, SIGNAL(activityChanged(QMailServiceAction::Activity)), this, SLOT(activityChanged(QMailServiceAction::Activity)));
    connect(mTransmitAction, SIGNAL(activityChanged(QMailServiceAction::Activity)), this, SLOT(activityChanged(QMailServiceAction::Activity)));
}

void EmailAccount::clear()
{
    delete mAccount;
    delete mAccountConfig;
    mAccount = new QMailAccount();
    mAccountConfig = new QMailAccountConfiguration();
    mAccount->setMessageType(QMailMessage::Email);
    mPassword.clear();
    init();
}

bool EmailAccount::save()
{
    bool result;
#ifdef HAS_MLITE
    QString signature = mSignatureConf->value().toString();
    mAccount->setSignature(signature);
    mAccount->setStatus(QMailAccount::AppendSignature, !signature.isEmpty());
#endif

#ifdef HAS_MLITE
    mRecvCfg->setValue("checkInterval", QString("%1").arg(mUpdateIntervalConf->value().toInt()));
#endif

    mAccount->setStatus(QMailAccount::UserEditable, true);
    mAccount->setStatus(QMailAccount::UserRemovable, true);
    mAccount->setStatus(QMailAccount::MessageSource, true);
    mAccount->setStatus(QMailAccount::CanRetrieve, true);
    mAccount->setStatus(QMailAccount::MessageSink, true);
    mAccount->setStatus(QMailAccount::CanTransmit, true);
    mAccount->setStatus(QMailAccount::Enabled, true);
    mAccount->setStatus(QMailAccount::CanCreateFolders, true);
    mAccount->setFromAddress(QMailAddress(address()));
    if (mAccount->id().isValid()) {
        result = QMailStore::instance()->updateAccount(mAccount, mAccountConfig);
    } else {
        if (preset() == noPreset) {
            // set description to server for custom email accounts
            setDescription(server());
        }
        result = QMailStore::instance()->addAccount(mAccount, mAccountConfig);
    }
    return result;
}

bool EmailAccount::remove()
{
    bool result = false;
    if (mAccount->id().isValid()) {
        result = QMailStore::instance()->removeAccount(mAccount->id());
        mAccount->setId(QMailAccountId());
    }
    return result;
}

void EmailAccount::test()
{
    QNetworkConfigurationManager networkManager;
    if (networkManager.isOnline()) {
        QTimer::singleShot(5000, this, SLOT(testConfiguration()));
    } else {
        // skip test if not connected to a network
        emit testSucceeded();
    }
}

void EmailAccount::testConfiguration()
{
    if (mAccount->id().isValid()) {
        if (mAccount->status() & QMailAccount::MessageSource) {
            mRetrievalAction->retrieveFolderList(mAccount->id(), QMailFolderId(), true);
        } else if (mAccount->status() & QMailAccount::MessageSink) {
            mTransmitAction->transmitMessages(mAccount->id());
        } else {
            qWarning() << "account has no message sources or sinks";
        }
    } else {
    }
}

void EmailAccount::activityChanged(QMailServiceAction::Activity activity)
{
    static const int AccountUpdatedByOther = 1040;

    if (sender() == static_cast<QObject*>(mRetrievalAction)) {
        const QMailServiceAction::Status status(mRetrievalAction->status());

        if ((activity == QMailServiceAction::Failed) && (status.errorCode == AccountUpdatedByOther)) {
            // ignore error 1040/"Account updated by other process"
            activity = QMailServiceAction::Successful;
        }

        if (activity == QMailServiceAction::Successful) {
            if (mAccount->status() & QMailAccount::MessageSink) {
                mTransmitAction->transmitMessages(mAccount->id());
            } else {
                emit testSucceeded();
            }
        } else if (activity == QMailServiceAction::Failed) {
            mErrorMessage = status.text;
            mErrorCode = status.errorCode;
            emit testFailed();
        }
    } else if (sender() == static_cast<QObject*>(mTransmitAction)) {
        const QMailServiceAction::Status status(mTransmitAction->status());

        if ((activity == QMailServiceAction::Failed) && (status.errorCode == AccountUpdatedByOther)) {
            // ignore error 1040/"Account updated by other process"
            activity = QMailServiceAction::Successful;
        }

        if (activity == QMailServiceAction::Successful) {
            emit testSucceeded();
        } else if (activity == QMailServiceAction::Failed) {
            mErrorMessage = status.text;
            mErrorCode = status.errorCode;
            emit testFailed();
        }
    }
}

void EmailAccount::applyPreset()
{
    switch(preset()) {
        case mobilemePreset:
            setRecvType("1");                    // imap
            setRecvServer("mail.me.com");
            setRecvPort("993");
            setRecvSecurity("1");                // SSL
            setRecvUsername(username());         // username only
            setRecvPassword(password());
            setSendServer("smtp.me.com");
            setSendPort("587");
            setSendSecurity("1");                // SSL
            setSendAuth("1");                    // Login
            setSendUsername(username());         // username only
            setSendPassword(password());
            break;
        case gmailPreset:
            setRecvType("1");                    // imap
            setRecvServer("imap.gmail.com");
            setRecvPort("993");
            setRecvSecurity("1");                // SSL
            setRecvUsername(address());          // full email address
            setRecvPassword(password());
            setSendServer("smtp.gmail.com");
            setSendPort("465");
            setSendSecurity("1");                // SSL
            setSendAuth("1");                    // Login
            setSendUsername(address());          // full email address
            setSendPassword(password());
            break;
        case yahooPreset:
            setRecvType("1");                    // imap
            setRecvServer("imap.mail.yahoo.com");
            setRecvPort("993");
            setRecvSecurity("1");                // SSL
            setRecvUsername(address());          // full email address
            setRecvPassword(password());
            setSendServer("smtp.mail.yahoo.com");
            setSendPort("465");
            setSendSecurity("1");                // SSL
            setSendAuth("1");                    // Login
            setSendUsername(address());          // full email address
            setSendPassword(password());
            break;
        case aolPreset:
            setRecvType("1");                    // imap
            setRecvServer("imap.aol.com");
            setRecvPort("143");
            setRecvSecurity("0");                // none
            setRecvUsername(username());         // username only
            setRecvPassword(password());
            setSendServer("smtp.aol.com");
            setSendPort("587");
            setSendSecurity("0");                // none
            setSendAuth("1");                    // Login
            setSendUsername(username());         // username only
            setSendPassword(password());
            break;
        case mslivePreset:
            setRecvType("0");                    // pop
            setRecvServer("pop3.live.com");
            setRecvPort("995");
            setRecvSecurity("1");                // SSL
            setRecvUsername(address());          // full email address
            setRecvPassword(password());
            setSendServer("smtp.live.com");
            setSendPort("587");
            setSendSecurity("2");                // TLS
            setSendAuth("1");                    // Login
            setSendUsername(address());          // full email address
            setSendPassword(password());
            break;
        case noPreset:
            setRecvType("1");                    // imap
            setRecvPort("993");
            setRecvSecurity("1");                // SSL
            setRecvUsername(username());         // username only
            setRecvPassword(password());
            setSendPort("587");
            setSendSecurity("1");                // SSL
            setSendAuth("1");                    // Login
            setSendUsername(username());         // username only
            setSendPassword(password());
            break;
        default:
            break;
    }
}

QString EmailAccount::description() const
{
    return mAccount->name();
}

void EmailAccount::setDescription(QString val)
{
    mAccount->setName(val);
}

bool EmailAccount::enabled() const
{
    return mAccount->status() & QMailAccount::Enabled;
}

void EmailAccount::setEnabled(bool val)
{
    mAccount->setStatus(QMailAccount::Enabled, val);
}

QString EmailAccount::name() const
{
    return mSendCfg->value("username");
}

void EmailAccount::setName(QString val)
{
    mSendCfg->setValue("username", val);
}

QString EmailAccount::address() const
{
    return mSendCfg->value("address");
}

void EmailAccount::setAddress(QString val)
{
    mSendCfg->setValue("address", val);
}

QString EmailAccount::username() const
{
    // read-only property, returns username part of email address
    return address().remove(QRegExp("@.*$"));
}

QString EmailAccount::server() const
{
    // read-only property, returns server part of email address
    return address().remove(QRegExp("^.*@"));
}

QString EmailAccount::password() const
{
    return mPassword;
}

void EmailAccount::setPassword(QString val)
{
    mPassword = val;
}

QString EmailAccount::recvType() const
{
    if (mRecvType == "pop3")
        return "0";
    else if (mRecvType == "imap4")
        return "1";
    else
        return QString();
}

void EmailAccount::setRecvType(QString val)
{
    // prevent bug where recv type gets reset
    // when loading the first time
    QString newRecvType;
    if (val == "0")
        newRecvType = "pop3";
    else if (val == "1")
        newRecvType = "imap4";
    if (newRecvType != mRecvType) {
        mAccountConfig->removeServiceConfiguration(mRecvType);
        mAccountConfig->addServiceConfiguration(newRecvType);
        mRecvType = newRecvType;
        delete mRecvCfg;
        mRecvCfg = new QMailServiceConfiguration(mAccountConfig, mRecvType);
        mRecvCfg->setType(QMailServiceConfiguration::Source);
        mRecvCfg->setVersion(100);
        /*
        if (newRecvType == "imap4") {
            mRecvCfg->setValue("authentication", "0");
            mRecvCfg->setValue("autoDownload", "0");
            mRecvCfg->setValue("baseFolder", "");
            mRecvCfg->setValue("canDelete", "1");
            mRecvCfg->setValue("checkInterval", "0");
            mRecvCfg->setValue("intervalCheckRoamingEnabled", "1");
            mRecvCfg->setValue("maxSize", "20");
            mRecvCfg->setValue("pushEnabled", "0");
            mRecvCfg->setValue("pushCapable", "1");
            mRecvCfg->setValue("pushFolders", "INBOX");
            mRecvCfg->setValue("textSubtype", "html");
            mRecvCfg->setValue("capabilities", "IMAP4rev1 SORT THREAD=REFERENCES MULTIAPPEND UNSELECT LITERAL+ IDLE CHILDREN NAMESPACE LOGIN-REFERRALS AUTH=PLAIN");
        }
        */
    }
}

QString EmailAccount::recvServer() const
{
    return mRecvCfg->value("server");
}

void EmailAccount::setRecvServer(QString val)
{
    mRecvCfg->setValue("server", val);
}

QString EmailAccount::recvPort() const
{
    return mRecvCfg->value("port");
}

void EmailAccount::setRecvPort(QString val)
{
    mRecvCfg->setValue("port", val);
}

QString EmailAccount::recvSecurity() const
{
    return mRecvCfg->value("encryption");
}

void EmailAccount::setRecvSecurity(QString val)
{
    mRecvCfg->setValue("encryption", val);
}

QString EmailAccount::recvUsername() const
{
    return mRecvCfg->value("username");
}

void EmailAccount::setRecvUsername(QString val)
{
    mRecvCfg->setValue("username", val);
}

QString EmailAccount::recvPassword() const
{
    return Base64::decode(mRecvCfg->value("password"));
}

void EmailAccount::setRecvPassword(QString val)
{
    mRecvCfg->setValue("password", Base64::encode(val));
}

QString EmailAccount::sendServer() const
{
    return mSendCfg->value("server");
}

void EmailAccount::setSendServer(QString val)
{
    mSendCfg->setValue("server", val);
}

QString EmailAccount::sendPort() const
{
    return mSendCfg->value("port");
}

void EmailAccount::setSendPort(QString val)
{
    mSendCfg->setValue("port", val);
}

QString EmailAccount::sendAuth() const
{
    return mSendCfg->value("authentication");
}

void EmailAccount::setSendAuth(QString val)
{
    mSendCfg->setValue("authentication", val);
}

QString EmailAccount::sendSecurity() const
{
    return mSendCfg->value("encryption");
}

void EmailAccount::setSendSecurity(QString val)
{
    mSendCfg->setValue("encryption", val);
}

QString EmailAccount::sendUsername() const
{
    return mSendCfg->value("smtpusername");
}

void EmailAccount::setSendUsername(QString val)
{
    mSendCfg->setValue("smtpusername", val);
}

QString EmailAccount::sendPassword() const
{
    return Base64::decode(mSendCfg->value("smtppassword"));
}

void EmailAccount::setSendPassword(QString val)
{
    mSendCfg->setValue("smtppassword", Base64::encode(val));
}

int EmailAccount::preset() const
{
    return mAccount->customField("preset").toInt();
}

void EmailAccount::setPreset(int val)
{
    mAccount->setCustomField("preset", QString::number(val));
}

QString EmailAccount::errorMessage() const
{
    return mErrorMessage;
}

int EmailAccount::errorCode() const
{
    return mErrorCode;
}
