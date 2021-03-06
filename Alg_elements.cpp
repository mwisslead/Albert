/*******************************************************************/
/***  FILE :     Alg_elements.c                                  ***/
/***  AUTHOR:    David P Jacobs                                  ***/
/***  PROGRAMMER:Sekhar Muddana                                  ***/
/***  MODIFIED: 9/93 - Trent Whiteley                            ***/
/***                   Changed basis_coef from array to ptr      ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      int *CreateAE()                                        ***/
/***      int InitAE()                                           ***/
/***      int DestroyAE()                                        ***/
/***      int ZeroOutAE()                                        ***/
/***      int IsZeroAE()                                         ***/
/***      int ScalarMultAE()                                     ***/
/***      int AssignAddAE()                                      ***/
/***      int AddAE()                                            ***/
/***      int MultAE()                                           ***/
/***      Alg_element *AllocAE()                                 ***/
/***      int AssignLeft()                                       ***/
/***      int AssignRight()                                      ***/
/***      int PrintAE()                                          ***/
/***  PRIVATE ROUTINES:                                          ***/
/***      int LeftTapAE()                                        ***/
/***      int CopyAE()                                           ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines dealing with Algebraic   ***/
/***      elements.                                              ***/
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Alg_elements.h"
#include "Build_defs.h"
#include "Memory_routines.h"
#include "Mult_table.h"
#include "Scalar_arithmetic.h"

using std::map;

static void clearZeros(Alg_element &p2);
static int LeftTapAE(Scalar x, Basis b, const Alg_element &p1, Alg_element &p2);
#if 1
void PrintAE(const Alg_element &p);
#endif

static void clearZeros(Alg_element &p) {
#if 0
    int ne = p.size();
    int nz = 0;
#endif
    map<Basis, Scalar>::iterator i;
    for(i = p.begin(); i != p.end();) {
      if(i->first == 0 || i->second == 0) {
        p.erase(i++);
#if 0
        nz++;
#endif
      } else {
        i++;
      } 
    }
#if 0
  if(nz > 0) {
      printf("%d of %d zero coef or basis terms removed\n", nz, ne);
  }
#endif
}

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     *p -- Alg_element.                                          */ 
/* RETURNS:                                                        */
/*     1 if all basis coef of Alg_element *p are zeroes.           */
/*     0 otherwise.                                                */
/*******************************************************************/ 
int IsZeroAE(const Alg_element &p)
{
    map<Basis, Scalar>::const_iterator i;
    for(i = p.begin(); i != p.end(); i++) {
      if(i->first != 0 && i->second != 0) return 0;
    }

  return 1;
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p -- Alg_element.                                          */ 
/* REQUIRES:                                                       */
/*     x -- Scalar to multiply *p with.                            */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Multiply Alg_element *p with scalar x.                      */
/*     *p = *p * x.                                                */
/*******************************************************************/ 
void ScalarMultAE(Scalar x, Alg_element &p)
{
    if (x == S_one()) {
    } else if (x == S_zero()) {
        p.clear();
    } else {
      map<Basis, Scalar>::iterator i;
      for(i = p.begin(); i != p.end(); i++) {
        i->second = S_mul(x, i->second);
      }
    }

    clearZeros(p);
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p2 -- Alg_element.                                         */ 
/* REQUIRES:                                                       */
/*     *p1 -- To be added to *p2.                                  */ 
/* RETURNS: None.                                                  */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Add Alg_element *p1 to *p2.                                 */ 
/*     *p2 = *p2 + *p1.                                            */
/* NOTE:                                                           */
/*     We have to scan p2 to find and store p2->first.             */
/*******************************************************************/ 
void AddAE(const Alg_element &p1, Alg_element &p2)
{
      map<Basis, Scalar>::const_iterator p1i = p1.begin();
      map<Basis, Scalar>::iterator p2i = p2.begin();
      
    while(p1i != p1.end()) {
      if(p2i == p2.end()) {  /* occurs if max_basis(p2) < max_basis(p1) */
        p2[p1i->first] = p1i->second;

        p1i++;
#if 0
      } else if(p2i->first == 0) {
        p2i->first = p1i->first;
        p2i->second = p1i->second;

        p1i++;
        p2i++;
#endif
      } else if(p2i->first == p1i->first) {
        p2i->second = S_add(p2i->second, p1i->second);

        p1i++;
        p2i++;
      } else if(p2i->first < p1i->first) {
        p2i++;
      } else if(p2i->first > p1i->first) {
        p2[p1i->first] = p1i->second;

        p1i++;
      }
    }

    clearZeros(p2);
}    

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p2 -- Alg_element.                                         */ 
/* REQUIRES:                                                       */
/*     x -- Coefficient of the term.                               */
/*     b -- Basis of the term.                                     */ 
/*     *p1 -- Alg_element used for multiplication.                 */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Multiply term with Alg_element *p1 and add to *p2.          */
/*         *p2 = xb.(*p1) + *p2                                    */
/*******************************************************************/ 
int LeftTapAE(Scalar x, Basis b, const Alg_element &p1, Alg_element &p2)
{
    int status = OK;

    const Scalar zero = S_zero();
    if (x != zero && !IsZeroAE(p1)) {
      map<Basis, Scalar>::const_iterator i;
      for(i = p1.begin(); i != p1.end(); i++) {
        if(i->second != zero) {
                if (status == OK)
                    status = Mult2basis(b, i->first, S_mul(x, i->second), p2); 
            }
      }
    }
    clearZeros(p2);

        return(status);
}
    
/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p3 -- Alg_element.                                         */ 
/* REQUIRES:                                                       */
/*     *p1,*p2 -- Alg_elements used for multiplication.            */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Multiply Alg_elements *p1,*p2 and add to *p3.               */
/*         *p3 = (*p1) * (*p2) + *p3                               */ 
/*******************************************************************/ 
int MultAE(const Alg_element &p1, const Alg_element &p2, Alg_element &p3)
{
    int status = OK;

    const Scalar zero = S_zero();
    if (!IsZeroAE(p1) && !IsZeroAE(p2)) {
      map<Basis, Scalar>::const_iterator p1i;
      for(p1i = p1.begin(); p1i != p1.end(); p1i++) {
        if(p1i->second != zero) {
                if (status == OK)
                    status = LeftTapAE(p1i->second, p1i->first, p2, p3); 
            }
        }
    }
    clearZeros(p3);

    return(status);
}

#if 0
/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     *p -- Alg_element to be printed.                            */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Print the Algebraic element *p.                             */
/*******************************************************************/ 
void PrintAE(const Alg_element &p)
{
    map<Basis, Scalar>::const_iterator pi;
    for(pi = p.begin(); pi != p.end(); pi++) {
         Basis basis = pi->first;
         Scalar coef = pi->second;
         if(coef != 0) {
             if (coef > 0)
                 printf(" + %d b[%d]", coef, basis);
             else
                 printf(" - %d b[%d]", coef, basis);
          }
    }
    printf("\n");
} 
#endif
