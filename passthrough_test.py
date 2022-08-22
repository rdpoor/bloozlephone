"""Stream microphone to the speaker on the fly"""

# On reducing latency:
# https://wiki.linuxaudio.org/wiki/raspberrypi#on-board_audio

import pyaudio
import time
import array
import math

SAMPLE_FORMAT = pyaudio.paInt16  # samples are 16 bit signed
CHANNEL_COUNT = 1                # number of channels
FRAME_RATE = 48000               # samples per second

# FRAMES_PER_BUFFER = 256
FRAMES_PER_BUFFER = 128
SAMPLES_PER_FRAME = 1
BYTES_PER_SAMPLE = 2
BUFFER_COUNT = 8

class Streamer(object):

    def __init__(self, input_device_index=None, output_device_index=None):
        # Allocate the PyAudio object
        self.audio = pyaudio.PyAudio()

        self.buffer_count = 0
        self.i_idx = 0  # istream index into buffers[]
        self.o_idx = 0  # ostream index into buffers[]

        # self.buffers will be a single-producer, single-consumer circular FIFO
        # of sample data between mic stream process and speaker stream process.
        self.buffers = [None] * BUFFER_COUNT

        # trampoline function for callback in order to get access to self...
        def ostream_callback(in_data, frame_count, time_info, status):
            return self.ostream_cb(in_data, frame_count, time_info, status);

        self.istream = self.audio.open(
            format=SAMPLE_FORMAT,
            channels=CHANNEL_COUNT,
            rate=FRAME_RATE,
            input=True,
            start=False,  # wait for self.start()
            input_device_index = input_device_index,
            frames_per_buffer=FRAMES_PER_BUFFER,
        )
        self.ostream = self.audio.open(
            format=SAMPLE_FORMAT,
            channels=CHANNEL_COUNT,
            rate=FRAME_RATE,
            output=True,
            start=False,  # wait until we have acquired mic buffer(s)
            output_device_index = output_device_index,
            frames_per_buffer=FRAMES_PER_BUFFER,
            stream_callback=ostream_callback
        )

    def ostream_cb(self, in_data, frame_count, time_info, status):
        # fetch the next buffer of sample data from the FIFO and pass it to the
        # output stream.
        # print("{} O: [{}] {}".format(time.perf_counter_ns(),
        #                              self.o_idx,
        #                              frame_count),
        #                              flush=True)
        data = self.buffers[self.o_idx]
        self.o_idx = (self.o_idx + 1) % BUFFER_COUNT
        samples = array.array('h', data)      # convert to int16_t
        # perform any processing on samples here...
        # make a copy of the sample data before sending to the driver
        return (bytes(samples), pyaudio.paContinue)

    def start(self):
        self.istream.start_stream()

    def shutdown(self):
        self.istream.close()
        self.ostream.close()
        self.audio.terminate();

if __name__ == "__main__":
    s = Streamer(input_device_index=1, output_device_index=1)
    try:
        s.start()        # start the microphone reading stream
        while s.istream.is_active():
            data = s.istream.read(FRAMES_PER_BUFFER)
            # s.buffers[s.i_idx] = bytes(data)  # make a copy
            s.buffers[s.i_idx] = data
            # print("{} I: [{}] {}".format(time.perf_counter_ns(),
            #                              s.i_idx,
            #                              FRAMES_PER_BUFFER),
            #                              flush=True)
            s.i_idx = (s.i_idx + 1) % BUFFER_COUNT
            # If the input stream has filled some elements of the FIFO, it is
            # time to start the output stream.
            if (s.ostream.is_active() == False) and (s.i_idx == 3):
                print("starting output stream", flush=True)
                s.ostream.start_stream()

            # print('data: {} {} {}'.format(
            #     time.perf_counter_ns(), type(data), len(data)), flush=True)
    finally:
        s.shutdown()
