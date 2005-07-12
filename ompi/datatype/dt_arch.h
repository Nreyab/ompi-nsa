/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 The Trustees of the University of Tennessee.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#ifndef DATATYPE_ARCH_H_HAS_BEEN_INCLUDED
#define DATATYPE_ARCH_H_HAS_BEEN_INCLUDED

#include "ompi_config.h"
#include <float.h>
#include <assert.h>
#include "include/constants.h"

/***************************************************
** This file tries to classify the most relevant
** plattforms regarding their data representation.
** Three aspects are important:
** - byte ordering (little or big endian)
** - intgere representation
** - floating point representation.

** In addition, don't forget about the C/Fortran problems.
**
*****************************************************/


/*****************************************************************
** Part 1: Integer representation.
**
** The following data types are considered relevant:
**
** short
** int
** long
** long long
** integer (fortran)
**
** The fortran integer is dismissed here, since there is no
** platform known to me, were fortran and C-integer do not match
**
** The following abbriviations are introduced:
**
** a) il32 (int long are 32 bits) (e.g. IA32 LINUX, SGI n32, SUN)
**
**    short:     16 (else it would appear in the name)
**    int:       32
**    long:      32
**    long long: 64
**
** b) il64 ( int long are 64 bits) (e.g. Cray T3E )
**    short:     32
**    int:       64
**    long:      64
**    long long: 64
**
** c) l64 (long are 64 bits) (e.g. SGI 64 IRIX, NEC SX5)
**
**     short:     16
**     int:       32
**     long:      64
**     long long: 64
**
***********************************************************************/

/*********************************************************************
**  Part 2: Floating point representation
**
**  The following datatypes are considered relevant
**
**   float
**   double
**   long double
**   real
**   double precision
**
**   Unfortunatly, here we have to take care, whether float and real,
**   respectively double and double precision do match...
**
**  a) fr32 (float and real are 32 bits) (e.g. SGI n32 and 64, SUN, NEC SX5,...)
**     float:       32
**     double:      64
**     long double: 128
**     real:        32
**     double prec.:64
**
**  a1) fr32ld96 (float and real 32, long double 96) (e.g. IA32 LINUX gcc/icc)
**     see a), except long double is 96
**
**  a2) fr32ld64  (e.g. IBM )
**     see a), except long double is 64
**
**  b) cray ( e.g. Cray T3E)
**     float:       32
**     double:      64
**     long double: 64
**     real:        64
**     double prec.:64
**
**
**  Problem: long double is really treated differently on every machine. Therefore,
**  we are storing besides the length of the long double also the length of the mantisee,
**  and the number of *relevant* bits in the exponent. Here are the values:
**
**  Architecture   sizeof(long double) mantisee  relevant bits for exp.
**
**  SGIn32/64:     128                 107       10
**  SUN(sparc):    128                 113       14
**  IA64:          128                 64        14
**  IA32:          96                  64        14
**  Alpha:         128                 113       14
**                 64                  53        10 (gcc)
**  IBM:           64                  53        10
**                (128                 106       10) (special flags required).
**  SX5:           128                 105       22
**
** We will not implement all of these routiens, but we consider them
** now when defining the header-settings
**
***********************************************************************/

/********************************************************************
**
** Classification of machines:
**
** IA32 LINUX: il32, fr32ld96, little endian
** SUN:        il32, fr32,     big endian
** SGI n32:    il32, fr32,     big endian
** SGI 64:     l64,  fr32,     big endian
** NEC SX5:    l64,  fr32      big endian
** Cray T3E:   il64, cray,     big endian
** Cray X1:    i32(+), fr32,   big endian
** IBM:        il32, fr32ld64, big endian
** ALPHA:      l64,  fr32,     little endian
** ITANIUM:    l64,  fr32,     little endian
**
**
** + sizeof ( long long ) not known
** ? alpha supports both, big and little endian
***********************************************************************/


/* Current conclusions:
** we need at the moment three settings:
** - big/little endian ?
** - is long 32 or 64 bits ?
** - is long double 64, 96 or 128 bits ?
** - no. of rel. bits in the exponent of a long double ( 10 or 14 )
** - no. of bits of the mantiss of a long double ( 53, 64, 105, 106, 107, 113 )
**
** To store this in a 32 bit integer, we use the following definition:
**
**     1        2        3        4
** 12345678 12345678 12345678 12345678
**
** 1. Byte:
**   bits 1 & 2: 00 (header) (to recognize the correct end)
**   bits 3 & 4: encoding: 00 = little, 01 = big
**   bits 5 & 6: reserved for later use. currently set to 00
**   bits 7 & 8: reserved for later use. currently set to 00
** 2. Byte:
**   bits 1 & 2: length of long: 00 = 32, 01 = 64
**   bits 3 & 4: lenght of long long (not used currently, set to 00).
**   bits 5 & 6: reserved for later use. currently set to 00
**   bits 7 & 8: reserved for later use. currently set to 00
** 3. Byte:
**   bits 1 & 2: length of long double: 00=64, 01=96,10 = 128
**   bits 3 & 4: no. of rel. bits in the exponent: 00 = 10, 01 = 14)
**   bits 5 - 7: no. of bits of mantisse ( 000 = 53,  001 = 64, 010 = 105,
**                                         011 = 106, 100 = 107,101 = 113 )
**   bit      8: intel or sparc representation of mantisse (0 = sparc,
**                                         1 = intel )
** 4. Byte:
**   bits 1 & 2: 11 (header) (to recognize the correct end)
**   bits 3 & 4: reserved for later use. currently set to 11
**   bits 5 & 6: reserved for later use. currently set to 11
**   bits 7 & 8: reserved for later use. currently set to 11
*/

/* These masks implement the specification above above */

#define OMPI_ARCH_HEADERMASK      0x03000000 /* set the fields for the header */
#define OMPI_ARCH_HEADERMASK2     0x00000003 /* other end, needed for checks */
#define OMPI_ARCH_UNUSEDMASK      0xfc000000 /* mark the unused fields */

#define OMPI_ARCH_ISBIGENDIAN     0x00000008
#define OMPI_ARCH_LONGIS64        0x00000200
#define OMPI_ARCH_LONGLONGISxx    0x0000F000  /* the size of the long long */
#define OMPI_ARCH_LONGDOUBLEIS96  0x00020000
#define OMPI_ARCH_LONGDOUBLEIS128 0x00010000

#define OMPI_ARCH_LDEXPSIZEIS15   0x00080000

#define OMPI_ARCH_LDMANTDIGIS64   0x00400000
#define OMPI_ARCH_LDMANTDIGIS105  0x00200000
#define OMPI_ARCH_LDMANTDIGIS106  0x00600000
#define OMPI_ARCH_LDMANTDIGIS107  0x00100000
#define OMPI_ARCH_LDMANTDIGIS113  0x00500000

#define OMPI_ARCH_LDISINTEL       0x00800000

int32_t ompi_arch_compute_local_id( uint32_t *var);

int32_t ompi_arch_checkmask ( uint32_t *var, uint32_t mask );
static inline int32_t ompi_arch_isbigendian ( void )
{
    const uint32_t value = 0x12345678;
    const char *ptr = (char*)&value;
    int x = 0;

    /* if( sizeof(int) == 8 ) x = 4; */
    if( ptr[x] == 0x12)  return 1; /* big endian, true */
    if( ptr[x] == 0x78 ) return 0; /* little endian, false */
    assert( 0 );  /* unknown architecture not little nor big endian */
    return -1;
}

/* we must find which representation of long double is used
 * intel or sparc. Both of them represent the long doubles using a close to
 * IEEE representation (seeeeeee..emmm...m) where the mantissa look like
 * 1.????. For the intel representaion the 1 is explicit, and for the sparc
 * the first one is implicit. If we take the number 2.0 the exponent is 1
 * and the mantissa is 1.0 (the sign of course should be 0). So if we check
 * for the first one in the binary representation of the number, we will
 * find the bit from the exponent, so the next one should be the begining
 * of the mantissa. If it's 1 then we have an intel representaion, if not
 * we have a sparc one. QED
 */
static inline int32_t ompi_arch_ldisintel( void )
{
    long double ld = 2.0;
    int i, j;
    uint32_t* pui = (uint32_t*) &ld;

    j = LDBL_MANT_DIG / 32;
    i = (LDBL_MANT_DIG % 32) - 1;
    if( ompi_arch_isbigendian() ) { /* big endian */
        j = (sizeof(long double) / sizeof(unsigned int)) - j;
        if( i < 0 ) {
            i = 31;
            j = j+1;
        }
    } else {
        if( i < 0 ) {
            i = 31;
            j = j-1;
        }
    }
    return (pui[j] & (1 << i) ? 1 : 0);
}

static inline void ompi_arch_setmask ( uint32_t *var, uint32_t mask)
{
    *var |= mask;
}

static inline void ompi_arch_setabit ( uint32_t* var, int32_t pos )
{
    assert( ((uint32_t)pos) <= (sizeof(uint32_t) * 8) );
    ompi_arch_setmask( var, (((uint32_t)1) << (pos - 1)) );
}

static inline void ompi_arch_create_empty_id( uint32_t* id )
{
    *id = (OMPI_ARCH_HEADERMASK | OMPI_ARCH_UNUSEDMASK);
}

#endif  /* DATATYPE_ARCH_H_HAS_BEEN_INCLUDED */

