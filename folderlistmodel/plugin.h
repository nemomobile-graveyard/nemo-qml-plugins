/*
 * Copyright (C) 2012 Robin Burchell <robin+nemo@viroteck.net>
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

#ifndef NEMO_QML_PLUGINS_FOLDERLISTMODEL
#define NEMO_QML_PLUGINS_FOLDERLISTMODEL

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtDeclarative>
#include <QDeclarativeEngine>
#include <QDeclarativeExtensionPlugin>
#include <QVector>
#include <QFileInfo>

#define PLUGIN_CLASS_EXPORT
#define PLUGIN_CLASS_EXTERNAL_EXPORT Q_EXPORT_PLUGIN2(nemofolderlistmodel, NemoFolderListModelPlugin);
#define PLUGIN_CLASS_EXTEND
typedef QDeclarativeExtensionPlugin QmlPluginParent;
typedef QDeclarativeEngine QmlEngine;
Q_DECLARE_METATYPE(QVector<QFileInfo>)

#else
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlExtensionPlugin>

#define PLUGIN_CLASS_EXPORT Q_DECL_EXPORT
#define PLUGIN_CLASS_EXTERNAL_EXPORT
#define PLUGIN_CLASS_EXTEND \
    Q_OBJECT \
    Q_PLUGIN_METADATA(IID "org.nemomobile.folderlistmodel")
typedef QQmlExtensionPlugin QmlPluginParent;
typedef QQmlEngine QmlEngine;
#endif

#include "dirmodel.h"

class PLUGIN_CLASS_EXPORT NemoFolderListModelPlugin  : public QmlPluginParent
{
    PLUGIN_CLASS_EXTEND

public:
    NemoFolderListModelPlugin();
    virtual ~NemoFolderListModelPlugin();

    void initializeEngine(QmlEngine *engine, const char *uri);
    void registerTypes(const char *uri);
};

PLUGIN_CLASS_EXTERNAL_EXPORT

#endif // NEMO_QML_PLUGINS_FOLDERLISTMODEL
