#ifndef NTE_H
#define NTE_H

#include <ncurses.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <fstream>

#define DEFAULT_BUFFLEN 1024
#define LINE_FEED 10
#define TAB 9
#define TAB_LENGTH 3

class Editor
{
    public:
        Editor(char *file);
        ~Editor();
        void GetError(std::string err);
        void GetInput();
        void ProcessArrowKey(int c);
        void RefreshScreen();
        void MoveCursor(int x, int y);
        void AddRow(std::string str);
        void AddRow(std::string str, int index);
        void RemoveRow(int index);
        void EditLine(int row, int col, std::string str);
        void DeleteChar(int row, int col);
        void OpenFile();
        void SaveFile();
        int GetLineIndex(int row);
        int GetLineLen(int row);
    
    private:
        int screen_height_;
        int screen_width_;
        int cursor_x_;
        int cursor_y_;
        int num_lines_;
        int row_offset_;
        std::string file_;
        std::vector<std::string> lines_;
};

#endif