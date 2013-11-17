/* Copyright 2013 Jorge Merlino

   This file is part of Context.

   Context is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Context is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Context.  If not, see <http://www.gnu.org/licenses/>.
*/
 
#ifndef GAMMA_FUNC_H
#define GAMMA_FUNC_H

#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_log.h>
#include <gsl/gsl_math.h>
#include "statistics.h"
#include "alpha.h"

/** Calculates the Krichevsky-Trofimov probability assignment. */
double kt(statistics_t);

/** Calculates the cost of a node. */
double nodeCost(statistics_t);

/** Calculates the cost of escapes in a node*/
double escapeCost(statistics_t, Uint *);

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
