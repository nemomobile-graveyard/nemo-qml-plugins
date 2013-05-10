/*
 * libseaside - Library that provides an interface to the Contacts application
 * Copyright (c) 2013, Matt Vogt
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "normalization_p.h"

namespace Normalization {

QString normalizePhoneNumber(const QString &input)
{
    // Use the same algorithm as maemo localNumber

    // Not actually the 'visual-separators' from RFC3966...
    // This logic is derived from qtcontacts-tracker
    static const QString separators(QString::fromLatin1(" .-()[]"));
    static const QString dtmfChars(QString::fromLatin1("pPwWxX"));

    // TODO: possibly make this tunable?
    static const int maxCharacters = 7;

    QString subset;
    subset.reserve(input.length());

    QString::const_iterator it = input.constBegin(), end = input.constEnd();
    for ( ; it != end; ++it) {
        if ((*it).isDigit()) {
            // Convert to ASCII, capturing unicode digit values
            subset.append(QChar::fromLatin1('0' + (*it).digitValue()));
        } else if (!separators.contains(*it) &&
                   (*it).category() != QChar::Other_Format) {
            // If this is a DTMF character, stop processing here
            if (dtmfChars.contains(*it)) {
                break;
            } else {
                subset.append(*it);
            }
        }
    }

    return subset.right(maxCharacters);
}

}

