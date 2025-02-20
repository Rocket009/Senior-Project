#ifndef AUDIOSESSIONCONTROLLER_H
#define AUDIOSESSIONCONTROLLER_H

#include <QObject>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <comdef.h>
#include <vector>

struct AudioSessionInfo {
    DWORD processId;
    QString friendlyName;
    QString iconPath;
    IMMDevice *device;
};

class AudioSessionController : public QObject
{
    Q_OBJECT

public:
    explicit AudioSessionController(QObject *parent = nullptr);
    ~AudioSessionController();

    bool setVolumeForProcess(DWORD processId, float volume);  // Volume: 0.0 - 1.0
    float getVolumeForProcess(DWORD processId);
    bool muteProcess(DWORD processId, bool mute);

    std::vector<AudioSessionInfo> getActiveAudioSessions();  // New method

private:
    bool getAudioSessionControl(DWORD processId, IAudioSessionControl **sessionControl);
    std::vector<AudioSessionInfo> getAudioSessions();
    IMMDevice* findDeviceSession(DWORD procID, const std::vector<AudioSessionInfo> &sessions);
};

#endif // AUDIOSESSIONCONTROLLER_H
