/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 	
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <qmailstore.h>
#include <qmailmessage.h>

#include "emailaccountsettingsmodel.h"

EmailAccountSettingsModel::EmailAccountSettingsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QHash<int,QByteArray> roles;
    roles[DescriptionRole] = "description";
    roles[EnabledRole] = "enabled";
    roles[NameRole] = "name";
    roles[AddressRole] = "address";
    roles[PasswordRole] = "password";

    roles[RecvTypeRole] = "recvType";
    roles[RecvServerRole] = "recvServer";
    roles[RecvPortRole] = "recvPort";
    roles[RecvSecurityRole] = "recvSecurity";
    roles[RecvUsernameRole] = "recvUsername";
    roles[RecvPasswordRole] = "recvPassword";

    roles[SendServerRole] = "sendServer";
    roles[SendPortRole] = "sendPort";
    roles[SendAuthRole] = "sendAuth";
    roles[SendSecurityRole] = "sendSecurity";
    roles[SendUsernameRole] = "sendUsername";
    roles[SendPasswordRole] = "sendPassword";

    roles[PresetRole] = "preset";
    setRoleNames(roles);

    /*
    service["IMAP"] = "imap4";
    service["POP"] = "pop3";

    encryption["None"] = "0";
    encryption["SSL"] = "1";
    encryption["TLS"] = "2";

    authentication["None"] = "0";
    authentication["Login"] = "1";
    authentication["Plain"] = "2";
    authentication["Cram MD5"] = "3";
    */

#ifdef HAS_MLITE
    mUpdateIntervalConf = new MGConfItem("/apps/meego-app-email/updateinterval");
    mSignatureConf = new MGConfItem("/apps/meego-app-email/signature");
    mNewMailNotificationConf = new MGConfItem("/apps/meego-app-email/newmailnotifications");
    mConfirmDeleteMailConf = new MGConfItem("/apps/meego-app-email/confirmdeletemail");
#endif

    init();
}

void EmailAccountSettingsModel::init()
{
    mAccounts.clear();
    mAccountConfigs.clear();
    QMailStore *mailstore = QMailStore::instance();
    QMailAccountIdList idlist = mailstore->queryAccounts(QMailAccountKey::messageType(QMailMessage::Email));
    QMailAccountId id;
    foreach (id, idlist) {
        mAccounts.append(mailstore->account(id));
        mAccountConfigs.append(mailstore->accountConfiguration(id));
    }

    // TODO: there is not really any point storing these.
#ifdef HAS_MLITE
    // initialize global settings from gconf
    mUpdateInterval = mUpdateIntervalConf->value().toInt();
    mSignature = mSignatureConf->value().toString();
    mNewMailNotification = mNewMailNotificationConf->value().toBool();
    mConfirmDeleteMail = mConfirmDeleteMailConf->value().toBool();
#else
    mUpdateInterval = 60;
    mSignature = "No GConf configured";
    mNewMailNotification = true;
    mConfirmDeleteMail = true;
#endif
}

QMailAccountConfiguration::ServiceConfiguration *EmailAccountSettingsModel::getRecvCfg(QMailAccountConfiguration &acctcfg)
{
    QStringList services;
    QString recvsvc;
    services = acctcfg.services();
    if (services.contains("imap4")) {
        recvsvc = "imap4";
    } else if (services.contains("pop3")) {
        recvsvc = "pop3";
    } else {
        return NULL;
    }
    return &acctcfg.serviceConfiguration(recvsvc);
}

void EmailAccountSettingsModel::reload()
{
    beginResetModel();
    init();
    endResetModel();
}

int EmailAccountSettingsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mAccounts.size();
}

QVariant EmailAccountSettingsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.row() < mAccounts.size()) {
        QMailAccountConfiguration::ServiceConfiguration svccfg;
        QStringList services;
        QString recvsvc;
        QString sendpass, recvpass;
        //determine receiving protocol
        services = mAccountConfigs[index.row()].services();
        if (services.contains("imap4")) {
            recvsvc = "imap4";
        } else if (services.contains("pop3")) {
            recvsvc = "pop3";
        } else {
            qWarning("EmailAccountSettingsModel::data: No IMAP or POP service found for account");
            return QVariant();
        }
        switch (role) {
            case DescriptionRole:
                return mAccounts[index.row()].name();
                break;
            case EnabledRole:
                return bool(mAccounts[index.row()].status() & QMailAccount::Enabled);
                break;
            case NameRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                return svccfg.value("username");
                break;
            case AddressRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                return svccfg.value("address");
                break;
            case PasswordRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration(recvsvc);
                recvpass = QMailDecoder::decode(svccfg.value("password"));
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                sendpass = QMailDecoder::decode(svccfg.value("smtppassword"));
                if (recvpass == sendpass) {
                    return recvpass;
                } else {
                    return QString();
                }
                break;
            case RecvTypeRole:
                if (recvsvc == "pop3")
                    return 0;
                else if (recvsvc == "imap4")
                    return 1;
                else
                    return QVariant();
                break;
            case RecvServerRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration(recvsvc);
                return svccfg.value("server");
                break;
            case RecvPortRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration(recvsvc);
                return svccfg.value("port");
                break;
            case RecvSecurityRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration(recvsvc);
                return svccfg.value("encryption");
                break;
            case RecvUsernameRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration(recvsvc);
                return svccfg.value("username");
                break;
            case RecvPasswordRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration(recvsvc);
                return QMailDecoder::decode(svccfg.value("password"));
                break;
            case SendServerRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                return svccfg.value("server");
                break;
            case SendPortRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                return svccfg.value("port");
                break;
            case SendAuthRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                return svccfg.value("authentication");
                break;
            case SendSecurityRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                return svccfg.value("encryption");
                break;
            case SendUsernameRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                return svccfg.value("smtpusername");
                break;
            case SendPasswordRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                return QMailDecoder::decode(svccfg.value("smtppassword"));
                break;
            case PresetRole:
                return mAccounts[index.row()].customField("preset");
                break;
            default:
                return QVariant();
        }
    }
    return QVariant();
}

bool EmailAccountSettingsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && index.row() < mAccounts.size()) {
        QMailAccountConfiguration::ServiceConfiguration svccfg;
        QStringList services;
        QString recvsvc;
        QString newrecvsvc;
        //determine receiving protocol
        services = mAccountConfigs[index.row()].services();
        if (services.contains("imap4")) {
            recvsvc = "imap4";
        } else if (services.contains("pop3")) {
            recvsvc = "pop3";
        } else {
            qWarning("EmailAccountSettingsModel::setData: No IMAP or POP service found for account");
            return false;
        }
        switch (role) {
            case DescriptionRole:
                mAccounts[index.row()].setName(value.toString());
                return true;
                break;
            case EnabledRole:
                mAccounts[index.row()].setStatus(QMailAccount::Enabled, value.toBool());
                return true;
                break;
            case NameRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                svccfg.setValue("username", value.toString());
                return true;
                break;
            case AddressRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                svccfg.setValue("address", value.toString());
                return true;
                break;
            case PasswordRole:
                if (!value.toString().isEmpty()) {
                    svccfg = mAccountConfigs[index.row()].serviceConfiguration(recvsvc);
                    svccfg.setValue("password", QMailDecoder::encode(value.toString()));
                    svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                    svccfg.setValue("smtppassword", QMailDecoder::encode(value.toString()));
                }
                return true;
                break;
            case RecvTypeRole:
                // prevent bug where recv type gets reset
                // when loading the first time
                if (value.toString() == "0") {
                    newrecvsvc = "pop3";
                } else if (value.toString() == "1") {
                    newrecvsvc = "imap4";
                } else {
                    return false;
                }
                if (newrecvsvc == recvsvc) {
                    return true;
                } else {
                    mAccountConfigs[index.row()].removeServiceConfiguration(recvsvc);
                    mAccountConfigs[index.row()].addServiceConfiguration(newrecvsvc);
                    getRecvCfg(mAccountConfigs[index.row()])->setValue("encryption", "1"); // SSL
                    getRecvCfg(mAccountConfigs[index.row()])->setValue("servicetype", "source");
                    getRecvCfg(mAccountConfigs[index.row()])->setValue("version", "100");
                    // automatically clear the recv fields in the UI
                    emit dataChanged(index, index);
                    return true;
                }
                break;
            case RecvServerRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration(recvsvc);
                svccfg.setValue("server", value.toString());
                return true;
                break;
            case RecvPortRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration(recvsvc);
                svccfg.setValue("port", value.toString());
                return true;
                break;
            case RecvSecurityRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration(recvsvc);
                svccfg.setValue("encryption", value.toString());
                return true;
                break;
            case RecvUsernameRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration(recvsvc);
                svccfg.setValue("username", value.toString());
                return true;
                break;
            case RecvPasswordRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration(recvsvc);
                svccfg.setValue("password", QMailDecoder::encode(value.toString()));
                return true;
                break;
            case SendServerRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                svccfg.setValue("server", value.toString());
                return true;
                break;
            case SendPortRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                svccfg.setValue("port", value.toString());
                return true;
                break;
            case SendAuthRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                svccfg.setValue("authentication", value.toString());
                return true;
                break;
            case SendSecurityRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                svccfg.setValue("encryption", value.toString());
                return true;
                break;
            case SendUsernameRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                svccfg.setValue("smtpusername", value.toString());
                return true;
                break;
            case SendPasswordRole:
                svccfg = mAccountConfigs[index.row()].serviceConfiguration("smtp");
                svccfg.setValue("smtppassword", QMailDecoder::encode(value.toString()));
                return true;
                break;
            case PresetRole:
                // setting preset not implemented here
            default:
                return false;
        }
    }

    return false;
}

QVariant EmailAccountSettingsModel::dataWrapper(int row, int role)
{
    return data(index(row, 0), role);
}

bool EmailAccountSettingsModel::setDataWrapper(int row, const QVariant &value, int role)
{
    return setData(index(row, 0), value, role);
}

int EmailAccountSettingsModel::updateInterval()
{
    return mUpdateInterval;
}

void EmailAccountSettingsModel::setUpdateInterval(int interval)
{
    mUpdateInterval = interval;
}

QString EmailAccountSettingsModel::signature()
{
    return mSignature;
}

void EmailAccountSettingsModel::setSignature(QString signature)
{
    mSignature = signature;
}

bool EmailAccountSettingsModel::newMailNotifications()
{
    return mNewMailNotification;
}

void EmailAccountSettingsModel::setNewMailNotifications(bool val)
{
    mNewMailNotification = val;
}

bool EmailAccountSettingsModel::confirmDeleteMail()
{
    return mConfirmDeleteMail;
}

void EmailAccountSettingsModel::setConfirmDeleteMail(bool val)
{
    mConfirmDeleteMail = val;
}

void EmailAccountSettingsModel::saveChanges()
{
    int i;
    QMailStore *mailstore = QMailStore::instance();

#ifdef HAS_MLITE
    mUpdateIntervalConf->set(mUpdateInterval);
    mSignatureConf->set(mSignature);
    mNewMailNotificationConf->set(mNewMailNotification);
    mConfirmDeleteMailConf->set(mConfirmDeleteMail);
#endif

    for (i = 0; i < mAccounts.size(); i++) {
        //set update interval and signature globally
        mAccounts[i].setSignature(mSignature);
        mAccounts[i].setStatus(QMailAccount::AppendSignature, !mSignature.isEmpty());
        getRecvCfg(mAccountConfigs[i])->setValue("checkInterval", QString("%1").arg(mUpdateInterval));
        mailstore->updateAccount(&mAccounts[i], &mAccountConfigs[i]);
    }
}

void EmailAccountSettingsModel::deleteRow(int row)
{
    // deletes a row immediately, rather than when saveChanges is called
    if (row >= 0 && row < mAccounts.size()) {
        beginResetModel();
        QMailStore *mailstore = QMailStore::instance();
        mailstore->removeAccount(mAccounts[row].id());
        init();
        endResetModel();
    }
}
