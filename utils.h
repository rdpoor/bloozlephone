#ifndef _UTILS_H_
#define _UTILS_H_

#ifndef SAMPLE_T
#define SAMPLE_T float
#endif

/**
 * @brief convert (power) db to ratio: 10 db => 10x gain
 */
SAMPLE_T utils_db_to_gain(SAMPLE_T db);

/**
 * @brief convert ratio to (power) db: 10x gain => 10 db
 */
SAMPLE_T utils_gain_to_db(SAMPLE_T gain);

/**
 * @brief convert midi-style pitch to frequency: pitch 69 => 440Hz
 */
SAMPLE_T utils_pitch_to_hz(SAMPLE_T midi_pitch);

/**
 * @brief convert frequency to midi-style pitch: 440Hz => pitch 69
 */
SAMPLE_T utils_hz_to_pitch(SAMPLE_T hz);

/**
 * @brief Return -1 if sample is negative, +1 if positive, 0 otherwise.
 */
SAMPLE_T utils_signum(SAMPLE_T sample);

#endif // #define _UTILS_H_
