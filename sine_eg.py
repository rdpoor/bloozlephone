"""Generate a sine tone on the fly"""
import pyaudio
import time
import array
import math

SAMPLE_FORMAT = pyaudio.paInt16  # samples are 16 bit signed
CHANNEL_COUNT = 1                # number of channels
FRAME_RATE = 48000               # samples per second

FRAMES_PER_BUFFER = 256
SAMPLES_PER_FRAME = 1
BYTES_PER_SAMPLE = 2
BUFFER_COUNT = 4

SIN_FRQ = 220.0
SIN_AMP = 0.5

class Streamer(object):

    def __init__(self, input_device_index=None, output_device_index=None):
        # Allocate the PyAudio object
        self.audio = pyaudio.PyAudio()

        self.theta = 0.0
        self.dtheta = SIN_FRQ * 2 * math.pi / FRAME_RATE
        self.amplitude = 32767 * SIN_AMP

        # pre-allocate a buffer of int16_t
        self.buffer = \
            array.array('h', [0] * FRAMES_PER_BUFFER * SAMPLES_PER_FRAME)

        # trampoline function for callback in order to get access to self...
        def ostream_callback(in_data, frame_count, time_info, status):
            return self.ostream_cb(in_data, frame_count, time_info, status);

        self.ostream = self.audio.open(
            format=SAMPLE_FORMAT,
            channels=CHANNEL_COUNT,
            rate=FRAME_RATE,
            output=True,
            start=True,
            output_device_index = output_device_index,
            frames_per_buffer=FRAMES_PER_BUFFER,
            stream_callback=ostream_callback
        )

    def ostream_cb(self, in_data, frame_count, time_info, status):
        # generate sine tone in self.buffer
        for i in range(frame_count):
            self.buffer[i] = round(self.amplitude * math.sin(self.theta))
            self.theta += self.dtheta
        # convert to a read-only bytes() object for the audio driver
        return (bytes(self.buffer), pyaudio.paContinue)

    def shutdown(self):
        self.ostream.close()
        self.audio.terminate();

if __name__ == "__main__":
    s = Streamer()
    try:
        # sleep (mostly) in foreground.  all processing will happen in the
        # ostream_cb method in a separate thread.
        while s.ostream.is_active():
            time.sleep(0.1)
    finally:
        s.shutdown()
