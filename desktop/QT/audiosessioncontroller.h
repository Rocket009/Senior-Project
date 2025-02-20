#ifndef AUDIOSESSIONCONTROLLER_H
#define AUDIOSESSIONCONTROLLER_H

#include <QObject>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <comdef.h>
#include <vector>

struct AudioSession {
    DWORD processId;
    QString friendlyName;
    QString iconPath;
};

struct AudioSessionInfo : public AudioSession {
    IMMDevice *device;
};


class AudioSessionController : public QObject
{
    Q_OBJECT

public:
    explicit AudioSessionController(QObject *parent = nullptr);
    ~AudioSessionController();

    bool setVolumeForProcess(const AudioSession &s, float volume);  // Volume: 0.0 - 1.0
    float getVolumeForProcess(const AudioSession &s);
    bool muteProcess(const AudioSession &s, bool mute);

    std::vector<AudioSession> getActiveAudioSessions();  // New method

private:
    bool getAudioSessionControl(DWORD processId, IAudioSessionControl **sessionControl);
    std::vector<AudioSessionInfo> getAudioSessions();
    IMMDevice* findDeviceSession(DWORD procID, const std::vector<AudioSessionInfo> &sessions);
};

#endif // AUDIOSESSIONCONTROLLER_H
