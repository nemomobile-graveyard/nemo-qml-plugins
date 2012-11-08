/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */


#include <QDeclarativeItem>
#include <QFileInfo>

#include <qmailaccount.h>
#include <qmailstore.h>

#include "emailagent.h"
#include "emailmessage.h"
#include "qmailnamespace.h"

EmailMessage::EmailMessage (QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_textOnly(true)
{
    setPriority(NormalPriority);
}

EmailMessage::~EmailMessage ()
{
}

void EmailMessage::setFrom (const QString &sender)
{
    QMailAccountIdList accountIds = QMailStore::instance()->queryAccounts(QMailAccountKey::status(QMailAccount::Enabled,
                                            QMailDataComparator::Includes), QMailAccountSortKey::name());
    // look up the account id for the given sender
    foreach (QMailAccountId id, accountIds) {
        QMailAccount account(id);
        QMailAddress from = account.fromAddress();
        if (from.address() == sender || from.toString() == sender || from.name() == sender) {
            m_account = account;
            m_msg.setParentAccountId(id);
            m_msg.setFrom(account.fromAddress());
        }
    }
}

void EmailMessage::setTo (const QStringList &toList)
{
    m_msg.setTo(QMailAddress::fromStringList(toList));
}

void EmailMessage::setCc (const QStringList &ccList)
{
    m_msg.setCc(QMailAddress::fromStringList(ccList));
}

void EmailMessage::setBcc(const QStringList &bccList)
{
    m_msg.setBcc(QMailAddress::fromStringList(bccList));
}

void EmailMessage::setSubject (const QString &subject)
{
    m_msg.setSubject(subject);
}

void EmailMessage::setBody (const QString &body, bool textOnly)
{
    m_textOnly = textOnly;
    m_bodyText = body;
}

void EmailMessage::setAttachments (const QStringList &uris)
{
    m_attachments = uris;
}

void EmailMessage::setPriority (EmailMessage::Priority priority)
{
    switch (priority) {
    case HighPriority:
        m_msg.appendHeaderField("X-Priority", "1");
        m_msg.appendHeaderField("X-MSMail-Priority", "High");
        break;
    case LowPriority:
        m_msg.appendHeaderField("X-Priority", "5");
        m_msg.appendHeaderField("X-MSMail-Priority", "Low");
        break;
    case NormalPriority:
    default:
        m_msg.appendHeaderField("X-Priority", "3");
        m_msg.appendHeaderField("X-MSMail-Priority", "Normal");
        break;
    }
}

void EmailMessage::send()
{
    buildMessage();

    bool stored = false;

    if (!m_msg.id().isValid())
        stored = QMailStore::instance()->addMessage(&m_msg);
    else
        stored = QMailStore::instance()->updateMessage(&m_msg);

    EmailAgent *emailAgent = EmailAgent::instance();
    if (stored && !emailAgent->isSynchronizing()) {
        connect(emailAgent, SIGNAL(sendCompleted()), this, SLOT(onSendCompleted()));
        emailAgent->sendMessages(m_msg.parentAccountId());
    }
    else
       qDebug() << "Error queuing message, stored: " << stored << "isSynchronising: " << emailAgent->isSynchronizing();

}

void EmailMessage::saveDraft()
{
    buildMessage();

    QMailFolderKey nameKey(QMailFolderKey::displayName("Drafts", QMailDataComparator::Includes));
    QMailFolderKey accountKey(QMailFolderKey::parentAccountId(m_msg.parentAccountId()));
    QMailFolderIdList draftsFolders = QMailStore::instance()->queryFolders(nameKey & accountKey);

    if (draftsFolders.length() > 0 && draftsFolders[0].isValid()) {
        m_msg.setParentFolderId(draftsFolders[0]);

        bool saved = false;

        // Unset outgoing and outbox so it wont really send
        // when we sync to the server Drafts folder
        m_msg.setStatus(QMailMessage::Outgoing, false);
        m_msg.setStatus(QMailMessage::Outbox, false);
        if (!m_msg.id().isValid())
            saved = QMailStore::instance()->addMessage(&m_msg);
        else
            saved = QMailStore::instance()->updateMessage(&m_msg);
        //
        // Sync to the server, so the message will be in the remote Drafts folder
        if (saved) {
            EmailAgent::instance()->flagMessages(QMailMessageIdList() << m_msg.id(),
                QMailMessage::Draft, 0);
            EmailAgent::instance()->exportAccountChanges(m_msg.parentAccountId());
        }
    }
}

void EmailMessage::onSendCompleted()
{
    emit sendCompleted();
}

void EmailMessage::buildMessage()
{
    QMailMessageContentType type;
    if (m_textOnly)
        type.setType("text/plain; charset=UTF-8");
    else
        type.setType("text/html; charset=UTF-8");

    if (m_attachments.size() == 0)
        m_msg.setBody(QMailMessageBody::fromData(m_bodyText, type, QMailMessageBody::Base64));
    else {
        QMailMessagePart body;
        body.setBody(QMailMessageBody::fromData(m_bodyText.toUtf8(), type, QMailMessageBody::Base64));
        m_msg.setMultipartType(QMailMessagePartContainer::MultipartMixed);
        m_msg.appendPart(body);
    }

    // Include attachments into the message
    processAttachments();

    // set message basic attributes
    m_msg.setDate(QMailTimeStamp::currentDateTime());
    m_msg.setStatus(QMailMessage::Outgoing, true);
    m_msg.setStatus(QMailMessage::ContentAvailable, true);
    m_msg.setStatus(QMailMessage::PartialContentAvailable, true);
    m_msg.setStatus(QMailMessage::Read, true);
    m_msg.setStatus((QMailMessage::Outbox | QMailMessage::Draft), true);

    m_msg.setParentFolderId(QMailFolder::LocalStorageFolderId);

    m_msg.setMessageType(QMailMessage::Email);
    m_msg.setSize(m_msg.indicativeSize() * 1024);
}

void EmailMessage::processAttachments ()
{
    QMailMessagePart attachmentPart;
    foreach (QString attachment, m_attachments) {
        // Attaching a file
        if (attachment.startsWith("file://"))
            attachment.remove(0, 7);
        QFileInfo fi(attachment);

        // Just in case..
        if (!fi.isFile())
            continue;

        QMailMessageContentType attachmenttype(QMail::mimeTypeFromFileName(attachment).toLatin1());
        attachmenttype.setName(fi.fileName().toLatin1());

        QMailMessageContentDisposition disposition(QMailMessageContentDisposition::Attachment);
        disposition.setFilename(fi.fileName().toLatin1());
        disposition.setSize(fi.size());

        attachmentPart = QMailMessagePart::fromFile(attachment,
                                                    disposition,
                                                    attachmenttype,
                                                    QMailMessageBody::Base64,
                                                    QMailMessageBody::RequiresEncoding);
        m_msg.appendPart(attachmentPart);
    }
}
