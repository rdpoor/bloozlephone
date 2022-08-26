/** @file paex_saw.c
    @ingroup examples_src
    @brief Pass mic in directly to speaker out
    @author R.D.Poor  rdpoor@gmail.com

cc -g -Wall -o paex_passthrough paex_passthrough.c -I../portaudio/include -lportaudio

*/

#include "portaudio.h"
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#define SAMPLE_RATE (44100)

static volatile bool s_quit_requested = false;

static int patestCallback(const void *inputBuffer,
                          void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) {
  float *in = (float *)inputBuffer;
  float *out = (float *)outputBuffer;
  (void)userData;                      // suppress compiler warning
  unsigned int i;

  for (i = 0; i < framesPerBuffer; i++) {
    *out++ = *in++;  /* left */
    *out++ = *in++;  /* right */
  }
  return 0;
}

void intHandler(int dummy) {
    (void)dummy;
    s_quit_requested = true;
}

/*******************************************************************/
int main(void) {
  PaStream *stream;
  PaError err;

  printf("PortAudio Test: microphone input to speaker output.\n");
  /* Initialize library before making any other calls. */
  err = Pa_Initialize();
  if (err != paNoError)
    goto error;

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
