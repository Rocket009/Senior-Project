from pycaw.pycaw import (
                        AudioUtilities, 
                        IAudioEndpointVolume, 
                        AudioSession, 
                        ISimpleAudioVolume, 
                        AudioDevice,
                        IAudioSessionManager2,
                        IAudioSessionControl2
                        )
from ctypes import POINTER, cast
from comtypes import CLSCTX_ALL
import comtypes

class AudioController:
    def __init__(self, process_name = ""):
        self._process_name = process_name
        self._endpoint = AudioUtilities.GetSpeakers()

    def _get_sessions(self):
        o = self._endpoint.Activate(IAudioSessionManager2._iid_, comtypes.CLSCTX_ALL, None)
        mgr = o.QueryInterface(IAudioSessionManager2)
        if mgr is None:
            return None
        sess_enum = mgr.GetSessionEnumerator()
        count = sess_enum.GetCount()

        # enumerate over sessions
        sessions: list[AudioSession] = []
        for i in range(count):
            ctl = sess_enum.GetSession(i)
            if ctl is None:
                continue
            ctl2 = ctl.QueryInterface(IAudioSessionControl2)
            if ctl2 is not None:
                sessions.append(AudioSession(ctl2))
        return sessions


    def _get_simple_audio_vol_int(self):
        sessions = self._get_sessions()
        for session in sessions:
            interface = session.SimpleAudioVolume
            if session.Process and session.Process.name() == self._process_name:
                return interface
        raise Exception("Could not find process in endpoint")

    @property
    def process_name(self) -> str:
        return self._process_name

    @process_name.setter
    def set_process_name(self, process_name: str):
        if process_name in self.get_process_names():
            self._process_name = process_name
        else:
            raise ValueError("Process does not exist in endpoint")

    def set_endpoint_device(self, name: str):
        """Sets the endpoint to the Friendly Name of the matching endpoint"""
        for dev in AudioUtilities.GetAllDevices():
            if dev._dev is not None and dev.FriendlyName is not None:
                if name == dev.FriendlyName:
                    self._endpoint = dev._dev
                    return
        raise ValueError("Endpoint not found")

    def get_endpoints(self) -> list[str]:
        """Returns a list of the Friendly Names of all of the endpoints"""
        return [i.FriendlyName for i in AudioUtilities.GetAllDevices() if i.FriendlyName is not None]

    def mute(self):
        """Mutes the process"""
        intf = self._get_simple_audio_vol_int()
        intf.SetMute(1, None)

    def unmute(self):
        """Unmutes the process"""
        intf = self._get_simple_audio_vol_int()
        intf.SetMute(0, None)

    def set_volume(self, level: float):
        """Sets the volume of the process to a level between 0.0 and 1.0"""
        if level > 1.0 or level < 0.0:
            raise ValueError("Level must be between 0.0 and 1.0")
        intf = self._get_simple_audio_vol_int()
        intf.SetMasterVolume(level, None)

    def get_process_names(self) -> list[str]:
        """Returns a list of the process names for the endpoint"""
        sessions = self._get_sessions()
        return [i.Process.name() for i in sessions if i.Process is not None]

    @staticmethod
    def get_default_process_names() -> list[str]:
        """Returns a list of the process names for the default endpoint"""
        sessions = AudioUtilities.GetAllSessions()
        return [i.Process.name() for i in sessions if i.Process is not None]
