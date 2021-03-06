/*******************************************************************/
/***  FILE :     Scalar_arithmetic.c                             ***/
/***  AUTHOR:    David P Jacobs                                  ***/
/***  PROGRAMMER: Sekhar Muddana                                 ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      Scalar S_init()                                        ***/
/***      Scalar S_zero()                                        ***/
/***      Scalar S_one()                                         ***/
/***      Scalar S_minus1()                                      ***/
/***      Scalar S_minus()                                       ***/
/***      Scalar S_add()                                         ***/
/***      Scalar S_sub()                                         ***/
/***      Scalar S_div()                                         ***/
/***      Scalar S_mul()                                         ***/
/***      Scalar S_inv()                                         ***/
/***  PRIVATE ROUTINES:                                          ***/
/***      void Print_inv_table()                                 ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines dealing with Scalar      ***/
/***      Arithmatic.                                            ***/
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "Scalar_arithmetic.h"
#include "Build_defs.h"
#include "driver.h"

Scalar Prime;
Scalar Inverse_table[PRIME_BOUND];

void S_init(void)
{
    Prime = GetField();    /* Initialize the global variable Prime. */

/* Initialize the global table of inverses. */
    for (Scalar i=1; i<Prime; i++) {
        for (Scalar j=1; j<Prime; j++) {
            if ((i * j) % Prime == 1) {
                Inverse_table[i] = j;
                break;
            }
        }
    }
}
