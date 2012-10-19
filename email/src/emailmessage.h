/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef EMAILMESSAGE_H
#define EMAILMESSAGE_H

#include <QDeclarativeItem>

#include <qmailaccount.h>
#include <qmailstore.h>

class EmailMessage : public QDeclarativeItem
{
    Q_OBJECT
    Q_ENUMS(Priority)

public:
    explicit EmailMessage (QDeclarativeItem *parent = 0);
    ~EmailMessage ();

    enum Priority { LowPriority, NormalPriority, HighPriority };

    Q_INVOKABLE void setFrom (const QString &sender);
    Q_INVOKABLE void setTo (const QStringList &toList);
    Q_INVOKABLE void setCc (const QStringList &ccList);
    Q_INVOKABLE void setBcc(const QStringList &bccList);
    Q_INVOKABLE void setSubject (const QString &subject);
    Q_INVOKABLE void setBody (const QString &body, bool textOnly);
    Q_INVOKABLE void setAttachments (const QStringList &uris);
    Q_INVOKABLE void setPriority (Priority priority);
    Q_INVOKABLE void send();
    Q_INVOKABLE void saveDraft();

signals:
    void sendCompleted();

private slots:
    void onSendCompleted();

private:
    void processAttachments();

    QMailMessage m_msg;
    QMailAccount m_account;
    QString m_bodyText;
    QStringList m_attachments;
    bool m_textOnly;
};

#endif
