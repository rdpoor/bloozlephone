""" Enumerate audio input and output devices """

import pyaudio
import pprint

class EnumerateAudio(object):

    def __init__(self):
        self.audio = pyaudio.PyAudio()

    def list_devices(self):
        for api_index in range(self.audio.get_host_api_count()):
            api_info = self.audio.get_host_api_info_by_index(api_index)
            print('=====================================================')
            pprint.pprint(api_info)
            self.list_api(api_info)

    def list_api(self, api_info):
        api_index = api_info.get('index')
        for device_index in range(api_info.get('deviceCount')):
            device_info = self.audio.get_device_info_by_host_api_device_index(api_index, device_index)
            pprint.pprint(device_info)

if __name__ == "__main__":
    en = EnumerateAudio()
    en.list_devices()
    en.audio.terminate();
