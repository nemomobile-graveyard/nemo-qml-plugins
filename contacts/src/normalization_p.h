/*
 * libseaside - Library that provides an interface to the Contacts application
 * Copyright (c) 2013, Matt Vogt
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#ifndef __NORMALIZATION_P_H__
#define __NORMALIZATION_P_H__

#include <QString>

namespace Normalization {

QString normalizePhoneNumber(const QString &input);

}

#endif
