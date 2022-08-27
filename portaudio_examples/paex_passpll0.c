/** @file paex_saw.c
    @ingroup examples_src
    @brief Pass mic in through pll to speaker out
    @author R.D.Poor  rdpoor@gmail.com

cc -g -Wall -o paex_passpll0 paex_passpll0.c ../biquad.c ../sinosc.c \
   -I.. -I../portaudio/include -lportaudio

*/

#include "portaudio.h"
#include "biquad.h"
#include "sinosc.h"
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

// =============================================================================

#define SAMPLE_RATE (44100)

static volatile bool s_quit_requested = false;

// =============================================================================

biquad_t s_biquad_l;
sinosc_t s_sinosc_l;

// =============================================================================

static sample_t pll_step(sample_t sample, sinosc_t *sinosc, biquad_t *biquad);

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

  printf("PortAudio Test: microphone input through pll to speaker.\n");
  /* Initialize library before making any other calls. */
  err = Pa_Initialize();
  if (err != paNoError) {
    goto error;

  }

  biquad_init(&s_biquad_l, LPF, 0.0, 100, SAMPLE_RATE, 0.5);
  sinosc_init(&s_sinosc_l, SAMPLE_RATE);

  /* Open an audio I/O stream. */
  err = Pa_OpenDefaultStream(&stream,
                             1,          /* stereo input */
                             1,          /* stereo output */
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

static sample_t pll(sample_t sample, sinosc_t *sinosc, biquad_t *biquad) {
    sample_t pd = sample * sinosc_prev(sinosc) * s_loop_gain;
    sample_t lp = biquad_step(biquad, pd);
    sample_t f0 = 220.0 + lp * 10000.0;  // swag
    sample_t vo = sinosc_step(sinosc, f0, 1.0);
    return vo;
}

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
      *out++ = pll(*in++, &s_sinosc_l, &s_biquad_l); // left
      // *out++ = pll(*in++, &s_sinosc_r, &s_biquad_r); // right
  }
  return 0;
}

static void intHandler(int dummy) {
    (void)dummy;
    s_quit_requested = true;
}
