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
 
#ifndef STATISTICS_H
#define STATISTICS_H

#include "types.h"

/** Structure that saves statistics for each tree node in the encoder. */
typedef struct statistics {
  Uint *count; /**< List with the number of occurrences of each character in this state. */
  Uchar *symbols; /**< List of occuring symbols */
  Uint symbolCount; /**< Number of occuring symbols */
  double cost; /**< Cost assigned to this subtree. */
} *statistics_t;

/** Creates and initializes a new statistics structure instance. */
statistics_t allocStatistics();

/** Deletes a statistics structure instance. */
void freeStatistics(statistics_t);

/** Returns a new statistics structure, can be created or obtained from the buffer */
statistics_t getStatistics();

/** Puts a statistics structure that is no longer used into the buffer */
void returnStatistics(statistics_t);

/** Deletes all statistics stored in the buffer */
void freeBuffer();

#endif

