#include "gammaTable.h"
#include "gammaFunc.h"
#include "debug.h"
#include "reset.h"
#include <math.h> /* for log */

#ifdef DEBUG

/** Number of gamma function evaluations resolved by table lookup. */
static int gammaHits=0;

/** Number of gamma function evaluations calculated explicitly. */
static int gammaMisses=0;

#endif

/** Ln of PI/2 */
#define LNPI_2 M_LNPI / 2

/** Log2 of the alphabet size. */
static double alphasizeLog = 0;

/** Binary entropy of 1/alphasize. */
static double alphaEntropy = 0;


/**
 * Calculates LN(n!) If offset != 0 calculates LN((n + offset)!) defined as LN((n + offset) * 
 *  (n - 1 + offset) * ... * (1 + offset))
 * @param[in] n the argument
 * @param[in] offset the offset to add to the argument in each step
 */
static double lnFactorial (int n, double offset) {
  int i;
  double fact = log(1 + offset);

  for (i=n; i>1; i--) {
    fact += log(i + offset);
  }

  return fact;
}


/**
 * Calculates Ln (gamma(n)). 
 * @param[in] n the argument.
 * @returns Ln (gamma(n)).
 */
static double evalLogGamma(int n)
{
  int index = n-2;

  if (index >= 0 && index < TBL_SIZE) {
    DEBUGCODE(gammaHits++);
    return logGammaTbl[index];
  }
  else if (n == 1) {
    DEBUGCODE(gammaHits++);
    return 0;
  }
  else {
    DEBUGCODE(gammaMisses++);
    return gsl_sf_lngamma(n);
  }
}


/**
 * Calculates Ln (gamma(n + 1/2)).
 * @param[in] n the argument.
 * @returns Ln (gamma(n + 1/2)).
 */
static double evalLogGamma2(int n)
{
  int index = n-1;

  if (index >= 0 && index < TBL_SIZE) {
    DEBUGCODE(gammaHits++);
    return logGammaTbl2[index];
  }
  else if (n == 0) {
    DEBUGCODE(gammaHits++);
    return LNPI_2; /* Log (gamma (1/2)) */
  }
  else {
    DEBUGCODE(gammaMisses++);
    return gsl_sf_lngamma(n + 0.5);
  }
}


/**
 * @param[in] stats The statistics needed to calculate the probability assignment.
 * @returns KT probability assignment.
 */
double kt (statistics_t stats) {
  Uint i, ns = 0, alpha_2 = alphasize / 2; /* integer division */
  double sum = 0, gamma1, gamma2, gamma3;
  
  for(i=0; i<alphasize; i++) {
    sum += evalLogGamma2(stats->count[i]); 
    ns += stats->count[i];
  }

  if (alphasize % 2 == 0) {
    gamma1 = evalLogGamma(ns + alpha_2);
    gamma2 = alpha_2 * M_LNPI;
    gamma3 = evalLogGamma(alpha_2);
  }
  else {
    gamma1 = evalLogGamma2(ns + alpha_2);
    gamma2 = (alpha_2 + 0.5) * M_LNPI;
    gamma3 = evalLogGamma2(alpha_2);
  }

  return (gamma1 + gamma2 - gamma3 - sum) / M_LN2;
}

double howard (statistics_t stats) {
  Uint ns = 0, i;
  double sum = 0;
  double loghalf = -0.6931471805599452862;
  double x;

  for (i=0; i<stats->symbolCount; i++) {
    ns += stats->count[stats->symbols[i]];
    if (stats->count[stats->symbols[i]] > 0) {
      sum += evalLogGamma2(stats->count[stats->symbols[i]]-1);
    }
  }
  x = (evalLogGamma(ns) - ((stats->symbolCount - 1) * loghalf) - evalLogGamma(stats->symbolCount) - sum + (stats->symbolCount * 0.5 * M_LNPI)) / M_LN2;
  return x;
}

double deckard (statistics_t stats) {
  Uint i;
  double ns = 0;
  double sum = 0;
  double loghalf = -0.6931471805599452862;
  double x, sumTmp;
  double MAX_COUNT_D = MAX_COUNT;

  for (i=0; i<stats->symbolCount; i++) {
    ns += stats->count[stats->symbols[i]];
    if (stats->count[stats->symbols[i]] > 0) {
      sumTmp = evalLogGamma(MAX_COUNT+2) + (MAX_COUNT_D / 2) + (evalLogGamma(MAX_COUNT+1) / 2);
      sum += stats->count[stats->symbols[i]] / (MAX_COUNT_D+1) * sumTmp;
    }
  }
  x = (-(ns / (MAX_COUNT_D+1)) * (evalLogGamma((2*MAX_COUNT_D) + 1) - ((MAX_COUNT+1) * loghalf) - evalLogGamma(MAX_COUNT)) - evalLogGamma(MAX_COUNT) - ((MAX_COUNT-1) * loghalf) +
    (ns / 2) * M_LNPI - ns * (evalLogGamma(MAX_COUNT + 1) / 2) + ((stats->symbolCount - 1) / ((MAX_COUNT_D / 2) - 1)) * (evalLogGamma(MAX_COUNT+1) - evalLogGamma(MAX_COUNT_D / 2)) - 
    evalLogGamma(MAX_COUNT_D / 2) - sum) / M_LN2;
  return x;
}

double aux (statistics_t stats, Uint * distinct) {
  Uint ns = 0, i;
  double sum = 0;
  double loghalf = -0.6931471805599452862;

  for (i=0; i<stats->symbolCount; i++) {
    ns += distinct[stats->symbols[i]];
    if (distinct[stats->symbols[i]] > 0) {
      sum += evalLogGamma2(distinct[stats->symbols[i]]-1);
    }
  }
  return (evalLogGamma(ns) - ((stats->symbolCount - 1) * loghalf) - evalLogGamma(stats->symbolCount) - sum + (stats->symbolCount * 0.5 * M_LNPI)) / M_LN2;
}

/**
 * @returns log2 of the alphabet size. 
 */
double log2Alpha () {
  if (alphasizeLog == 0) {
    alphasizeLog = gsl_sf_log(alphasize) / M_LN2;
  }
  return alphasizeLog;
}


/** 
 * @returns binary entropy of 1/alphasize. 
 */
double hAlpha () {
  if (alphaEntropy == 0) {
    double invAlpha = (double)1 / alphasize;
    alphaEntropy = (invAlpha * log2Alpha()) - ((1 - invAlpha) * gsl_sf_log(1 - invAlpha) / M_LN2);
    DEBUGCODE(printf(">>>> %e\n", alphaEntropy));
  }
  return alphaEntropy;
}

#ifdef DEBUG

/**
 * @returns number of table lookup evaluations.
 */
int getHits() {
  return gammaHits;
}


/**
 * @returns number of explicit evaluations. 
 */
int getMisses() {
  return gammaMisses;
}

#endif
