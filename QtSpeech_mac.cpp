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

// some defines for throwing exceptions
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

// internal data
class QtSpeech::Private {
public:
    Private()
        :textScript(kCFStringEncodingMacRoman),onFinishSlot(0L) {}

    VoiceName name;
    SInt16 textScript;
    SpeechChannel channel;

    static const QString VoiceId;
    typedef QPointer<QtSpeech> Ptr;
    static QList<Ptr> ptrs;

    const char * onFinishSlot;
    QPointer<QObject> onFinishObj;
    void speechFinished();
    static void speechFinished(SpeechChannel, long refCon);
};
const QString QtSpeech::Private::VoiceId = QString("macosx:%1");
QList<QtSpeech::Private::Ptr> QtSpeech::Private::ptrs = QList<QtSpeech::Private::Ptr>();

// implementation
QtSpeech::QtSpeech(VoiceName n, QObject * parent)
    :QObject(parent), d(new Private)
{
    if (n.id.isEmpty()) {
        VoiceDescription info;
        SysCall( GetVoiceDescription(NULL, &info, sizeof(VoiceDescription)), InitError);
        n.name = QString::fromAscii((const char *)(info.name+1), int(info.name[0]));
        n.id = d->VoiceId.arg(info.voice.id);
    }

    if (n.id.isEmpty())
        throw InitError(Where+"No default voice in system");

    SInt16 count;
    VoiceSpec voice;
    VoiceSpec * voice_ptr(0L);
    SysCall( CountVoices(&count), InitError);
    for (int i=1; i<= count; i++) {
        SysCall( GetIndVoice(i, &voice), InitError);
        QString id = d->VoiceId.arg(voice.id);
        if (id == n.id) {
            voice_ptr = &voice;
            VoiceDescription info;
            SysCall( GetVoiceDescription(&voice, &info, sizeof(VoiceDescription)), InitError);
            d->textScript = info.script;
            break;
        }
    }

    SysCall( NewSpeechChannel(voice_ptr, &d->channel), InitError);
    SysCall( SetSpeechInfo(d->channel, soSpeechDoneCallBack,
                           (void *)NewSpeechDoneUPP(&Private::speechFinished)), InitError);
    d->name = n;
    d->ptrs << this;
}

QtSpeech::~QtSpeech()
{
    if (!d->channel)
        throw CloseError(Where+"No speech channel to close");

    SysCall( StopSpeech(d->channel), CloseError);
    SysCall( DisposeSpeechChannel(d->channel), CloseError);

    d->ptrs.removeAll(this);
    delete d;
}

const QtSpeech::VoiceName & QtSpeech::name() const {
    return d->name;
}

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
        VoiceName vname = { Private::VoiceId.arg(voice.id), name };
        vs << vname;
    }
    return vs;
}

void QtSpeech::tell(QString text) const {
    tell(text, 0L,0L);
}

void QtSpeech::tell(QString text, QObject * obj, const char * slot) const
{
    d->onFinishObj = obj;
    d->onFinishSlot = slot;
    if (obj && slot)
        connect(const_cast<QtSpeech *>(this), SIGNAL(finished()), obj, slot);

    CFStringRef cf_text = CFStringCreateWithCharacters(0,
                            reinterpret_cast<const UniChar *>(
                              text.unicode()), text.length());

    OSErr ok = SpeakCFString(d->channel, cf_text, NULL);
    CFRelease(cf_text);

    if (ok != noErr) throw LogicError(Where+"SpeakCFString()");
}

void QtSpeech::Private::speechFinished(SpeechChannel chan, long refCon)
{
    Q_UNUSED(refCon);
    foreach(QtSpeech * c, ptrs) {
        if (c && c->d->channel == chan) {
            if (c->d->onFinishObj && c->d->onFinishSlot) {
                c->finished();
                disconnect(c, SIGNAL(finished()),
                           c->d->onFinishObj, c->d->onFinishSlot);
                c->d->onFinishSlot = 0L;
                c->d->onFinishObj = 0L;
            }
            break;
        }
    }
}

} // namespace QtSpeech_v1
