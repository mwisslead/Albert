/******************************************************************/
/***  FILE :        Sparse_structs.h                            ***/
/***  AUTHOR:       David Lee                                   ***/
/***  PROGRAMMER:   David Lee                                   ***/
/***  DATE WRITTEN: April 1992.                                 ***/
/***  PUBLIC ROUTINES:                                          ***/
/***  PRIVATE ROUTINES:                                         ***/
/***  MODULE DESCRIPTION:                                       ***/
/***          The include file for the Sparse Matrix code       ***/
/***          Gives the structure of a node                     ***/
/******************************************************************/

 struct Node_struct {
			struct Node_struct *Next_Node;
			unsigned short int column;
			unsigned char element;
		   } ;


typedef struct Node_struct Node;



