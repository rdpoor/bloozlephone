#ifndef Yin_h
#define Yin_h

#include <stddef.h>
#include <stdint.h>


#define YIN_DEFAULT_THRESHOLD 0.15

/**
 * @struct  Yin
 * @brief	Object to encapsulate the parameters for the Yin pitch detection
 * algorithm
 */
typedef struct _Yin {
  size_t halfBufferSize; /**< Half the buffer length */
  float *yinBuffer; /**< intermediate storage for analysis */
  float sampleRate;
  float frequency;
  float probability; /**< probability that the pitch found is correct */
} Yin;

/**
 * Initialise the Yin pitch detection object
 * @param yin Yin pitch detection object to initialise
 * @param sampleCount Length of the audio buffer to analyse
 * @param yinBuffer User-allocated buffer of float[sampleCount/2]
 * @param sampleRate Sampling rate (in Hz)
 *
 * NOTE: the autocorrelation inner loop is O(N^2) (albeit with low K), but you
 * should try to keep the sampleCount to the smallest workable value if speed is
 * a concern.
 */
void Yin_init(Yin *yin, size_t sampleCount, float *yinBuffer, float sampleRate);

/**
 * Runs the Yin pitch detection algortihm
 * @param  yin Initialised Yin object
 * @param  sampleBuffer Buffer of samples to analyse (MUST be sampleCount)
 * @param threshold  Allowed uncertainty (e.g 0.05 will return a pitch with ~95%
 * probability)
 */
void Yin_analyze(Yin *yin, int16_t *sampleBuffer, float threshold);

/**
 * return pitch deteced in most recent call to Yin_analyze()
 * @param  yin Yin object previouly passed to Yin_analyze()
 * return Detected frequency (or -1 if not detected)
 */
float Yin_getFrequency(Yin *yin);

/**
 * Certainty of the pitch found
 * @param  yin Yin object previouly passed to Yin_analyze()
 * @return Certainty of the detected frequency (0.0-1.0) or 0.0 if not detected.
 */
float Yin_getProbability(Yin *yin);

#endif
