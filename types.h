#ifndef TYPES_H
#define TYPES_H

#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <windows.h> /* BOOL */
#endif

/*
  Some rules about types:
  - do not use Ulong, these are not portable.
  - do not use the constants, UINT_MAX, INT_MAX and INT_MIN
  The following are the assumptions about the types:
  - size(Uint) >= 4
  - size(Sint) >= 4
  - size(Ushort) = 2
  - size(Sshort) = 2
  No other assumptions are to be made.
*/

/*
  This file contains some basic type definition.
*/

/** Unsigned char type. */
typedef unsigned char  Uchar;         

/** Unsigned short type */
typedef unsigned short Ushort;        

/** Unsigned int type. */
typedef unsigned long  Uint;

/** Signed int type. */          
typedef signed   long  Sint;          

/*
  The following is the central case distinction to accomodate
  code for 32 bit integers and 64 bit integers.
*/

#ifdef SIXTYFOURBITS

/** Base 2 logarithm of wordsize. */
#define LOGWORDSIZE    6            

/** Unsigned integer constant. */
#define UintConst(N)   (N##UL)  

#else
         
/** Base 2 logarithm of wordsize. */
#define LOGWORDSIZE   5              

/** Unsigned integer constant. */ 
#define UintConst(N)  (N##U)        

#endif


/** Type of unsigned integer in <i>printf</i>. */
typedef unsigned long Showuint;   

/** Type of signed integer in <i>printf</i>. */
typedef signed long Showsint;      

#ifndef BOOL

/** Boolean data type. */
#define BOOL unsigned char

#endif


#ifndef False

/** False boolean constant. */
#define False ((BOOL) 0)

#endif


#ifndef True

/** True boolean constant. */
#define True ((BOOL) 1)

#endif


#endif

