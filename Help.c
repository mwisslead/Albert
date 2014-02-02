/**********************************************************************/
/***  FILE :     Help.c                                             ***/
/***  AUTHOR:    Trent Whiteley                                     ***/
/***  MODIFIED:                                                     ***/
/***                                                                ***/
/***  PUBLIC ROUTINES:                                              ***/
/***    int Help();                                                 ***/
/***  PRIVATE ROUTINES:                                             ***/
/***    char * getHelp();                                           ***/
/***    void displayHelp();                                         ***/
/***  MODULE DESCRIPTION:                                           ***/
/***    This module contains routines used to handle the help       ***/
/***    system for albert.                                          ***/
/**********************************************************************/

#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "Help.h"

int initHelp()
{
  initscr();                            /* initialize LINES and COLS */
  helpLines = LINES;
  helpCols = COLS;
  clear();                              /* clear the buffer */
  refresh();                            /* display the buffer */
  endwin();
}

int Help(topic)
char topic[];
{
  char str[80], *helpPtr, *test;

  char * getHelp();
  void displayHelp();
  void scrollScreen();

  scrollScreen();
  str[0] = '\0';
  if(!strlen(topic)){
    strcpy(str, "H");			/* if no topic, display menu */
  }
  else{
    strcpy(str, topic);
  }
  while(strlen(str)){
    helpPtr = getHelp(str);
    if(helpPtr){
      displayHelp(helpPtr, helpLines, helpCols);
    }
    else{
      printf("No help is available for %s\n", str);
    }
    printf(" Type a letter for help, or carriage return to\n return\
	to the Albert session.\n\n\nHELP--> ");
    fflush(stdout);
    test = fgets(str, 80, stdin);
    str[strlen(str)-1] = '\0';
  }
  return(TRUE);
}




char * getHelp(helpRqst)
char *helpRqst;
{
  int i;

  for(i = 0; i < NUM_COMMANDS; ++i){
    if(!strncmp(helpRqst, Help_lines[i].help_topic, strlen(helpRqst))){
      return(Help_lines[i].help_text);
    }
  }
  return(NULL);
}




void displayHelp(helpPtr, rows, cols)
char *helpPtr;
int rows;
int cols;
{
  char line[500 + 1], *blank, *pos = helpPtr;
  int i, j, done, blankNdx;

  for(j = 0; *pos; ++j){
    for(i = 0, done = FALSE, blankNdx = 0; i < cols && *pos && !done; ++i,
++pos){
      if(*pos == ' ' || *pos == '\t'){
        blank = pos;		/* set blank to point to most current white space */
        blankNdx = i;		/* save the index to the last white space */
      }
      else if(*pos == '\n'){
	done = TRUE;
      }
      line[i] = *pos;
    }
    if(*(pos-1) == '\n'){	/* substitute EOL for CRLF */
      line[i-1] = '\0';
    }
    else if(blankNdx != 0){	/* set EOL at last white space encountered */
      line[blankNdx] = '\0';
      pos = blank;
    }
    else{			/* if no white space encountered, break up the word */
      line[cols + 1] = '\0';
    }
    printf("%s\n", line);	/* write the line */
    if(j >= rows-2){
      j += 3;			/* increment the row counter */
      printf("\nHit Return to continue-->");	/* print more message at
bottom left of screen */
      fflush(stdout);
      getchar();
      printf("\n");
      j = 0;			/* reset the line number */
    }
  }
}


void more(lines)
int *lines;
{
  if(*lines >= helpLines-2){
    *lines = 0;
    printf("\nHit Return to continue-->");    /* print more message at
bottom left of screen */
    fflush(stdout);
    getchar();
    printf("\n");
  }
}



void scrollScreen()
{
  int i;

  for(i = 0; i < helpLines; ++i){
    printf("\n");
  }
}


