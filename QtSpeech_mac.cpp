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

#include <QtCore>
#include <QtSpeech>
#include <ApplicationServices/ApplicationServices.h>

namespace QtSpeech_v1 { // API v1.0

// some defines
#define Where QString("%1:%2:").arg(__FILE__).arg(__LINE__)
#define SysCall(x,e) {\
    OSErr ok = x;\
    if (ok != noErr) {\
        QString msg = #e;\
        msg += ":"+QString(__FILE__);\
        msg += ":"+QString::number(__LINE__)+":"+#x;\
        throw e(msg);\
    }\
}

// sources
QtSpeech::VoiceNames QtSpeech::voices()
{
    SInt16 count;
    VoiceNames vs;
    VoiceDescription desc;
    SysCall( CountVoices(&count), LogicError);
    SysCall( GetVoiceDescription(NULL, &desc, sizeof(VoiceDescription)), LogicError);
    for (int i=1; i<= count; i++) {
        VoiceSpec voice;
        VoiceDescription info;
        SysCall( GetIndVoice(i, &voice), LogicError);
        SysCall( GetVoiceDescription(&voice, &info, sizeof(VoiceDescription)), LogicError);
        QString name = QString::fromAscii((const char *)(info.name+1), int(info.name[0]));
        VoiceName vname = { QString("macosx:%1").arg(voice.id), name };
        vs << vname;
    }
    return vs;
}

} // namespace QtSpeech_v1
