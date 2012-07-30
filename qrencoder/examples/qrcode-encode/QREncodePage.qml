/*
 * main.qml - QREncode QML Example
 *
 * Copyright (C) 2011  Imogen Software Carsten Valdemar Munk
 * All rights reserved.
 *
 * Author: Tom Swindell - <t.swindell@rubyx.co.uk>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the <organization>.
 * 4. Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
import QtQuick 1.0
import com.nokia.meego 1.0

import stage.rubyx.QREncode 1.0
import 'javascripts/storage.js' as Storage

Page {
    id:page

    Component.onCompleted: {
        var text = Storage.getItem('qrcode-last-entry');
        if(text) textarea.text = text;
    }

    Component.onDestruction: {
        Storage.setItem('qrcode-last-entry', textarea.text);
    }

    states: [
        State {
            name:'portrait'
            when:main.inPortrait
            PropertyChanges {
                target:textarea
                x:0
                width:code.width
                height:page.height - code.height
            }
        },
        State {
            name:'landscape'
            when:!main.inPortrait
            PropertyChanges {
                target:textarea
                x:code.width
                width:page.width - code.width
                height:page.height
            }
        }
    ]

    QRCode {
        id:code
        width:main.inPortrait ? parent.width : parent.height
        height:width
        text:textarea.text
        padding:40
        color:'#3f8fdf'
        background:'#ffffff'

        MouseArea {
            anchors.fill:parent
            onClicked:code.save('/tmp/test.png', 512)
        }
    }

    TextArea {
        id:textarea
        anchors.bottom:parent.bottom
        text:'#MeeGo #Harmattan #N950Club'
        onHeightChanged: if(main.inPortrait) {height = 0 + height;}
    }
}

