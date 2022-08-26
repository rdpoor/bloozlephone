/** @file paex_saw.c
    @ingroup examples_src
    @brief Pass mic in through biquad bandpass to speaker out
    @author R.D.Poor  rdpoor@gmail.com

cc -g -Wall -o paex_passreson paex_passreson.c -I../portaudio/include -lportaudio

*/

#include "portaudio.h"
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

// =============================================================================

#define SAMPLE_RATE (44100)

#ifndef M_LN2
#define M_LN2	   0.69314718055994530942
#endif

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

/* whatever sample type you want */
typedef float sample_t;

/* filter types */
typedef enum {
    LPF, /* low pass filter */
    HPF, /* High pass filter */
    BPF, /* band pass filter */
    NOTCH, /* Notch Filter */
    PEQ, /* Peaking band EQ filter */
    LSH, /* Low shelf filter */
    HSH, /* High shelf filter */
    APF    /* Allpass Filter */
} biquad_type_t;

static volatile bool s_quit_requested = false;

/* biquad state */
typedef struct {
    sample_t a0, a1, a2, a3, a4;
    sample_t x1, x2, y1, y2;
} biquad_t;

// =============================================================================

biquad_t s_biquad_l;
biquad_t s_biquad_r;

// =============================================================================

static biquad_t *biquad_init(biquad_t *b,
                             biquad_type_t type,   // filter type
                             sample_t dbGain,      // gain
                             sample_t freq,        // center frequency
                             sample_t srate,       // sample rate
                             sample_t bandwidth);  // bandwidth in octaves

static sample_t biquad_step(biquad_t *b, sample_t sample);


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

/* sets up a BiQuad Filter */
static biquad_t *biquad_init(biquad_t *b,
                             biquad_type_t type,
                             sample_t dbGain,
                             sample_t freq,
                             sample_t srate,
                             sample_t bandwidth) {
    sample_t A, omega, sn, cs, alpha, beta;
    sample_t a0, a1, a2, b0, b1, b2;

    /* setup variables */
    A = pow(10, dbGain /40);
    omega = 2 * M_PI * freq /srate;
    sn = sin(omega);
    cs = cos(omega);
    alpha = sn * sinh(M_LN2 /2 * bandwidth * omega /sn);
    beta = sqrt(A + A);

    switch (type) {
    case LPF:
        b0 = (1 - cs) /2;
        b1 = 1 - cs;
        b2 = (1 - cs) /2;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case HPF:
        b0 = (1 + cs) /2;
        b1 = -(1 + cs);
        b2 = (1 + cs) /2;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case BPF:
        b0 = alpha;
        b1 = 0;
        b2 = -alpha;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case NOTCH:
        b0 = 1;
        b1 = -2 * cs;
        b2 = 1;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case PEQ:
        b0 = 1 + (alpha * A);
        b1 = -2 * cs;
        b2 = 1 - (alpha * A);
        a0 = 1 + (alpha /A);
        a1 = -2 * cs;
        a2 = 1 - (alpha /A);
        break;
    case LSH:
        b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
        b1 = 2 * A * ((A - 1) - (A + 1) * cs);
        b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
        a0 = (A + 1) + (A - 1) * cs + beta * sn;
        a1 = -2 * ((A - 1) + (A + 1) * cs);
        a2 = (A + 1) + (A - 1) * cs - beta * sn;
        break;
    case HSH:
        b0 = A * ((A + 1) + (A - 1) * cs + beta * sn);
        b1 = -2 * A * ((A - 1) + (A + 1) * cs);
        b2 = A * ((A + 1) + (A - 1) * cs - beta * sn);
        a0 = (A + 1) - (A - 1) * cs + beta * sn;
        a1 = 2 * ((A - 1) - (A + 1) * cs);
        a2 = (A + 1) - (A - 1) * cs - beta * sn;
        break;
    case APF:
        b0 = 1 - alpha;
        b1 = -2 * cs;
        b2 = 1 + alpha;
        a0 = 1 + alpha;
        a1=  -2 * cs;
        a2 = 1 - alpha;
        break;
    default:
        return NULL;
    }

    /* precompute the coefficients */
    b->a0 = b0 /a0;
    b->a1 = b1 /a0;
    b->a2 = b2 /a0;
    b->a3 = a1 /a0;
    b->a4 = a2 /a0;

    /* zero initial samples */
    b->x1 = b->x2 = 0;
    b->y1 = b->y2 = 0;

    return b;
}

/* Computes a BiQuad filter on a sample */
static sample_t biquad_step(biquad_t *b, sample_t sample) {
    sample_t result;

    /* compute result */
    result = b->a0 * sample + b->a1 * b->x1 + b->a2 * b->x2 -
        b->a3 * b->y1 - b->a4 * b->y2;

    /* shift x1 to x2, sample to x1 */
    b->x2 = b->x1;
    b->x1 = sample;

    /* shift y1 to y2, result to y1 */
    b->y2 = b->y1;
    b->y1 = result;

    return result;
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
      *out++ = biquad_step(&s_biquad_l, *in++);  // left
      *out++ = biquad_step(&s_biquad_r, *in++);  // right
  }
  return 0;
}

static void intHandler(int dummy) {
    (void)dummy;
    s_quit_requested = true;
}
