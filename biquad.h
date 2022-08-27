#ifndef _BIQUAD_H_
#define _BIQUAD_H_

#ifndef SAMPLE_T
#define SAMPLE_T float
#endif

/* filter types */
typedef enum {
  LPF,   /* low pass filter */
  HPF,   /* High pass filter */
  BPF,   /* band pass filter */
  NOTCH, /* Notch Filter */
  PEQ,   /* Peaking band EQ filter */
  LSH,   /* Low shelf filter */
  HSH,   /* High shelf filter */
  APF    /* Allpass Filter */
} biquad_type_t;

/* biquad state */
typedef struct {
  SAMPLE_T a1, a2, b0, b1, b2; // coefficients
  SAMPLE_T x1, x2, y1, y2;     // delayed samples
} biquad_t;

biquad_t *biquad_init(biquad_t *b,
                      biquad_type_t type,  // filter type
                      SAMPLE_T dbGain,     // gain
                      SAMPLE_T freq,       // center frequency
                      SAMPLE_T srate,      // sample rate
                      SAMPLE_T bandwidth); // bandwidth in octaves

SAMPLE_T biquad_step(biquad_t *b, SAMPLE_T sample);

#endif // #define _BIQUAD_H_
