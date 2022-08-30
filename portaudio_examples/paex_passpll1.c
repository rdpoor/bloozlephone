/** @file paex_saw.c
    @ingroup examples_src
    @brief Pass mic in through pll to speaker out
    @author R.D.Poor  rdpoor@gmail.com

This version uses signum(signal) x signum(reference) for phase comparison.

cc -g -Wall -o paex_passpll1 paex_passpll1.c ../biquad.c ../sinosc.c ../utils.c \
   -I.. -I../portaudio/include -lportaudio

*/

#include "portaudio.h"
#include "biquad.h"
#include "sinosc.h"
#include "utils.h"
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

// =============================================================================

#define SAMPLE_RATE (44100)

#define SAMPLE_T float

typedef struct {
    SAMPLE_T filter_cutoff;
    SAMPLE_T filter_bandwidth;
    SAMPLE_T loop_gain;
} pll_params_t;

// =============================================================================

static biquad_t s_biquad_l;  // used for low pass filter
static sinosc_t s_sinosc_l;  // DCO
static volatile bool s_quit_requested = false;
static int s_params_idx = 0;
static SAMPLE_T s_loop_gain;
static int s_frame_count = 0;

static pll_params_t s_pll_params[] = {
    {20.0, 1.0, 0.0},
    {20.0, 1.0, 16.0},
    {20.0, 1.0, 32.0},
    {20.0, 1.0, 64.0},
    {20.0, 1.0, 128.0},
    {20.0, 1.0, 256.0},
    {20.0, 1.0, 512.0},
};

#define N_PARAMS (sizeof(s_pll_params)/sizeof(s_pll_params[0]))

// =============================================================================

static SAMPLE_T pll_step(SAMPLE_T sample, sinosc_t *sinosc, biquad_t *biquad);

static int patestCallback(const void *inputBuffer,
                          void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData);

static void intHandler(int dummy);

static void update_pll_params(void);

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

  biquad_init(&s_biquad_l, LPF, 0.0, 5, SAMPLE_RATE, 0.5);
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

__attribute__((unused))
static SAMPLE_T pll_step(SAMPLE_T sample, sinosc_t *sinosc, biquad_t *biquad) {
    if ((s_frame_count & 0x3ffff) == 0) {
        update_pll_params();
    }
    s_frame_count += 1;
    SAMPLE_T pd = utils_signum(sample) * utils_signum(sinosc_prev(sinosc));
    SAMPLE_T lp = biquad_step(biquad, pd);
    SAMPLE_T f0 = 261.626 * 1.5 + (lp * s_loop_gain);  // swag
    // printf("%f %f %f\n", pd, lp, f0);
    SAMPLE_T vo = sinosc_step(sinosc, f0, 1.0);
    return sample + (vo + 0.001);
}

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

  // printf("%ld\n", framesPerBuffer);
  for (i = 0; i < framesPerBuffer; i++) {
      // *out++ = *in++;
      *out++ = pll_step(*in++, &s_sinosc_l, &s_biquad_l); // left
      // *out++ = pll(*in++, &s_sinosc_r, &s_biquad_r); // right
  }
  return 0;
}

static void intHandler(int dummy) {
    (void)dummy;
    s_quit_requested = true;
}

__attribute__((unused))
static void update_pll_params(void) {
    if (s_params_idx == 0) {
        printf("# ==============================\n");
    }
    pll_params_t *params = &s_pll_params[s_params_idx];
    biquad_init(&s_biquad_l, LSH, 0.0, params->filter_cutoff, SAMPLE_RATE, params->filter_bandwidth);
    s_loop_gain = params->loop_gain;
    s_params_idx += 1;
    printf("%f, %f, %f\n", params->filter_cutoff, params->filter_bandwidth, params->loop_gain);
    if (s_params_idx >= N_PARAMS) {
        s_params_idx = 0;
    }
}
