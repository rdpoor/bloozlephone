#ifndef _PLL_H_
#define _PLL_H_

#ifndef SAMPLE_T
#define SAMPLE_T float
#endif

/* pll state */
typedef struct {
  SAMPLE_T alpha;
  SAMPLE_T beta;
  SAMPLE_T phase_out;
  SAMPLE_T frequency_out;
} pll_t;

pll_t *pll_init(pll_t *pll, SAMPLE_T alpha; SAMPLE_T beta);

SAMPLE_T pll_step(pll_t *pll, SAMPLE_T sample);

#endif // #define _PLL_H_
