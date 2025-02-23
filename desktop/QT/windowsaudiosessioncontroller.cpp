#include "WindowsAudioSessionController.h"
#include <QDebug>
#include <propvarutil.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <Windows.h>
#include <QFileInfo>
#include <Psapi.h>


PROPERTYKEY key = { { 0x9F4C2855, 0x9F79, 0x4B39, { 0xA8, 0xD0, 0xE1, 0xD4, 0x62, 0x1E, 0xE8, 0x83 } }, 5 };
static const PROPERTYKEY PKEY_AppUserModel_RelaunchIconResource =
    { { 0x9F4C2855, 0x9F79, 0x4B39, { 0xA8, 0xD0, 0xE1, 0xD4, 0x62, 0x1E, 0xE8, 0x83 } }, 17 };


WindowsAudioSessionController::WindowsAudioSessionController(QObject *parent)
{
    CoInitialize(nullptr);
}

WindowsAudioSessionController::~WindowsAudioSessionController()
{
    CoUninitialize();
}

IMMDevice* WindowsAudioSessionController::findDeviceSession(DWORD procID, const std::vector<AudioSessionInfo> &session)
{
    for(const auto &s : session)
    {
        if(s.processId == procID) return s.device;
    }
    return nullptr;
}
bool WindowsAudioSessionController::getAudioSessionControl(DWORD processId, IAudioSessionControl **sessionControl)
{
    *sessionControl = nullptr;

    // Get the default audio device
    IMMDeviceEnumerator *deviceEnumerator = nullptr;
    IMMDevice *device = nullptr;
    IAudioSessionManager2 *sessionManager = nullptr;
    IAudioSessionEnumerator *sessionEnumerator = nullptr;

    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                __uuidof(IMMDeviceEnumerator), (void **)&deviceEnumerator)))
    {
        qWarning() << "Failed to create MMDeviceEnumerator";
        return false;
    }
    std::vector<AudioSessionInfo> sessions = getAudioSessions();
    device = findDeviceSession(processId, sessions);
    if(device == nullptr)
    {
        qWarning() << "Could not get matching endpoint";
        if (FAILED(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &device)))
        {
            qWarning() << "Failed to get default audio endpoint";
            deviceEnumerator->Release();
            return false;
        }
    }

    if (FAILED(device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void **)&sessionManager)))
    {
        qWarning() << "Failed to activate Audio Session Manager";
        device->Release();
        deviceEnumerator->Release();
        return false;
    }

    if (FAILED(sessionManager->GetSessionEnumerator(&sessionEnumerator)))
    {
        qWarning() << "Failed to get session enumerator";
        sessionManager->Release();
        device->Release();
        deviceEnumerator->Release();
        return false;
    }

    int sessionCount = 0;
    if (FAILED(sessionEnumerator->GetCount(&sessionCount)))
    {
        qWarning() << "Failed to get session count";
        sessionEnumerator->Release();
        sessionManager->Release();
        device->Release();
        deviceEnumerator->Release();
        return false;
    }

    for (int i = 0; i < sessionCount; i++)
    {
        IAudioSessionControl *session = nullptr;
        if (FAILED(sessionEnumerator->GetSession(i, &session)))
            continue;

        IAudioSessionControl2 *sessionControl2 = nullptr;
        if (SUCCEEDED(session->QueryInterface(__uuidof(IAudioSessionControl2), (void **)&sessionControl2)))
        {
            DWORD sessionPid = 0;
            if (SUCCEEDED(sessionControl2->GetProcessId(&sessionPid)))
            {
                if (sessionPid == processId)
                {
                    *sessionControl = sessionControl2;
                    sessionEnumerator->Release();
                    sessionManager->Release();
                    device->Release();
                    deviceEnumerator->Release();
                    return true;
                }
            }
            sessionControl2->Release();
        }
        session->Release();
    }

    sessionEnumerator->Release();
    sessionManager->Release();
    device->Release();
    deviceEnumerator->Release();

    return false;
}

bool WindowsAudioSessionController::setVolumeForProcess(const AudioSession &s, float volume)
{
    if (volume < 0.0f || volume > 1.0f)
    {
        qWarning() << "Volume must be between 0.0 and 1.0";
        return false;
    }

    IAudioSessionControl *sessionControl = nullptr;
    if (!getAudioSessionControl(s.processId, &sessionControl))
    {
        qWarning() << "Failed to find audio session for process";
        return false;
    }

    ISimpleAudioVolume *audioVolume = nullptr;
    if (FAILED(sessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void **)&audioVolume)))
    {
        qWarning() << "Failed to get SimpleAudioVolume";
        sessionControl->Release();
        return false;
    }

    if (FAILED(audioVolume->SetMasterVolume(volume, nullptr)))
    {
        qWarning() << "Failed to set volume";
        audioVolume->Release();
        sessionControl->Release();
        return false;
    }

    audioVolume->Release();
    sessionControl->Release();
    return true;
}

float WindowsAudioSessionController::getVolumeForProcess(const AudioSession &s)
{
    IAudioSessionControl *sessionControl = nullptr;
    if (!getAudioSessionControl(s.processId, &sessionControl))
    {
        qWarning() << "Failed to find audio session for process";
        return -1.0f;
    }

    ISimpleAudioVolume *audioVolume = nullptr;
    float volume = -1.0f;
    if (SUCCEEDED(sessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void **)&audioVolume)))
    {
        audioVolume->GetMasterVolume(&volume);
        audioVolume->Release();
    }

    sessionControl->Release();
    return volume;
}

bool WindowsAudioSessionController::muteProcess(const AudioSession &s, bool mute)
{
    IAudioSessionControl *sessionControl = nullptr;
    if (!getAudioSessionControl(s.processId, &sessionControl))
    {
        qWarning() << "Failed to find audio session for process";
        return false;
    }

    ISimpleAudioVolume *audioVolume = nullptr;
    if (FAILED(sessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void **)&audioVolume)))
    {
        qWarning() << "Failed to get SimpleAudioVolume";
        sessionControl->Release();
        return false;
    }

    if (FAILED(audioVolume->SetMute(mute, nullptr)))
    {
        qWarning() << "Failed to set mute state";
        audioVolume->Release();
        sessionControl->Release();
        return false;
    }

    audioVolume->Release();
    sessionControl->Release();
    return true;
}

std::vector<AudioSession> WindowsAudioSessionController::getActiveAudioSessions()
{
    std::vector<AudioSession> sessions;

    IMMDeviceEnumerator *deviceEnumerator = nullptr;
    IMMDevice *device = nullptr;
    IAudioSessionManager2 *sessionManager = nullptr;
    IAudioSessionEnumerator *sessionEnumerator = nullptr;

    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                __uuidof(IMMDeviceEnumerator), (void **)&deviceEnumerator)))
    {
        qWarning() << "Failed to create MMDeviceEnumerator";
        return sessions;
    }

    IMMDeviceCollection *pCollection = nullptr;
    if(FAILED(deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection)))
    {
        qWarning() << "Failed to EnumAudioEndpoints";
        deviceEnumerator->Release();
        return sessions;
    }
    UINT count = 0;
    if(FAILED(pCollection->GetCount(&count)))
    {
        qWarning() << "Failed to EnumAudioEndpoints";
        deviceEnumerator->Release();
        return sessions;
    }
    for(ULONG i = 0; i < count; i++)
    {
    // if (FAILED(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &device)))
    // {
    //     qWarning() << "Failed to get default audio endpoint";
    //     deviceEnumerator->Release();
    //     return sessions;
    // }
        if(FAILED(pCollection->Item(i, &device)))
        {
            qWarning() << "Failed to EnumAudioEndpoints";
            deviceEnumerator->Release();
            return sessions;
        }

        if (FAILED(device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void **)&sessionManager)))
        { qWarning() << "Failed to activate Audio Session Manager";
            device->Release();
            deviceEnumerator->Release();
            return sessions;
        }

        if (FAILED(sessionManager->GetSessionEnumerator(&sessionEnumerator)))
        {
            qWarning() << "Failed to get session enumerator";
            sessionManager->Release();
            device->Release();
            deviceEnumerator->Release();
            return sessions;
        }

        int sessionCount = 0;
        if (FAILED(sessionEnumerator->GetCount(&sessionCount)))
        {
            qWarning() << "Failed to get session count";
            sessionEnumerator->Release();
            sessionManager->Release();
            device->Release();
            deviceEnumerator->Release();
            return sessions;
        }

        for (int i = 0; i < sessionCount; i++)
        {
            IAudioSessionControl *session = nullptr;
            if (FAILED(sessionEnumerator->GetSession(i, &session)))
                continue;

            IAudioSessionControl2 *sessionControl2 = nullptr;
            if (SUCCEEDED(session->QueryInterface(__uuidof(IAudioSessionControl2), (void **)&sessionControl2)))
            {
                DWORD sessionPid = 0;
                AudioSessionState state;
                if (SUCCEEDED(sessionControl2->GetState(&state)))
                {
                    if (state == AudioSessionStateActive || state == AudioSessionStateInactive)
                    {
                        if (SUCCEEDED(sessionControl2->GetProcessId(&sessionPid)) && sessionPid != 0)
                        {
                            AudioSessionInfo info;
                            info.processId = sessionPid;

                            // 🔹 Get process name
                            HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, sessionPid);
                            if (hProcess)
                            {
                                wchar_t processName[MAX_PATH];
                                DWORD size = MAX_PATH;
                                if (QueryFullProcessImageNameW(hProcess, 0, processName, &size))
                                {
                                    // Get just the base (stem) name of the exe
                                    info.friendlyName = QFileInfo(QString::fromWCharArray(processName)).baseName();
                                }
                                CloseHandle(hProcess);
                            }

                            sessions.push_back(info);
                        }
                    }
                }
                sessionControl2->Release();
            }
            session->Release();
        }
    }

    pCollection->Release();
    sessionEnumerator->Release();
    sessionManager->Release();
    device->Release();
    deviceEnumerator->Release();

    return sessions;
}

std::vector<WindowsAudioSessionController::AudioSessionInfo> WindowsAudioSessionController::getAudioSessions()
{
    std::vector<AudioSessionInfo> sessions;

    IMMDeviceEnumerator *deviceEnumerator = nullptr;
    IMMDevice *device = nullptr;
    IAudioSessionManager2 *sessionManager = nullptr;
    IAudioSessionEnumerator *sessionEnumerator = nullptr;

    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                __uuidof(IMMDeviceEnumerator), (void **)&deviceEnumerator)))
    {
        qWarning() << "Failed to create MMDeviceEnumerator";
        return sessions;
    }

    IMMDeviceCollection *pCollection = nullptr;
    if(FAILED(deviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection)))
    {
        qWarning() << "Failed to EnumAudioEndpoints";
        deviceEnumerator->Release();
        return sessions;
    }
    UINT count = 0;
    if(FAILED(pCollection->GetCount(&count)))
    {
        qWarning() << "Failed to EnumAudioEndpoints";
        deviceEnumerator->Release();
        return sessions;
    }
    for(ULONG i = 0; i < count; i++)
    {
        if(FAILED(pCollection->Item(i, &device)))
        {
            qWarning() << "Failed to EnumAudioEndpoints";
            deviceEnumerator->Release();
            return sessions;
        }

        if (FAILED(device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void **)&sessionManager)))
        { qWarning() << "Failed to activate Audio Session Manager";
            device->Release();
            deviceEnumerator->Release();
            return sessions;
        }

        if (FAILED(sessionManager->GetSessionEnumerator(&sessionEnumerator)))
        {
            qWarning() << "Failed to get session enumerator";
            sessionManager->Release();
            device->Release();
            deviceEnumerator->Release();
            return sessions;
        }

        int sessionCount = 0;
        if (FAILED(sessionEnumerator->GetCount(&sessionCount)))
        {
            qWarning() << "Failed to get session count";
            sessionEnumerator->Release();
            sessionManager->Release();
            device->Release();
            deviceEnumerator->Release();
            return sessions;
        }

        for (int i = 0; i < sessionCount; i++)
        {
            IAudioSessionControl *session = nullptr;
            if (FAILED(sessionEnumerator->GetSession(i, &session)))
                continue;

            IAudioSessionControl2 *sessionControl2 = nullptr;
            if (SUCCEEDED(session->QueryInterface(__uuidof(IAudioSessionControl2), (void **)&sessionControl2)))
            {
                DWORD sessionPid = 0;
                AudioSessionState state;
                if (SUCCEEDED(sessionControl2->GetState(&state)))
                {
                    // 🔥 Allow both Active and Inactive sessions
                    if (state == AudioSessionStateActive || state == AudioSessionStateInactive)
                    {
                        if (SUCCEEDED(sessionControl2->GetProcessId(&sessionPid)) && sessionPid != 0)
                        {
                            AudioSessionInfo info;
                            info.processId = sessionPid;
                            info.device = device;

                            // 🔹 Get process name
                            HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, sessionPid);
                            if (hProcess)
                            {
                                wchar_t processName[MAX_PATH];
                                DWORD size = MAX_PATH;
                                if (QueryFullProcessImageNameW(hProcess, 0, processName, &size))
                                {
                                    info.friendlyName = QString::fromWCharArray(processName);
                                }
                                CloseHandle(hProcess);
                            }

                            sessions.push_back(info);
                        }
                    }
                }
                sessionControl2->Release();
            }
            session->Release();
        }
    }

    pCollection->Release();
    sessionEnumerator->Release();
    sessionManager->Release();
    // caller manages freeing this memory
    // device->Release();
    deviceEnumerator->Release();

    return sessions;

}
