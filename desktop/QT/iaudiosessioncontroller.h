#ifndef IAUDIOSESSIONCONTROLLER_H
#define IAUDIOSESSIONCONTROLLER_H
#include <QObject>
#include <QString>

typedef unsigned long Pid;

/**
 * @struct AudioSession
 * @brief Represents the PID friendly name and icon path of the controlled audio session
*/
struct AudioSession {
    Pid processId;
    QString friendlyName;
    QString iconPath;
};

/**
 * @class IAudioSessionController
 * @brief This is an interface (abstract class) for implmenting audio session controllers
 */
class IAudioSessionController : public QObject
{
    Q_OBJECT

public:
    IAudioSessionController(QObject* parent = nullptr) {}
    virtual ~IAudioSessionController() = default;

    /**
     * @brief setVolumeForProcess
     * @param AudioSession to set the volume of
     * @param volume from 0.0 to 1.0
     * @return true on success
     */
    virtual bool setVolumeForProcess(const AudioSession &s, float volume) = 0;

    /**
     * @brief getVolumeForProcess
     * @param AudioSession to get the volume of
     * @return value from 0 to 1.0 representing the volume
     */
    virtual float getVolumeForProcess(const AudioSession &s) = 0;

    /**
     * @brief muteProcess
     * @param AudioSession to mute
     * @param mute
     * @return true on success
     */
    virtual bool muteProcess(const AudioSession &s, bool mute) = 0;

    /**
     * @brief getActiveAudioSessions
     * @return returns a vector containing the active audio sessions avaliable to control
     */
    virtual std::vector<AudioSession> getActiveAudioSessions() = 0;
};

#endif // IAUDIOSESSIONCONTROLLER_H
