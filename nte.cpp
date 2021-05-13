#include "nte.h"

Editor::Editor(char *file)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    clear();

    getmaxyx(stdscr, screen_height_, screen_width_);
    cursor_x_ = 0;
    cursor_y_ = 0;
    num_lines_ = 0;
    row_offset_ = 0;
    file_ = file;
}

Editor::~Editor()
{
    endwin();
}

void Editor::GetError(std::string err)
{
    endwin();
    std::cerr << err << std::endl;
    exit(1);
}

void Editor::GetInput()
{
    int ch = 0;

    while (ch != KEY_F(1) && (ch = getch()) != KEY_END)
    {
        switch(ch)
        {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
                ProcessArrowKey(ch);
                break;
            case KEY_BACKSPACE:
                DeleteChar(GetLineIndex(cursor_y_), cursor_x_);
                RefreshScreen();
                break;
            case LINE_FEED:
                AddRow(std::string(""), GetLineIndex(cursor_y_+1));
                ProcessArrowKey(KEY_DOWN);
                RefreshScreen();
                break;
            case TAB:
                EditLine(GetLineIndex(cursor_y_), cursor_x_, std::string(TAB_LENGTH, 32));
                for (int i = 0; i < TAB_LENGTH; ++i)
                {
                    ++cursor_x_;
                }
                RefreshScreen();
            case KEY_F(1):
                SaveFile();
                break;
            default:
                if (31 < ch < 127 && ch != '\t')
                {
                    EditLine(GetLineIndex(cursor_y_), cursor_x_, std::string(1, ch));
                    ++cursor_x_;
                    RefreshScreen();
                }
        }
        move(cursor_y_, cursor_x_);
        refresh();
    }
}

void Editor::ProcessArrowKey(int c)
{
    switch(c)
    {
        case KEY_UP:
            if (cursor_y_ > 0)
            {
                --cursor_y_;
                cursor_x_ = GetLineLen(cursor_y_);
            }
            else if (cursor_y_ == 0 && row_offset_ > 0)
            {
                --row_offset_;
                cursor_x_ = GetLineLen(cursor_y_);
                RefreshScreen();
            }
            break;
        case KEY_DOWN:
            if (cursor_y_ < screen_height_ - 1)
            {
                ++cursor_y_;
                cursor_x_ = GetLineLen(cursor_y_);
            }
            else if (cursor_y_ == screen_height_ - 1 &&
                row_offset_ < (num_lines_ - screen_height_))
            {
                ++row_offset_;
                cursor_x_ = GetLineLen(cursor_y_);
                RefreshScreen();
            }
            break;
        case KEY_LEFT:
            if (cursor_x_ > 0)
            {
                --cursor_x_;
            }
            else
            {
                ProcessArrowKey(KEY_UP);
            }
            break;
        case KEY_RIGHT:
            if (cursor_x_ < GetLineLen(cursor_y_))
            {
                ++cursor_x_;
            }
            else
            {
                ProcessArrowKey(KEY_DOWN);
            }
            break;
    }
}

void Editor::RefreshScreen()
{
    int line;
    getmaxyx(stdscr, screen_height_, screen_width_);
    move(0, 0);
    clear();
    for (int i = 0; i < num_lines_; ++i)
    {
        line = i + row_offset_;
        mvprintw(i, 0, "%s", lines_[line].c_str());
    }
    move(cursor_y_, cursor_x_);
    refresh();
}

void Editor::AddRow(std::string str)
{
    lines_.push_back(str);
    ++num_lines_;
}

void Editor::AddRow(std::string str, int index)
{
    lines_.insert(lines_.begin()+index, str);
    ++num_lines_;
}

void Editor::RemoveRow(int index)
{
    lines_.erase(lines_.begin()+index);
    --num_lines_;
}

void Editor::EditLine(int row, int col, std::string str)
{
    if (row > num_lines_ || row < 0)
    {
        GetError("Row index for EditLine is out of bounds");
    }

    //std::string line = lines_[row];
    if (col > GetLineLen(row) || col < 0)
    {
        GetError("Col index for EditLine is out of bounds");
    }
    lines_[row].insert(col, str);
    //RemoveRow(row);
    //line.insert(col, str);
    //AddRow(line, row);
}

void Editor::DeleteChar(int row, int col)
{
    if (GetLineLen(row) == 0)
    {
        RemoveRow(row);
        ProcessArrowKey(KEY_UP);
    }
    else
    {
        if (col > GetLineLen(row) || col < 0)
        {
            GetError("Col index in DeleteChar is out of bounds");
        }
        lines_[row].erase(col-1, 1);
        ProcessArrowKey(KEY_LEFT);
    }
}
void Editor::OpenFile()
{
    char c[DEFAULT_BUFFLEN];
    std::string str;
    std::fstream fs;
    fs.open(file_, std::fstream::in | std::fstream::out | std::fstream::trunc);
    if ((fs.rdstate() & std::fstream::failbit) != 0)
    {
        fs.close();
        GetError("Failure opening file stream.");
    }
    while (!fs.eof())
    {
        fs.getline(c, DEFAULT_BUFFLEN);
        str = c;
        AddRow(str);
    }
    fs.close();
    RefreshScreen();
}

void Editor::SaveFile()
{
    std::fstream fs;
    fs.open(file_, std::fstream::out);
    if ((fs.rdstate() & std::fstream::failbit) != 0)
    {
        fs.close();
        GetError("Failure opening file stream.");
    }
    for (int i = 0; i < num_lines_; ++i)
    {
        fs.write(lines_[i].c_str(), lines_[i].length());
        fs.write("\n", 1);
    }
    fs.close();
}

int Editor::GetLineIndex(int row)
{
    if (row + row_offset_ > num_lines_)
    {
        GetError("Row in GetLineIndex out of bounds");
    }
    return row + row_offset_;
}

int Editor::GetLineLen(int row)
{
    int index = GetLineIndex(row);
    if (index > num_lines_)
    {
        GetError("Row index for GetLineLen out of bounds");
    }
    return lines_[index].length();
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " filename" << std::endl;
        exit(1);
    }
    Editor editor(argv[1]);
    editor.OpenFile();
    editor.GetInput();

    return 0;
}