#ifndef IAUDIOSESSIONCAPTURE_H
#define IAUDIOSESSIONCAPTURE_H

#include <QObject>
#include "iaudiosessioncontroller.h"

typedef unsigned char u8;
typedef unsigned short u16;

enum AudioSessionErr
{
    BUFFER_OVERLOAD,
    PROCESS_CLOSED
};

struct AudioSegment
{
  //TODO
};

class IAudioSessionCapture : public QObject
{
    Q_OBJECT

public:
    IAudioSessionCapture(QObject *parent = nullptr);
    virtual ~IAudioSessionCapture() = default;
    virtual void startCapture(AudioSession s) = 0;
    virtual void stopCapture() = 0;
    virtual AudioSession getCurrentCapture() = 0;
signals:
    virtual void OnAudioSegment(const AudioSegment &s) = 0;
    virtual void OnError(AudioSessionErr err, AudioSession s) = 0;


};

#endif // IAUDIOSESSIONCAPTURE_H
