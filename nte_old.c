/*
   Nick's Text Editor: NTE 
   written by Nick Wiley
*/

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define APPENDBUFFER_INIT {NULL, 0}
#define NTE_VERSION "0.0.2"

enum editorKey {
   ARROW_UP = 420,
   ARROW_DOWN,
   ARROW_LEFT,
   ARROW_RIGHT,
   DELETE_KEY
};

typedef struct erow {
   int size;
   char *chars;
} erow;

struct editorConfig 
{
   int cx;
   int cy;
   int screenrows;
   int screencols;
   int numrows;
   int col_offset;
   int row_offset;
   erow *row;
   struct termios orig;
};

struct appendBuffer
{
   char *buffer;
   int len;
};

struct editorConfig econfig;

//Prints error and exits program
void printError(const char *error)
{
   write(STDOUT_FILENO, "\x1b[2J", 4);
   write(STDOUT_FILENO, "\x1b[H", 3);

   perror(error);
   exit(1);
}

void abAppend(struct appendBuffer *ab, const char *s, int len)
{
   char *new = realloc(ab->buffer, ab->len + len);

   if (new == NULL)
   {
      return;
   }
   memcpy(&new[ab->len], s, len);
   ab->buffer = new;
   ab->len += len;
}

void abFree(struct appendBuffer *ab)
{
   free(ab->buffer);
}

//returns terminal to original settings
void disableRawMode()
{
   if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &econfig.orig) == -1)
   {
      printError("Failed to set terminal attributes back to original\n");
   }
}

//disables normal terminal settings to allow for a text editor style output
void enableRawMode()
{
   if (tcgetattr(STDIN_FILENO, &econfig.orig) == -1)
   {
      printError("Failed to get terminal attributes\n");
   }
   atexit(disableRawMode);

   struct termios raw = econfig.orig;
   raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);
   raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
   raw.c_oflag &= ~(OPOST);
   raw.c_cflag |= (CS8);
   raw.c_cc[VMIN] = 0;
   raw.c_cc[VTIME] = 1;

   if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
   {
      printError("Failed to set raw attributes\n");
   }
}

int readKey()
{
   int num_read;
   char c;
   while ((num_read = read(STDIN_FILENO, &c, 1)) != 1)
   {
      if (num_read == -1 && errno != EAGAIN)
      {
         printError("Error occurred while reading\n");
      }
   }

   if (c == '\x1b')
   {
      char seq[3];
      if (read(STDIN_FILENO, &seq[0], 1) != 1) {
         return '\x1b';
      }
      if (read(STDIN_FILENO, &seq[1], 1) != 1) {
         return '\x1b';
      }

      if (seq[0] == '[') {
         if (seq[1] >= '0' && seq[1] <= '9') {
            if (read(STDIN_FILENO, &seq[2], 1) != 1) {
               return '\x1b';
            }
            if (seq[2] == '~') {
               switch (seq[1]) {
                  case '3': return DELETE_KEY;
               }
            }
         } else {
            switch (seq[1]) 
            {
               case 'A': return ARROW_UP;
               case 'B': return ARROW_DOWN;
               case 'C': return ARROW_RIGHT;
               case 'D': return ARROW_LEFT;
            }
         }
      }
      return '\x1b';
   }
   else
   {
      return c;
   }
}

void moveCursor(int key) {
   switch (key) {
      case ARROW_LEFT:
         if (econfig.cx != 0) {
            econfig.cx--;
         }
         if (econfig.cx == 0 && econfig.col_offset > 0) {
            econfig.col_offset--;
         }
         break;
      case ARROW_RIGHT:
         if (econfig.cx != econfig.screencols - 1) {
            econfig.cx++;
         }
         
         break;
      case ARROW_UP:
         if (econfig.cy != 0) {
            econfig.cy--;
         }
         if (econfig.cy == 0 && econfig.row_offset > 0) {
            econfig.row_offset--;
         }
         break;
      case ARROW_DOWN:
         if (econfig.cy != econfig.screenrows - 1) {
            econfig.cy++;
         }
         if (econfig.cy == econfig.screenrows - 1) { 
            if (econfig.row_offset < (econfig.numrows - econfig.screenrows)) {
               econfig.row_offset++;
            }   
         }
         break;
   }
}

int getCursorPosition(int *rows, int *cols)
{
   char buf[32];
   unsigned int i = 0;

   if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
   {
      return -1;
   }

   while (i < sizeof(buf) - 1)
   {
      if (read(STDIN_FILENO, &buf[i], 1) != 1)
      {
         break;
      }
      if (buf[i] == 'R')
      {
         break;
      }
      i++;
   }
   buf[i] = '\0';

   if (buf[0] != '\x1b' || buf[1] != '[')
   {
      return -1;
   }
   if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
   {
      return -1;
   }
   return 0;
}

int getWindowSize(int *rows, int *cols)
{
   struct winsize ws;

   if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
   {
      if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      {
         return -1;
      }
      return getCursorPosition(rows, cols);
   }
   else
   {
      *cols = ws.ws_col;
      *rows = ws.ws_row;
      return 0;
   }
}

void editorAppendRow(char *s, size_t len) {
   econfig.row = realloc(econfig.row, sizeof(erow) * (econfig.numrows + 1));
   
   int current = econfig.numrows;
   econfig.row[current].size = len;
   econfig.row[current].chars = malloc(len + 1);
   memcpy(econfig.row[current].chars, s, len);
   econfig.row[current].chars[len] = '\0';
   econfig.numrows++;
}

void editorOpen(char *file) {
   FILE *fp = fopen(file, "r");
   if (fp == NULL) {
      printError("Failed to open file\n");
   }
   char *line = NULL;
   ssize_t linelen;
   size_t linecap = 0;

   while ((linelen = getline(&line, &linecap, fp)) != -1) {
      while (linelen > 0 && (line[linelen-1] == '\n' ||
                              line[linelen-1] == '\r'))
      {
         linelen--;
      }
      editorAppendRow(line, linelen);
   }
   free(line);
   fclose(fp);
}

void processKeypress()
{
   int c = readKey();

   switch(c)
   {
      case CTRL_KEY('q'):
         //Clears screen
         write(STDOUT_FILENO, "\x1b[2J", 4);
         //Moves cursor to top left
         write(STDOUT_FILENO, "\x1b[H", 3);
         exit(0);
         break;

      case ARROW_UP:
      case ARROW_DOWN:
      case ARROW_RIGHT:
      case ARROW_LEFT:
         moveCursor(c);
         break;

   }
}

void drawRows(struct appendBuffer *ab) {
   for (int i = 0; i < econfig.screenrows; i++) {
      if (i >= econfig.numrows) {
         if (econfig.numrows == 0 && i == 0) {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome),
               "Nick's Text Editor: version %s", NTE_VERSION);
            if (welcomelen > econfig.screencols) {
               welcomelen = econfig.screencols;
            }
            int padding = (econfig.screencols - welcomelen) / 2;
            while (padding--) {
               abAppend(ab, " ", 1);
            }
            abAppend(ab, welcome, welcomelen);
         } else {
            abAppend(ab, "~", 1);
         }
      } else {
         int offset = i + econfig.row_offset;
         int len = econfig.row[offset].size;
         if (len > econfig.screencols) {
            len = econfig.screencols;
         }
         abAppend(ab, econfig.row[offset].chars, len);
      }
      //Erases whole line
      abAppend(ab, "\x1b[K", 3);
      if (i < econfig.screenrows - 1) {
         abAppend(ab, "\r\n", 2);
      }
   }
}

void refreshScreen()
{
   struct appendBuffer ab = APPENDBUFFER_INIT;

   abAppend(&ab, "\x1b[?25l", 6);
   abAppend(&ab, "\x1b[H", 3);

   drawRows(&ab);

   char buff[32];
   snprintf(buff, sizeof(buff), "\x1b[%d;%dH", econfig.cy + 1, econfig.cx + 1);
   abAppend(&ab, buff, strlen(buff));

   abAppend(&ab, "\x1b[?25h", 6);

   write(STDOUT_FILENO, ab.buffer, ab.len);
   abFree(&ab);
}

void initEditor()
{
   econfig.cx = 0;
   econfig.cy = 0;
   econfig.numrows = 0;
   econfig.row_offset = 0;
   econfig.row = NULL;

   if (getWindowSize(&econfig.screenrows, &econfig.screencols) == -1)
   {
      printError("Error initializing editor\n");
   }
}

int main(int argc, char* argv[])
{
   enableRawMode();
   initEditor();
   if (argc >= 2) {
      editorOpen(argv[1]);
   }

   while (1)
   {
      refreshScreen();
      processKeypress();
   }

   return 0;
}
