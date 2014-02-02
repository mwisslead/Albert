/******************************************************************/
/***  FILE :        Build.c                                     ***/
/***  AUTHOR:       David P. Jacobs                             ***/
/***  PROGRAMMER:   Sekhar Muddana                              ***/
/***  DATE WRITTEN: May 1990.                                   ***/
/***  MODIFIED:     Aug 1992. David Lee.                        ***/
/***                           Sparse Matrix Code Added.        ***/
/***                10/93 - Trent Whiteley                      ***/
/***                        changes made to implement interrupt ***/
/***                        handler                             ***/
/***  PUBLIC ROUTINES:                                          ***/
/***      int BuildDriver()                                     ***/ 
/***  PRIVATE ROUTINES:                                         ***/
/***      int InitializeStructures()                            ***/ 
/***      int DestroyStructures()                               ***/ 
/***      int PrintProgress()                                   ***/
/***      int ProcessDegree()                                   ***/
/***      int ProcessType()                                     ***/
/***      int SolveEquations()                                  ***/
/***  MODULE DESCRIPTION:                                       ***/
/***      Implement the Build Command.                          ***/
/***      Reads the sparse global variable to determine         ***/
/***      whether the traditional or sparse code should be used ***/
/***      in the SolveEquations routine.                        ***/
/******************************************************************/

#include <stdio.h>
#include <time.h>
#include "Build_defs.h"
#include "Mult_table.h"
#include "CreateMatrix.h"
#include "Po_parse_exptext.h"
#include "Id_routines.h"
#include "Sparse_structs.h"
#include "Sparse_defs.h"
#include "Debug.h"


#define assert_not_null(p) if (p == NULL) return(0)

/*****************************
 These variables are for the sparse implementation
******************************/
extern short int sparse;
short int gather_density_flag;
unsigned long int num_elements;
unsigned long int max_num_elements;
unsigned long int matrix_size;

extern int sigIntFlag;		/* TW 10/8/93 - flag for Ctrl-C */

Type Target_type;
struct id_queue_node *First_id_node;

static long Start_time; 
long Current_time; 
Basis Current_dimension;

/*******************************************************************/
/* MODIFIES:                                                       */
/*     Mult_table -- is fully built.                               */
/*     Basis_table -- will be Complete.                            */ 
/* REQUIRES:                                                       */
/*     Target_Type -- Target to be reached.                        */
/*     Identities -- All the identities entered by User.           */
/* FUNCTION:                                                       */
/*     This is the highest level function in this part of the      */
/*     system. It gets called from the command interpretter when   */
/*     "build" is invoked.                                         */
/*     For each degree, New Basis are created and New Products are */
/*     entered.                                                    */ 
/*******************************************************************/
int Build(Idq_node,Ttype)
struct id_queue_node *Idq_node;
Type Ttype;
{
    int GetDegreeName();

    char *convtime, *ctime();

    int Target_degree;

    int status = OK;
    int i;

    double density;

    time(&Start_time);
    convtime = ctime(&Start_time);
    printf("\nBuild begun at %s\n",convtime);
    printf("Degree    Current Dimension   Elapsed Time(in seconds) \n");

    Target_type = Ttype;

    First_id_node = Idq_node;

    status = InitializeStructures();

    Target_degree = GetDegreeName(TypeToName(Target_type));

    gather_density_flag = FALSE;	
    if (status == OK) {
        for (i=1; i <= Target_degree; i++)  {
	if (i == Target_degree)
           gather_density_flag = TRUE;	
		
            status = ProcessDegree(i);
	    if(sigIntFlag == 1){
/*	      printf("Returning from Build().\n");*/
	      return(-1);
	    }
            if (status != OK) 
                break;
            PrintProgress(i);
        }
    }
#if PRINT_BASIS_TABLE
    PrintBasisTable();
#endif

/*#if PRINT_TYPE_TABLE
    PrintTypetable();
#endif

#if DEBUG_MT
    PrintMT();
#endif*/

#if PRINT_MULT_TABLE
    Print_MultTable();
#endif

    DestroyStructures();
    density= 100 * ((double)max_num_elements/(double)matrix_size);
    if (status == OK)
    {
        printf("Build completed. ");
        if (density > 0.00)
        {
            printf("Last Matrix %2.2f%% dense.\n",density);
        }
        else
        {
           if (matrix_size==0)
           {
              printf("No Matrix.\n");
           }
           else
           {
             printf("Last Matrix %f%%dense.\n",density);
           }
        }
    }
    else
        printf("Build incomplete\n");
    return (status);
}


/*******************************************************************/
/* GLOBALS INITIALIZED:                                            */
/*      Scalar Inverse_table -- For Scalar Arithemetic.            */
/*      Mult_table -- Multiplication Table.                        */
/*      Type_table -- Type Table for the Given Target Type.        */
/*      Basis_table -- to 0's.                                     */
/*******************************************************************/
int InitializeStructures()
{
    int CreateMultTable();
    int CreateTypeTable();
    int CreateBasisTable();

    int status = OK; 

    int Target_degree;

    if (status == OK) 
        status = CreateMultTable();
    if (status == OK) 
        status = CreateTypeTable(Target_type);
    if (status == OK) { 
        Target_degree = GetDegreeName(TypeToName(Target_type));
        status = CreateBasisTable(Target_degree);
    }
    return (status);
}


DestroyStructures()
{
     DestroyTypeTable();
}


PrintProgress(i)
int i;
{
    time(&Current_time);

    printf("  %2d             %4d            %5d\n",i,Current_dimension,
                                     Current_time - Start_time);
}


/*******************************************************************/
/* REQUIRES:                                                       */
/*     i -- to process degree i.                                   */ 
/* FUNCTION:                                                       */
/*     Process all Types of degree i.                              */
/*******************************************************************/
ProcessDegree(i)
int i;
{
   Name FirstTypeDegree();
   Name NextTypeSameDegree();
   Name n;
   int status = OK;
   Basis begin_basis;
   Basis end_basis;

   if (i == 1)
       InstallDegree1();
   else {
       n = FirstTypeDegree(i);
       while ((status == OK) && (n != -1)) {
           begin_basis = GetNextBasisTobeFilled();
           status = ProcessType(n);
	   if(sigIntFlag == 1){	/* TW 10/5/93 - Ctrl-C check */
/*	     printf("Returning from ProcessDegree().\n");*/
	     return(-1);
	   }
           end_basis = GetNextBasisTobeFilled() - 1;
           if (end_basis < begin_basis)
               UpdateTypeTable(n,0,0);    /* No Basis table entries. */
           else
               UpdateTypeTable(n,begin_basis,end_basis);
           n = NextTypeSameDegree(n);
       }
       Current_dimension = end_basis;
   }
   return(status);
}


/*******************************************************************/
/* REQUIRES: None.                                                 */
/* FUNCTION:                                                       */
/*     All Degree 1 Basis i.e Generators are entered into Basis    */
/*     Table.                                                      */ 
/*******************************************************************/
InstallDegree1()
{
    Type GetNewType();

    int i,j;
    int len;
    Name n;
    Basis begin_basis;
    Basis end_basis;
    Type temp_type;

    temp_type = GetNewType();
    
    len = GetTargetLen();

    for (i=0;i<len;i++) {
        for (j=0;j<len;j++)
            temp_type[j] = 0;
        temp_type[i] = 1;
        n = TypeToName(temp_type);
        begin_basis = GetNextBasisTobeFilled();
        EnterBasis(0,0,n);
        end_basis = GetNextBasisTobeFilled() - 1;
        UpdateTypeTable(n,begin_basis,end_basis);
    }
    Current_dimension = end_basis;
}


/*******************************************************************/
/* REQUIRES:                                                       */
/*     t -- to process Type t.                                     */ 
/* FUNCTION:                                                       */
/*     For each identity f, whose degree is less than the degree   */
/*     of type t, generate equations corresponding to f.           */
/*     Then solve those equations, to get New Baisis and write     */
/*     other basis pairs in terms of existing basis.               */ 
/*******************************************************************/
/* Process type t for degree i */
ProcessType(n)
Name n;
{
    Eqn_list_node *GetNewEqnListNode();

    int status = OK;
    Eqn_list_node *L;               /* Header record of linked list */
    struct polynomial *f;
    struct id_queue_node *temp_id_node;

    L = NULL;
    L = GetNewEqnListNode();
    assert_not_null(L);

    temp_id_node = First_id_node;

    while ((temp_id_node != NULL) && (status == OK)) {
        f = temp_id_node->identity;
        if ((status == OK) && (f->degree <= GetDegreeName(n)))
            status = GenerateEquations(f,n,L);
	if(sigIntFlag == 1){		/* TW 10/5/93 - Ctrl-C check */
	  if(L != NULL){
	    FreeEqns(L);
	  }
/*	  printf("Returning from ProcessType().\n");*/
	  return(-1);
	}
        temp_id_node = temp_id_node->next;
    }

#if DEBUG_EQNS
    PrintEqns(L);
#endif

    if (status == OK) 
        status = SolveEquations(L,n);			
    if (L != NULL)
        FreeEqns(L);

    return(status);
}

/*******************************************************************/
/* REQUIRES:                                                       */
/*     L -- Head to List of Equations.                             */ 
/*     t -- Current Type being processed.                          */
/* FUNCTION:                                                       */
/*     Convert the given list of equations into Matrix, i.e one    */
/*     row for each equation and one column for each unique basis  */
/*     pair present in all equations.                              */
/*     Then Reduce that Matrix into row canonical form.            */
/*     Then Extract from the Reduced Matrix i.e Find New Basis     */
/*     and enter them into Basis Table. Then write Dependent Basis */
/*     pairs into Basis by entering products into Mult_table.      */ 
/*******************************************************************/
SolveEquations(L,n)
Eqn_list_node *L;             /* Linked list of pair lists */
Name n;
{
    int rows = 0;              /* Size of matrix */
    int cols = 0;              /* Size of matrix */
    Matrix *mptr = NULL;       /* Pointer to matrix */
    MAT_PTR Sparse_Matrix = NULL;
    Unique_basis_pair_list *BPCptr = NULL; /* pointer to BPtoCol */
    int rank = 0;
    int status = OK;


   /* this flag is only set on the last degree of the generator */

   if (gather_density_flag)
   {
      num_elements=0;
      max_num_elements=0;
      matrix_size=0;
   }
/* CreateMatrix will initialize rows, cols, m, BP
   and fill in matrix and BPtoCol */

/* determine the matrix structure */

   if (!sparse)
   {
     status = CreateTheMatrix(L, &mptr,&rows, &cols, &BPCptr,n);	
   }
   else
   {
     status = SparseCreateTheMatrix(L, &Sparse_Matrix,&rows, &cols, &BPCptr,n);
   }

   /* make sure that we get the correct information from creating the 
      matrix for our statistics */

   if (gather_density_flag)
   {
      max_num_elements=num_elements;
      matrix_size=rows*cols;
   }


#if DEBUG_MATRIX
   PrintColtoBP();
   PrintTheMatrix();
#endif


/* determine the matrix structure */

   if (status == OK)
   {
/* determine the matrix structure */
      if (!sparse)
      {
         status = ReduceTheMatrix(mptr, rows, cols,&rank);
      }
      else
      {
         status = SparseReduceMatrix(&Sparse_Matrix,rows,cols,&rank);
      }
   }

#if DEBUG_MATRIX
   PrintTheRMatrix();
#endif

/* ExtractMatrix will expand basis table & MultTable ! */
  if (status == OK) 
  {
/* determine the matrix structure */
     if (!sparse)
     {
        status = ExtractFromTheMatrix(mptr, rows, cols, rank, n,BPCptr);	
     }
     else
     {
	status =SparseExtractFromMatrix(Sparse_Matrix,rows,cols,rank,n,BPCptr);
     }
 }
#if DEBUG_MATRIX
   PrintDependent();
#endif

   DestroyBPtoCol();     /* Make sure not null before malloc */
   if (status == OK) 
   {
      if (!sparse)
      {
         DestroyTheMatrix();
      }
      else
      {
         DestroySparseMatrix(Sparse_Matrix);
      }
   }
/* 
   DestroyDependent(); 
     This array was already freed!  This resulted in
     an error in which the same memory was freed, allocatted, freed
     and allocated, thus begin allocated to two different functions.
     This was discovered in June, 1993 by Sekhar.
*/
   return(status);
}
