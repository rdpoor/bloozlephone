/** @file paex_saw.c
    @ingroup examples_src
    @brief Pass mic in through biquad bandpass to speaker out
    @author R.D.Poor  rdpoor@gmail.com

cc -g -Wall -o paex_passreson paex_passreson.c ../biquad.c \
   -I.. -I../portaudio/include -lportaudio

*/

#include "portaudio.h"
#include "biquad.h"
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

// =============================================================================

#define SAMPLE_RATE (44100)

static volatile bool s_quit_requested = false;

typedef biquad_sample_t sample_t;

// =============================================================================

biquad_t s_biquad_l;
biquad_t s_biquad_r;

// =============================================================================

static int patestCallback(const void *inputBuffer,
                          void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData);

static void intHandler(int dummy);

// =============================================================================

int main(void) {
  PaStream *stream;
  PaError err;

  printf("PortAudio Test: microphone input through biquad to speaker.\n");
  /* Initialize library before making any other calls. */
  err = Pa_Initialize();
  if (err != paNoError) {
    goto error;

  }

  // biquad_init(&s_biquad_l, APF, 10.0, 660, SAMPLE_RATE, 0.0001);
  // biquad_init(&s_biquad_r, APF, 10.0, 550, SAMPLE_RATE, 0.0001);
  biquad_init(&s_biquad_l, BPF, 10.0, 660, SAMPLE_RATE, 0.03);
  biquad_init(&s_biquad_r, BPF, 10.0, 550, SAMPLE_RATE, 0.03);

  /* Open an audio I/O stream. */
  err = Pa_OpenDefaultStream(&stream,
                             2,          /* stereo input */
                             2,          /* stereo output */
                             paFloat32,  /* 32 bit floating point output */
                             SAMPLE_RATE,
                             128,        /* frames per buffer */
                             patestCallback,
                             NULL);
  if (err != paNoError)
    goto error;

  err = Pa_StartStream(stream);
  if (err != paNoError)
    goto error;

  signal(SIGINT, intHandler);
  while(!s_quit_requested) {
    /* Sleep briefly before checking for quit. */
    Pa_Sleep(500);
  }

  err = Pa_StopStream(stream);
  if (err != paNoError)
    goto error;
  err = Pa_CloseStream(stream);
  if (err != paNoError)
    goto error;
  Pa_Terminate();
  printf("Test finished.\n");
  return err;
error:
  Pa_Terminate();
  fprintf(stderr, "An error occurred while using the portaudio stream\n");
  fprintf(stderr, "Error number: %d\n", err);
  fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
  return err;
}

// =============================================================================

static int patestCallback(const void *inputBuffer,
                          void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) {
  // cast void* samples to floats
  sample_t *in = (sample_t *)inputBuffer;
  sample_t *out = (sample_t *)outputBuffer;
  (void)userData;                      // suppress compiler warning
  unsigned int i;

  for (i = 0; i < framesPerBuffer; i++) {
      *out++ = biquad_step(&s_biquad_l, *in++);  // left
      *out++ = biquad_step(&s_biquad_r, *in++);  // right
  }
  return 0;
}

static void intHandler(int dummy) {
    (void)dummy;
    s_quit_requested = true;
}
