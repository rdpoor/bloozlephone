/** @file paex_saw.c
    @ingroup examples_src
    @brief Pass mic in through pll to speaker out
    @author R.D.Poor  rdpoor@gmail.com

This version uses the Yin pictch tracker, no actual PLL (yet)

cc -g -Wall -o paex_passpll2 paex_passpll2.c ../sinosc.c ../utils.c \
   ../Yin.c -I.. -I../portaudio/include -lportaudio
*/

#include "portaudio.h"
#include "sinosc.h"
#include "utils.h"
#include "Yin.h"
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

// =============================================================================

#define SAMPLE_RATE (44100)

#define SAMPLE_T float

#define FRAMES_PER_BUFFER 1024

// =============================================================================

static sinosc_t s_sinosc_l;  // DCO
static Yin s_yin;            // pitch tracker
static volatile bool s_quit_requested = false;

static float s_yin_buffer[FRAMES_PER_BUFFER/2];

static int16_t s_int_samples[FRAMES_PER_BUFFER];

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

  printf("PortAudio Test: microphone input through pitch tracker to speaker.\n");
  /* Initialize library before making any other calls. */
  err = Pa_Initialize();
  if (err != paNoError) {
    goto error;

  }

  sinosc_init(&s_sinosc_l, SAMPLE_RATE);
  Yin_init(&s_yin, FRAMES_PER_BUFFER, s_yin_buffer, SAMPLE_RATE);

  /* Open an audio I/O stream. */
  err = Pa_OpenDefaultStream(&stream,
                             1,          /* stereo input */
                             1,          /* stereo output */
                             paFloat32,  /* 32 bit floating point output */
                             SAMPLE_RATE,
                             FRAMES_PER_BUFFER,        /* frames per buffer */
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
  SAMPLE_T *in = (SAMPLE_T *)inputBuffer;
  SAMPLE_T *out = (SAMPLE_T *)outputBuffer;
  (void)userData;                      // suppress compiler warning
  unsigned int i;

  // ugly: convert float samples to int16_t samples for Yin_analyze().
  // Yin_analyze should have multiple flavors.
  for (int i=0; i<FRAMES_PER_BUFFER; i++) {
      s_int_samples[i] = in[i] * INT16_MAX;
  }
  Yin_analyze(&s_yin, s_int_samples, 1.0);
  float freq = Yin_getFrequency(&s_yin);
  // shift pitch make things interesting...
  float freq2 = utils_pitch_to_hz(utils_hz_to_pitch(freq) + 2.0);
  printf("%ld %8.2f => %8.2f %f, %8.2f %8.2f\n", framesPerBuffer, freq, freq2, Yin_getProbability(&s_yin),
    sinosc_prev(&s_sinosc_l), s_sinosc_l.theta);
  float amp = Yin_getProbability(&s_yin) > 0.8 ? 0.5 : 0.0;

  // printf("%ld\n", framesPerBuffer);
  for (i = 0; i < framesPerBuffer; i++) {
      // *out++ = s_int_samples[i] / (float)INT16_MAX;
      *out++ = sinosc_step(&s_sinosc_l, freq2, amp); // left
  }
  return 0;
}

static void intHandler(int dummy) {
    (void)dummy;
    s_quit_requested = true;
}
