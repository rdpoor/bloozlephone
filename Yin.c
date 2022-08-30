#include "Yin.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* =============================================================================
PRIVATE (LOCAL) FUNCTIONS
============================================================================= */

/**
 * Step 1: Calculates the squared difference of the signal with a shifted
 * version of itself.
 * @param buffer Buffer of samples to process.
 *
 * This is the Yin algorithms tweak on autocorellation. Read
 * http://audition.ens.fr/adc/pdf/2002_JASA_YIN.pdf for more details on what is
 * in here and why it's done this way.
 */
static void Yin_difference(Yin *yin, int16_t *buffer) {
  size_t i;
  size_t tau;
  float delta;

  /* Calculate the difference for difference shift values (tau) for the half of
   * the samples */
  for (tau = 0; tau < yin->halfBufferSize; tau++) {

    /* Take the difference of the signal with a shifted version of itself, then
     * square it. (This is the Yin algorithm's tweak on autocorellation) */
    for (i = 0; i < yin->halfBufferSize; i++) {
      delta = buffer[i] - buffer[i + tau];
      yin->yinBuffer[tau] += delta * delta;
    }
  }
}

/**
 * Step 2: Calculate the cumulative mean on the normalised difference calculated
 * in step 1
 * @param yin #Yin structure with information about the signal
 *
 * This goes through the Yin autocorellation values and finds out roughly where
 * shift is which produced the smallest difference
 */
static void Yin_cumulativeMeanNormalizedDifference(Yin *yin) {
  int16_t tau;
  float runningSum = 0;
  yin->yinBuffer[0] = 1;

  /* Sum all the values in the autocorellation buffer and nomalise the result,
   * replacing
   * the value in the autocorellation buffer with a cumulative mean of the
   * normalised difference */
  for (tau = 1; tau < yin->halfBufferSize; tau++) {
    runningSum += yin->yinBuffer[tau];
    yin->yinBuffer[tau] *= tau / runningSum;
  }
}

/**
 * Step 3: Search through the normalised cumulative mean array and find values
 * that are over the threshold
 * @return Shift (tau) which caused the best approximate autocorellation. -1 if
 * no suitable value is found over the threshold.
 */
static int16_t Yin_absoluteThreshold(Yin *yin, float threshold) {
  int16_t tau;

  /* Search through the array of cumulative mean values, and look for ones that
   * are over the threshold The first two positions in yinBuffer are always so
   * start at the third (index 2) */
  for (tau = 2; tau < yin->halfBufferSize; tau++) {
    if (yin->yinBuffer[tau] < threshold) {
      while (tau + 1 < yin->halfBufferSize &&
             yin->yinBuffer[tau + 1] < yin->yinBuffer[tau]) {
        tau++;
      }
      /* found tau, exit loop and return
       * store the probability
       * From the YIN paper: The yin->threshold determines the list of
       * candidates admitted to the set, and can be interpreted as the
       * proportion of aperiodic power tolerated
       * within a periodic signal.
       *
       * Since we want the periodicity and and not aperiodicity:
       * periodicity = 1 - aperiodicity */
      yin->probability = 1 - yin->yinBuffer[tau];
      break;
    }
  }

  /* if no pitch found, tau => -1 */
  if (tau == yin->halfBufferSize || yin->yinBuffer[tau] >= threshold) {
    tau = -1;
    yin->probability = 0;
  }

  return tau;
}

/**
 * Step 5: Interpolate the shift value (tau) to improve the pitch estimate.
 * @param  yin         [description]
 * @param  tauEstimate [description]
 * @return             [description]
 *
 * The 'best' shift value for autocorellation is most likely not an interger
 * shift of the signal. As we only autocorellated using integer shifts we should
 * check that there isn't a better fractional shift value.
 */
static float Yin_parabolicInterpolation(Yin *yin, int16_t tauEstimate) {
  float betterTau;
  int16_t x0;
  int16_t x2;

  /* Calculate the first polynomial coeffcient based on the current estimate of
   * tau */
  if (tauEstimate < 1) {
    x0 = tauEstimate;
  } else {
    x0 = tauEstimate - 1;
  }

  /* Calculate the second polynomial coeffcient based on the current estimate of
   * tau */
  if (tauEstimate + 1 < yin->halfBufferSize) {
    x2 = tauEstimate + 1;
  } else {
    x2 = tauEstimate;
  }

  /* Algorithm to parabolically interpolate the shift value tau to find a better
   * estimate */
  if (x0 == tauEstimate) {
    if (yin->yinBuffer[tauEstimate] <= yin->yinBuffer[x2]) {
      betterTau = tauEstimate;
    } else {
      betterTau = x2;
    }
  } else if (x2 == tauEstimate) {
    if (yin->yinBuffer[tauEstimate] <= yin->yinBuffer[x0]) {
      betterTau = tauEstimate;
    } else {
      betterTau = x0;
    }
  } else {
    float s0, s1, s2;
    s0 = yin->yinBuffer[x0];
    s1 = yin->yinBuffer[tauEstimate];
    s2 = yin->yinBuffer[x2];
    // fixed AUBIO implementation, thanks to Karl Helgason:
    // (2.0f * s1 - s2 - s0) was incorrectly multiplied with -1
    betterTau = tauEstimate + (s2 - s0) / (2 * (2 * s1 - s2 - s0));
  }

  return betterTau;
}

/* =============================================================================
PUBLIC FUNCTIONS
============================================================================= */

void Yin_init(Yin *yin, size_t sampleCount, float *yinBuffer,
              float sampleRate) {
  /* Initialise the fields of the Yin structure passed in */
  yin->halfBufferSize = sampleCount / 2;
  yin->yinBuffer = yinBuffer;
  yin->sampleRate = sampleRate;
  yin->frequency = -1.0;
  yin->probability = 0.0;

  memset(yin->yinBuffer, 0, sizeof(yin->yinBuffer[0]) * yin->halfBufferSize);
}

void Yin_analyze(Yin *yin, int16_t *buffer, float threshold) {
  int16_t tauEstimate = -1;

  memset(yin->yinBuffer, 0, sizeof(yin->yinBuffer[0]) * yin->halfBufferSize);

  /* Frequency not yet known */
  yin->frequency = -1;

  /* Step 1: Calculates the squared difference of the signal with a shifted
   * version of itself. */
  Yin_difference(yin, buffer);

  /* Step 2: Calculate the cumulative mean on the normalised difference
   * calculated in step 1 */
  Yin_cumulativeMeanNormalizedDifference(yin);

  /* Step 3: Search through the normalised cumulative mean array and find values
   * that are over the threshold */
  tauEstimate = Yin_absoluteThreshold(yin, threshold);

  /* Step 5: Interpolate the shift value (tau) to improve the pitch estimate. */
  if (tauEstimate != -1) {
    yin->frequency =
        yin->sampleRate / Yin_parabolicInterpolation(yin, tauEstimate);
  }
}

float Yin_getFrequency(Yin *yin) { return yin->frequency; }

float Yin_getProbability(Yin *yin) { return yin->probability; }

/* =============================================================================
UNIT TESTING
============================================================================= */

/*
Compile with -DYIN_STANDALONE_TESTS, e.g.:

cc -g -Wall -DYIN_STANDALONE_TESTS -o Test_Yin Yin.c && ./Test_Yin && rm -f ./Test_Yin
*/
#ifdef YIN_STANDALONE_TESTS

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define ASSERT assert

#ifndef M_PI
#define M_PI 3.141592653589793238462643383
#endif

#define SAMPLE_RATE 44100

// test buffer is 0.25 seconds long
// #define NUM_SAMPLES (SAMPLE_RATE / 4)
#define NUM_SAMPLES 800

static bool approx_eq(float x, float y, float epsilon) {
  return (fabs(x - y) < epsilon);
}

static void reset_sample_buffer(int16_t *sample_buffer, size_t sample_count) {
  memset(sample_buffer, 0, sizeof(sample_buffer[0]) * sample_count);
}

/**
 * @brief Write amplitude * sin(wt + phase) into s_test_buf.
 *
 * Note: when adding multiple sine waves, total amplitude should be <= 1.0.
 */
static void mix_to_sample_buffer(int16_t *sample_buffer, size_t sample_count,
                                 float frequency, float amplitude,
                                 float initial_phase) {
  float amp = amplitude * INT16_MAX;
  float dtheta = frequency * 2.0 * M_PI / SAMPLE_RATE;
  float theta = initial_phase;
  for (int i = 0; i < sample_count; i++) {
    sample_buffer[i] += round(amp * sin(theta));
    theta += dtheta;
  }
}

static bool yin_test(int16_t *sample_buf, size_t sample_count, float threshold,
                     float expected_frequency, float min_probability) {
  Yin yin;
  float analysis_buf[sample_count / 2];

  Yin_init(&yin, sample_count, analysis_buf, SAMPLE_RATE);
  Yin_analyze(&yin, sample_buf, threshold);
  printf("thresh = %f, freq = %f, prob = %f\n", threshold,
         Yin_getFrequency(&yin), Yin_getProbability(&yin));
  // allow 1Hz error in frequency
  return approx_eq(Yin_getFrequency(&yin), expected_frequency, 1.0) &&
         (Yin_getProbability(&yin) > min_probability);
}

int main(void) {
  int16_t sample_buf[NUM_SAMPLES];

  // Test pure 220 sine
  reset_sample_buffer(sample_buf, NUM_SAMPLES);
  mix_to_sample_buffer(sample_buf, NUM_SAMPLES, 220.0, 1.0, 0.0);
  ASSERT(yin_test(sample_buf, NUM_SAMPLES, YIN_DEFAULT_THRESHOLD, 220.0, 0.9));

  // Test 220 + 440 + 660
  reset_sample_buffer(sample_buf, NUM_SAMPLES);
  mix_to_sample_buffer(sample_buf, NUM_SAMPLES, 220.0, 0.25, 0.0);
  mix_to_sample_buffer(sample_buf, NUM_SAMPLES, 440.0, 0.25, 0.0);
  mix_to_sample_buffer(sample_buf, NUM_SAMPLES, 660.0, 0.25, 0.0);
  ASSERT(yin_test(sample_buf, NUM_SAMPLES, YIN_DEFAULT_THRESHOLD, 220.0, 0.9));

  // Test 220 + 440 + 660 with phase offsets
  reset_sample_buffer(sample_buf, NUM_SAMPLES);
  mix_to_sample_buffer(sample_buf, NUM_SAMPLES, 220.0, 0.25, M_PI * 0.333);
  mix_to_sample_buffer(sample_buf, NUM_SAMPLES, 440.0, 0.25, M_PI * 0.667);
  mix_to_sample_buffer(sample_buf, NUM_SAMPLES, 660.0, 0.25, 0.0);
  ASSERT(yin_test(sample_buf, NUM_SAMPLES, YIN_DEFAULT_THRESHOLD, 220.0, 0.9));

  // fundamental absent
  reset_sample_buffer(sample_buf, NUM_SAMPLES);
  mix_to_sample_buffer(sample_buf, NUM_SAMPLES, 440.0, 0.25, M_PI * 0.667);
  mix_to_sample_buffer(sample_buf, NUM_SAMPLES, 660.0, 0.25, 0.0);
  ASSERT(yin_test(sample_buf, NUM_SAMPLES, YIN_DEFAULT_THRESHOLD, 220.0, 0.9));

  // test 110 hz
  reset_sample_buffer(sample_buf, NUM_SAMPLES);
  mix_to_sample_buffer(sample_buf, NUM_SAMPLES, 110.0, 1.0, 0.0);
  ASSERT(yin_test(sample_buf, NUM_SAMPLES, YIN_DEFAULT_THRESHOLD, 110.0, 0.9));

  return 0;
}

#endif /* #ifdef YIN_STANDALONE_TESTS */
