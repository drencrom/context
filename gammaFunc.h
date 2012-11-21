#ifndef GAMMA_FUNC_H
#define GAMMA_FUNC_H

#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_log.h>
#include <gsl/gsl_math.h>
#include "statistics.h"
#include "alpha.h"

/** Calculates the Krichevsky-Trofimov probability assignment. */
double kt(statistics_t);

/** Calculates an approximation of the Moffat PPMC probability assignment. */
double moffat(statistics_t);
double moffat2(statistics_t);

/** Calculates an aproximation of the Howard probability. */
double howard(statistics_t);

double aux (statistics_t, Uint *);

double newstats(statistics_t);

/** Returns the log2 of the alphabet size. */
double log2Alpha (); 

/** Returns the binary entropy of 1/alphasize. */
double hAlpha (); 

#ifdef DEBUG

/** Returns the number of gamma function evaluations resolved by table lookup. */
int getHits();

/** Retruns the number of gamma function evaluations calculated explicitly. */
int getMisses();

#endif

#endif
