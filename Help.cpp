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
#if 0
#include <curses.h>
#endif

#include "Help.h"
#include "Help_pri.h"

static const char *getHelp(const char *helpRqst);
static void displayHelp(const char *helpPtr, int rows, int cols);

void initHelp(void)
{
#if 1
  helpLines = 24; /* resonable defaults */
  helpCols = 80;
#else
  initscr();                            /* initialize LINES and COLS */
  helpLines = LINES;
  helpCols = COLS;
  clear();                              /* clear the buffer */
  refresh();                            /* display the buffer */
  endwin();
#endif
}

int Help(char topic[])
{
  char str[80];
  const char *helpPtr; /*, *test;*/

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
    /*test =*/ fgets(str, 80, stdin);
    str[strlen(str)-1] = '\0';
  }
  return 1;
}




const char *getHelp(const char *helpRqst)
{
  int i;

  for(i = 0; i < NUM_COMMANDS; ++i){
    if(!strncmp(helpRqst, Help_lines[i].help_topic, strlen(helpRqst))){
      return(Help_lines[i].help_text);
    }
  }
  return(NULL);
}




void displayHelp(const char *helpPtr, int rows, int cols)
{
#if 1
  puts(helpPtr);
#else
  char line[500 + 1], *blank = NULL, *pos = helpPtr;
  int i, j, done, blankNdx;

  for(j = 0; *pos; ++j){
    for(i = 0, done = 0, blankNdx = 0; i < cols && *pos && !done; ++i, ++pos){
      if(*pos == ' ' || *pos == '\t'){
        blank = pos;		/* set blank to point to most current white space */
        blankNdx = i;		/* save the index to the last white space */
      }
      else if(*pos == '\n'){
	done = 1;
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
#if 0
    if(j >= rows-2){
      j += 3;			/* increment the row counter */
      printf("\nHit Return to continue-->");	/* print more message at bottom left of screen */
      fflush(stdout);
      getchar();
      printf("\n");
      j = 0;			/* reset the line number */
    }
#endif
  }
#endif
}


void more(int *lines)
{
  return;

  if(*lines >= helpLines-2){
    *lines = 0;
    printf("\nHit Return to continue-->");    /* print more message at bottom left of screen */
    fflush(stdout);
    getchar();
    printf("\n");
  }
}