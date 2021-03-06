#ifndef _PO_PARSE_POLY_H_
#define _PO_PARSE_POLY_H_

#define     DEBUG_PARSE  0
#define     TRUE         1
#define     FALSE        0
#define     MAX_INT      67108864     /* 2^26 */
#define     NSYMBOLS     41
#define     NUM_PRODS    36
#define     STRING_LEN   100
#define     MAXLINE      100
#define     HANDLE_LEN   50
#define     NO_RELATION  0
#define     EQ_RELATION  1
#define     GT_RELATION  2
#define     LT_RELATION  3
#define     SCALAR_U_BOUND  65535    /* 2^16 - 1 */
#define     SCALAR_L_BOUND  -65535    /* 2^16 */

/* The following are codes for tokens */
#define     INVALID_TOKEN  -1
#define     CENT            0
#define     MINUS           19
#define     PLUS            20
#define     INT             21
#define     EXP_SYM         22
#define     STAR            23
#define     LETTER          24
#define     LEFT_PARAN      25
#define     RIGHT_PARAN     26
#define     LEFT_BRACKET    27
#define     COMMA           28
#define     RIGHT_BRACKET   29
#define     JORDAN          30
#define     TOKEN_LESS      31
#define     TOKEN_GREATER   32
#define     LEFT_BRACE      33
#define     RIGHT_BRACE     34
#define     LEFT_QUOTE      35
#define     RIGHT_QUOTE     36
#define     COLON           37
#define     A_WORD          38
#define     SEMI_COLON      39 
#define     DOLLAR          40

/*
 * The polynomial string gets converted into a tree (max arity is 3).
 * The nodes of the tree are unexpanded tree nodes (unexp_tnode).
 * If the node represents an atom (either a small letter or integer),
 * then operand1,2,3 are NULL's. Otherwise the node will have an operator
 * and 1 or more operands. (i.e pointers to other nodes i.e subtrees)
 *
 */
typedef struct unexp_tnode {
    int    op;
    char   s_letter;
    int    scalar_num;
    struct unexp_tnode *operand1;
    struct unexp_tnode *operand2;
    struct unexp_tnode *operand3;
    struct unexp_tnode *next;
} UNEXP_TNODES;

struct unexp_tnode *Create_parse_tree(char Poly_str[]);

#endif
