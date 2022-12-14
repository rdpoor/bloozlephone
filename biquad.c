/** @file biquad.c
    @brief Implementation of a biquad filter
    @author R.D.Poor  rdpoor@gmail.com
*/


#include "biquad.h"
#include <math.h>
#include <stddef.h>

// =============================================================================

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =============================================================================

/* initialize a BiQuad Filter */
biquad_t *biquad_init(biquad_t *b,
                      biquad_type_t type,
                      SAMPLE_T dbGain,
                      SAMPLE_T freq,
                      SAMPLE_T srate,
                      SAMPLE_T bandwidth) {
  SAMPLE_T A, omega, sn, cs, alpha, beta;
  SAMPLE_T a0, a1, a2, b0, b1, b2;

  /* setup variables */
  A = pow(10, dbGain / 40);
  omega = 2 * M_PI * freq / srate;
  sn = sin(omega);
  cs = cos(omega);
  alpha = sn * sinh(M_LN2 / 2 * bandwidth * omega / sn);
  beta = sqrt(A + A);

  switch (type) {
  case LPF:
    b0 = (1 - cs) / 2;
    b1 = 1 - cs;
    b2 = (1 - cs) / 2;
    a0 = 1 + alpha;
    a1 = -2 * cs;
    a2 = 1 - alpha;
    break;
  case HPF:
    b0 = (1 + cs) / 2;
    b1 = -(1 + cs);
    b2 = (1 + cs) / 2;
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
    a0 = 1 + (alpha / A);
    a1 = -2 * cs;
    a2 = 1 - (alpha / A);
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
    a1 = -2 * cs;
    a2 = 1 - alpha;
    break;
  default:
    return NULL;
  }

  /* normalize the coefficients by a0 (implied to be 1.0) */
  b->b0 = b0 / a0;
  b->b1 = b1 / a0;
  b->b2 = b2 / a0;
  b->a1 = a1 / a0;
  b->a2 = a2 / a0;

  /* zero initial samples */
  b->x1 = b->x2 = 0;
  b->y1 = b->y2 = 0;

  return b;
}

/* Computes a BiQuad filter on a sample */
SAMPLE_T biquad_step(biquad_t *b, SAMPLE_T sample) {
  SAMPLE_T result;

  /* compute result */
  result = b->b0 * sample + b->b1 * b->x1 + b->b2 * b->x2 - b->a1 * b->y1 -
           b->a2 * b->y2;

  /* shift x1 to x2, sample to x1 */
  b->x2 = b->x1;
  b->x1 = sample;

  /* shift y1 to y2, result to y1 */
  b->y2 = b->y1;
  b->y1 = result;

  return result;
}
