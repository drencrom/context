#ifndef GAMMA_FUNC_H
#define GAMMA_FUNC_H

#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_log.h>
#include <gsl/gsl_math.h>
#include "statistics.h"
#include "alpha.h"

/** Calculates the Krichevsky-Trofimov probability assignment. */
double kt(statistics_t);

/** Calculates an aproximation of the Howard probability. */
double howard(statistics_t);

double aux (statistics_t, Uint *);

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
