#ifndef WINDOWSAUDIOSESSIONCONTROLLER_H
#define WINDOWSAUDIOSESSIONCONTROLLER_H

#include "iaudiosessioncontroller.h"
#include <QObject>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <comdef.h>
#include <vector>


class WindowsAudioSessionController : public IAudioSessionController
{
    Q_OBJECT

public:
    explicit WindowsAudioSessionController(QObject *parent = nullptr);
    ~WindowsAudioSessionController();

    /**
     * @copydoc IAudioSessionController::setVolumeForProcess
     */
    bool setVolumeForProcess(const AudioSession &s, float volume) override;  // Volume: 0.0 - 1.0

    /**
     * @copydoc IAudioSessionController::getVolumeForProcess
     */
    float getVolumeForProcess(const AudioSession &s) override;

    /**
     * @copydoc IAudioSessionController::muteProcess
     */
    bool muteProcess(const AudioSession &s, bool mute) override;

    /**
     * @copydoc IAudioSessionController::getActiveAudioSessions
     */
    std::vector<AudioSession> getActiveAudioSessions() override;

private:
    struct AudioSessionInfo : public AudioSession {
        IMMDevice *device;
    };
    bool getAudioSessionControl(DWORD processId, IAudioSessionControl **sessionControl);
    std::vector<AudioSessionInfo> getAudioSessions();
    IMMDevice* findDeviceSession(DWORD procID, const std::vector<AudioSessionInfo> &sessions);
};

#endif // WINDOWSAUDIOSESSIONCONTROLLER_H
