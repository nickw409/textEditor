#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "linkedList.h"

#define NTE_VERSION "0.1.1"

#define PAGE_SIZE 0x1000

typedef struct editorConfig {
   int screenheight;
   int screenwidth;
   int cursorX;
   int cursorY;
   int numrows;
   int rowOffset;
   ListNode *line;
} editorConfig;

struct lineBuffer {
   int len;
   ListNode *str;
} lineBuffer;

editorConfig E;

void initNcurses() {
   initscr();
   cbreak();
   noecho();
   keypad(stdscr, TRUE);
   clear();
}

void initEditor() {
   getmaxyx(stdscr, E.screenheight, E.screenwidth);
   E.cursorX = 0;
   E.cursorY = 0;
   E.numrows = 0;
   E.rowOffset = 0;
   E.line = NULL;
}

void printError(const char *error) {
   /*Move cursor to top of screen and print error msg*/
   move(0, 0);
   refresh();

   perror(error);
   exit(1);
}

void refreshScreen() {
   int height = E.screenheight;
   move(0, 0);
   refresh();
   if (E.numrows < E.screenheight) {
      height = E.numrows;
   }
   for (int i = E.rowOffset; i < (height + E.rowOffset); i++) {
      printw("%s", E.line[i].buffer);
   }
}

int getInput() {
   int c;

   while ((c = getch()) != KEY_END) {
      switch (c) {
         case KEY_UP:
            if (E.cursorY > 0) {
               E.cursorY--;
            }
            if (E.cursorY == 0 && E.rowOffset > 0) {
               E.rowOffset--;
               refreshScreen();
            }
            break;
         case KEY_DOWN:
            if (E.cursorY < E.screenheight - 1) {
               E.cursorY++;
            }
            if (E.cursorY == E.screenheight - 1 && 
                  E.rowOffset < (E.numrows - E.screenheight))
            {
               E.rowOffset++;
               refreshScreen();
            }
            break;
         case KEY_RIGHT:
            if (E.cursorX < E.screenwidth - 1) {
               E.cursorX++;
            }
            break;
         case KEY_LEFT:
            if (E.cursorX > 0) {
               E.cursorX--;
            }
            break;
      }
      move(E.cursorY, E.cursorX);
      refresh();
   }
   return 0;
}

void addRow(int size, char *s, int index) 
{
   ListNode *chars = 
   E.line = addHead(E.line, 

   if (E.line == NULL) 
   {
      printError("Error allocating memory for new row");
   }

   E.line[E.numrows].len = size;
   E.line[E.numrows].buffer = malloc(size + 1);
   E.line[E.numrows].buffer = memcpy(E.line[E.numrows].buffer, s, size);
   E.line[E.numrows].buffer[size] = 0;
   E.numrows++;
}

int openFile(char *file) 
{
   int c;
   int len = 0;
   int maxLen = 4096;
   char *buff = malloc(maxLen);
   FILE *fp = fopen(file, "r");
   while ((c = fgetc(fp)) != EOF) {
      buff[len] = c;
      len++;
      if (len > 0 && (c == '\n')) {
         appendRow(len, buff);
         len = 0;
      }
      if (len == maxLen) {
         maxLen *= 2;
         buff = realloc(buff, maxLen);
      }
   }
   free(buff);
   fclose(fp);
   refreshScreen();
   return 0;
}

int main(int argc, char* argv[]) 
{
   int erro = 0;
   if (argc < 2) {
      fprintf(stderr, "Usage: %s filename\n", argv[0]);
      exit(1);
   }
   initNcurses();
   initEditor();
   erro = openFile(argv[1]);
   if (erro == 1) {
      printError("Failed to read file");
   }
   erro = getInput();
   if (erro == 1) {
      printError("Failed to get user input");
   }
   endwin();
   return 0;
}
