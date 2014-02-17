/*******************************************************************/
/***  FILE :        CreateMatrix.c                               ***/
/***  AUTHOR:       David P Jacobs                               ***/
/***  PROGRAMMER:   Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990.                                    ***/
/***  MODIFIED:     David Lee (August 1992)                      ***/
/***                   Added routines SparseCreateTheMatrix()    ***/
/***                and SparseFillTheMatrix() for sparse method  ***/
/***                support.                                     ***/
/***                9/93 - Trent Whiteley                        ***/
/***                changed Pair_present from a two-dimensional  ***/
/***                array to a ptr and added calls to get and    ***/
/***                values within it                             ***/
/***                                                             ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      int CreateMatrix()                                     ***/
/***      int SparseCreateTheMatrix()                            ***/
/***  PRIVATE ROUTINES:                                          ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines dealing with creating    ***/
/***      a Matrix for the given list of equations. (i.e list of ***/
/***      Basis pairs.                                           ***/ 
/***      1) Initialize PairPresent bit Matrix to 0's.           ***/
/***      2) Compute 'N', the number of distinct Basis pairs     ***/
/***         present in the given list of equations.             ***/
/***      3) malloc ColtoBP array of 'N' pairs w.o coefficients. ***/  
/***      4) Fill the ColtoBP array with the distinct Bais pairs.***/
/***         Use the PairPresent bit Matrix while filling.       ***/
/***      5) Allocate space for the Matrix of equations.         ***/
/***         It's number of rows is equal to number of equations.***/
/***         And number of columns equal to 'N'.                 ***/
/***         ZeroOut the Matrix.                                 ***/
/***         For each equation, one row of the Matrix is filled. ***/
/***         For each Basis pair in the equation, scan the array ***/
/***         to find the column number. It is nothing but the    ***/
/***         position where we could find the required Basis pair***/
/***         Enter the Scalar corresponding to the Basis pair in ***/
/***         the equaition into the Matrix in the corresponding  ***/
/***         column.                                             ***/ 
/*******************************************************************/

#include <algorithm>
#include <list>
#include <vector>

using std::fill;
using std::list;
using std::vector;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CreateMatrix.h"
#include "Basis_table.h"
#include "Build_defs.h"
#include "Memory_routines.h"
#include "Scalar_arithmetic.h"
#include "SparseReduceMatrix.h"
#include "Type_table.h"
#include "pair_present.h"

static void ZeroOutPairPresent(void);
static void FillPairPresent(const Eqn_list_node *Eq_list, int &Num_unique_basis_pairs, int &Num_equations);
static int CreateColtoBP(Name N, int Num_unique_basis_pairs, vector<Unique_basis_pair> &ColtoBP);
static int AreBasisElements(Degree d);
static void Process(vector<Unique_basis_pair> &ColtoBP, Degree d1, Degree d2, int *col_to_bp_index_ptr);
static int FillTheMatrix(const Eqn_list_node *Eq_list, const vector<Unique_basis_pair> &ColtoBP, int Num_unique_basis_pairs, int Num_equations);
static int SparseFillTheMatrix(const Eqn_list_node *Eq_list, const vector<Unique_basis_pair> &ColtoBP, int Num_unique_basis_pairs, int Num_equations, vector<list<Node> > &SM);
#if 0
static void PrintPairPresent(void);
static void PrintColtoBP(void);
static void PrintTheMatrix(void);
#endif

static Matrix TheMatrix;

int CreateTheMatrix(Eqn_list_node *Eq_list, Matrix *Mptr, int *Rows, int *Cols, vector<Unique_basis_pair> &ColtoBP, Name n)
{
    TheMatrix = NULL;

    if (!Eq_list || !Eq_list->basis_pairs)
        return(OK);

    ZeroOutPairPresent();

    int Num_unique_basis_pairs, Num_equations;
    FillPairPresent(Eq_list, Num_unique_basis_pairs, Num_equations);
/*
    PrintPairPresent();	
*/
    if (CreateColtoBP(n, Num_unique_basis_pairs, ColtoBP) != OK)
        return(0);
    if (FillTheMatrix(Eq_list, ColtoBP, Num_unique_basis_pairs, Num_equations) != OK)
        return(0);

    *Mptr = TheMatrix;
    *Rows = Num_equations;
    *Cols = Num_unique_basis_pairs; 

    return(OK);
}

/* Added by DCL (8/92). This is virtually identical to CreateTheMatrix()
   except that we call SparseFillTheMatrix() and copy the Matrix ptr to
   a pointer internal to this module to be altered and then copy it back
   before returning to SolveEquations() in Build.c */

int SparseCreateTheMatrix(Eqn_list_node *Eq_list, vector<list<Node> > &SM, int *Rows, int *Cols, vector<Unique_basis_pair> &ColtoBP, Name n)
{
    if (!Eq_list || !Eq_list->basis_pairs)
        return(OK);

    ZeroOutPairPresent();

    int Num_unique_basis_pairs, Num_equations;
    FillPairPresent(Eq_list, Num_unique_basis_pairs, Num_equations);
/*
    PrintPairPresent();
*/
    if (CreateColtoBP(n, Num_unique_basis_pairs, ColtoBP) != OK)
        return(0);
    if (SparseFillTheMatrix(Eq_list, ColtoBP, Num_unique_basis_pairs, Num_equations, SM) != OK)
        return(0);

    /* pass locally derived values back to the calling routine */

    *Rows = Num_equations;
    *Cols = Num_unique_basis_pairs; 

    return(OK);
}


void DestroyTheMatrix(void)
{
    assert_not_null_nv(TheMatrix);
    free(TheMatrix);
}


void ZeroOutPairPresent(void)
{
  pp_clear();
}


void FillPairPresent(const Eqn_list_node *Eq_list, int &Num_unique_basis_pairs, int &Num_equations)
{
    Num_unique_basis_pairs = 0;
    Num_equations = 0;

    const Eqn_list_node *temp = Eq_list;

    while (temp) {
        int i = 0;
        if(!temp->basis_pairs) break;

        while (temp->basis_pairs[i].coef != 0) {
            pp_set(temp->basis_pairs[i].left_basis,
                   temp->basis_pairs[i].right_basis);
            i++;
        }

        Num_equations++;
        temp = temp->next;
    }

    Num_unique_basis_pairs = pp_count();
}

              
int CreateColtoBP(Name N, int Num_unique_basis_pairs, vector<Unique_basis_pair> &ColtoBP)
{
    int col_to_bp_index = 0;

    int i;
    Degree d;

    if (Num_unique_basis_pairs == 0)
        return(OK);

    ColtoBP.resize(Num_unique_basis_pairs);

    d = GetDegreeName(N);
     
    for (i=1;i<d;i++)
        /*  	This check added 5/94   (DPJ)    */
        if ( AreBasisElements(i) && AreBasisElements(d-i) ) 
        {
            Process(ColtoBP, i,d-i,&col_to_bp_index);
        }
    return(OK);
}

/*
5/94 (DPJ)
Returns true if there are basis elements at degree d.
Note: when the algebra is nilpotent it is possible
that no new basis elements were entered at degree d.
*/
int AreBasisElements(Degree d)
{
   if (BasisStart(d) == 0) 
       return FALSE;
   else
       return TRUE;
}


void Process(vector<Unique_basis_pair> &ColtoBP, Degree d1, Degree d2, int *col_to_bp_index_ptr)
{
    Basis b1,b2,b3,b4;
    /*Basis row,col;*/
    /*int col_bit;*/
    Basis i,j;

    b1 = BasisStart(d1);
    b2 = BasisEnd(d1);
    b3 = BasisStart(d2);
    b4 = BasisEnd(d2);

    for (i=b1;i<=b2;i++) {
        for (j=b3;j<=b4;j++) {
          if(pp_contains(i, j)) {
                ColtoBP[*col_to_bp_index_ptr].left_basis = i; 
                ColtoBP[*col_to_bp_index_ptr].right_basis = j; 
                (*col_to_bp_index_ptr)++;
            }
        }
    }
}
            

int FillTheMatrix(const Eqn_list_node *Eq_list, const vector<Unique_basis_pair> &ColtoBP, int Num_unique_basis_pairs, int Num_equations)
{
    int i,j;
    int eq_number = 0;
    int col;
    int thematrix_index;
    Scalar x;

    if ((Num_unique_basis_pairs == 0) || (Num_equations == 0))
        return(OK);
    TheMatrix = (Scalar *) (Mymalloc(Num_equations*Num_unique_basis_pairs
                                   *sizeof(Scalar)));
    assert_not_null(TheMatrix);
    for (i=0;i<Num_equations;i++)
        for (j=0;j<Num_unique_basis_pairs;j++) 
            TheMatrix[i * Num_unique_basis_pairs + j] = 0;

    const Eqn_list_node *temp = Eq_list;
    while (temp) {
        i = 0;
        if (temp->basis_pairs == NULL)
			{
            return(OK);
			}
        while (temp->basis_pairs[i].coef != 0) {
            col = GetCol(ColtoBP,
                         temp->basis_pairs[i].left_basis,
                         temp->basis_pairs[i].right_basis);
            thematrix_index = eq_number * Num_unique_basis_pairs + col;
            x = TheMatrix[thematrix_index];
            TheMatrix[thematrix_index] = S_add(x, temp->basis_pairs[i].coef);
            i++;
        }
        eq_number++;
        temp = temp->next;
    }
    return(OK);
}

int SparseFillTheMatrix(const Eqn_list_node *Eq_list, const vector<Unique_basis_pair> &ColtoBP, int Num_unique_basis_pairs, int Num_equations, vector<list<Node> > &SM)
{
  if (Num_unique_basis_pairs == 0 || Num_equations == 0)
    return(OK);
	
  SM.resize(Num_equations);

  int eq_number = 0;
  for(const Eqn_list_node *temp = Eq_list; temp && temp->basis_pairs; temp = temp->next, eq_number++) {
    list<Node> &t_row = SM[eq_number];

    for(int i=0; temp->basis_pairs[i].coef != 0; i++) {
      int col = GetCol(ColtoBP,
                       temp->basis_pairs[i].left_basis,
		       temp->basis_pairs[i].right_basis);
      Scalar coef = temp->basis_pairs[i].coef;

      list<Node>::iterator ii;
      for(ii = t_row.begin(); ii != t_row.end() && ii->column < col; ii++) {
      }

      if(ii == t_row.end() || ii->column != col) {
	  /* Here we should add a node since there is no node in 
	     the row with the column value we are looking for */

          Scalar t = S_add(S_zero(), coef);

          if(t != S_zero()) {
            Node node;
            node.column = col;
            node.element = t;
            //if(ii == r_row.end()) {
            //  t_row.push_back(node);
            //} else {
            //}
            t_row.insert(ii, node);
          }
      } else {
          /* There is a node here with the same column value we are
               looking for so add the new value to this one */

          Scalar t = S_add(ii->element, coef);

          if(t == S_zero()) {
            /* If the result is zero then we will want to delete this node */
            t_row.erase(ii);
          } else {
            /* the result was nonzero and we just change the node element field to the result */
            ii->element = t;
          }
      }
        }
  }

  return OK;
}


int GetCol(const vector<Unique_basis_pair> &ColtoBP, Basis Left_basis, Basis Right_basis)
{
    int Num_unique_basis_pairs = pp_count();

    //if (Num_unique_basis_pairs == 0 || Num_equations == 0)
    //    return(-1);

    int low = 0;
    int high = Num_unique_basis_pairs - 1;

    while (low <= high) {
        int middle = (low + high)/2; 
        if (Left_basis < ColtoBP[middle].left_basis)
            high = middle - 1;
        else
            if (Left_basis > ColtoBP[middle].left_basis)
                low = middle + 1;
        else {
            if (Right_basis < ColtoBP[middle].right_basis)
                high = middle - 1;
            else
                if (Right_basis > ColtoBP[middle].right_basis)
                    low = middle + 1;
            else
                return(middle);
        }
    }

    return(-1);
}


#if 0
void PrintPairPresent(void)
{
    int i,j,k;

    assert_not_null_nv(Pair_present);

    for(i=0;i<DIMENSION_LIMIT;i++){
        for(j=0;j<PP_COL_SIZE;j++){
            for(k=0;k<8;k++){
/*                if (Pair_present[i][j] & BIT_VECTOR[k]) */
		if(getPairPresent(i, j) & BIT_VECTOR[k]){	/* 9/93 - TW - new Pair_present routine */
                    printf("1");
		}
                else{
                    printf("0");
		}
	    }
        }
        printf("\n");
    }
}


void PrintColtoBP(void)
{
    int i,j=0;

    assert_not_null_nv(ColtoBP);

    printf("The ColtoBP is : \n");

    for (i=0;i<Num_unique_basis_pairs;i++) {
        printf("b[%d]b[%d] ",ColtoBP[i].left_basis,ColtoBP[i].right_basis);
        j = (j+1)%7;
        if (j == 0)
            printf("\n");
    }
    printf("\n");
}


void PrintTheMatrix(void)
{
    int i,j,k=0;
    int thematrix_index;

    assert_not_null_nv(TheMatrix);

    printf("The Matrix to be solved is : \n");

    for (i=0;i<Num_equations;i++) {
        for (j=0;j<Num_unique_basis_pairs;j++) {
            thematrix_index = i * Num_unique_basis_pairs + j;
            printf(" %3d",TheMatrix[thematrix_index]);
            k = (k + 1)%25;
            if (k == 0)
                printf("\n");
        }
        k = 0;
        printf("\n");
    }
    printf("\n");
}
#endif

