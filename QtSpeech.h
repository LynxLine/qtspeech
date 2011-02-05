/*  QtSpeech -- a small cross-platform library to use TTS
    Copyright (C) 2010-2011 LynxLine.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General
    Public License along with this library; if not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301 USA */

#ifndef QtSpeech_H
#define QtSpeech_H

#include <QObject>
#include <QVariant>

namespace QtSpeech_v1 {

class QtSpeech : public QObject {
Q_OBJECT
public:
    class Error {
    public:
        Error(QString s):msg(s) {}
        QString msg;
    };

    class InitError : public Error { public: InitError(QString s):Error(s) {} };
    class LogicError : public Error { public: LogicError(QString s):Error(s) {} };
    class CloseError : public Error { public: CloseError(QString s):Error(s) {} };

    virtual ~QtSpeech() {}

};

} // namespace QtSpeech_v1
#endif // QtSpeech_H
