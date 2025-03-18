#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string>
#include <vector>
#include <cmath>
#include <utility>
#include <fstream>
#include "stringtoint.hpp"
#include "strbcmp.h"
#include "center.h"

#define clearline(x) {\
	move((x),0);\
	clrtoeol();\
}

#define confirmation() {\
	printfeedback((char*)"Are you sure? All contents will be lost. [Y/N]");\
	clearline(row - 1);\
	getnstr(commandbuffer, 1);\
	if (!(strbcmp(commandbuffer, "y") || strbcmp(commandbuffer, "Y")))\
	{\
		return -2;\
	}\
}

#define markchange() {\
	lastbuffer = std::move(buffer);\
}

#define printfeedback(f) {_printfeedback((f),currentFile);}

#define getinput(m,l) {\
	printfeedback((char*)(m));\
	clearline(row - 1);\
	getnstr(commandbuffer, (l));\
}

#define LINECONTENTS "Enter line contents:"
#define LINENUMBER "Enter line number:"
#define FILENAME "Enter file name:"

std::vector<std::string> buffer;
const std::vector<std::string> emptyfile = {
	""
};
const std::vector<std::string> helpfile = {
	":h - help",
	":x - clear buffer",
	":q - quit TINR",
	":q! - quit without warning",
	":n - new line",
	":na - new line at",
	":e - edit line",
	":d - delete line",
	":w - write",
	":o! - open",
	":s - scrolling mode"
};

char* commandbuffer = (char*)malloc(1023);
int row, col;

std::string title = "TINR - TIN Reborn";
int titlelen = strlen(title.data());

std::string tip = "Type :h to open internal help file";

std::string feedback;

std::string currentFile;

bool saved = false;

bool scrollingmode = false;
int scrolloffset = 0;;

int
count_digits(int n) {
    if (n == 0) {
        return 1;
    }

    int count = 0;
    
    if (n < 0) {
        n = -n;
    }
    while (n != 0) {
        n /= 10;
        count++;
    }
    return count;
}

void
_printfeedback(char* feedback, std::string currentFile)
{
	int pr, pc;
	int row, col;

	char* file = currentFile.data();	

	if (currentFile.empty())
	{
		file = (char*)"None";
	}

	int length = strlen(file);

	getmaxyx(stdscr, row, col);

	getyx(stdscr, pr, pc);

	clearline(row - 2);

	move(row - 2, 0);
	printw("%s", feedback);

	move(row - 2, col - length);
	printw("%s", file);

	move(row - 2, 0);
	chgat(-1, A_NORMAL, 2, NULL);

	move(pr, pc);
}

int
quit(bool warning)
{
	if (warning)
	{
		confirmation();
	}
	return -1;
}

void
insert (int ln)
{
	getinput(LINECONTENTS, 1024);
	buffer.insert(buffer.begin() + (ln - 1), commandbuffer);
	feedback = "Inserted 1 line";
}

void
remove (void)
{
	getinput(LINENUMBER, 8);
	buffer.erase(buffer.begin() + (stringToInt(commandbuffer) - 1));
	feedback = "Removed 1 line";
}

int
open (void)
{
	buffer = std::move(std::vector<std::string>());

	std::ifstream file(commandbuffer);

	if (!file.is_open())
	{
		return -2;
	}
		
	std::string line;

	while (std::getline(file, line))
	{
		buffer.push_back(line);
	}

	file.close();

	feedback = "Opened file";

	return 0;
}

int
write (void)
{
	std::ofstream outputFile;

	outputFile.open(currentFile);

	if (!outputFile.is_open())
	{
		feedback = "fate";
		return -2;
	}

	for (std::string line : buffer)
	{
		outputFile << line << std::endl;
	}

	saved = true;
	feedback = "Saved.";

	return 0;
}

int
process_normal(void)
{
	int r;

	getnstr(commandbuffer, 25);	

	feedback = commandbuffer;

	if (strbcmp(commandbuffer, ":q"))
	{
		return quit(!saved);
	}
	if (strbcmp(commandbuffer, ":q!"))
	{
		return quit(false);
	}
	if (strbcmp(commandbuffer, ":n"))
	{
		insert(buffer.size() + 1);
	}
	if (strbcmp(commandbuffer, ":h"))
	{
		confirmation();
		buffer = std::move(helpfile);
		feedback = "Opened helpfile";
	}
	if (strbcmp(commandbuffer, ":x"))
	{
		confirmation();
		buffer = std::move(emptyfile);
		feedback = "I hope this is what you wanted.";
	}
	if (strbcmp(commandbuffer, ":d"))
	{
		remove();
	}
	if (strbcmp(commandbuffer, ":na"))
	{
		getinput(LINENUMBER, 8);
		int ln = stringToInt(commandbuffer);
		insert(ln);
	}
	if (strbcmp(commandbuffer, ":e"))
	{
		getinput(LINENUMBER, 8);		
		int ln = stringToInt(commandbuffer);
		getinput(LINECONTENTS, 1024);
		buffer[ln - 1] = commandbuffer;
		feedback = "Changed 1 line";
	}
	if (strbcmp(commandbuffer, ":o!"))
	{
		getinput(FILENAME, 256);		
		currentFile = commandbuffer;

		r = open();	
	}
	if (strbcmp(commandbuffer, ":w"))
	{
		if (currentFile.empty())
		{
			getinput(FILENAME, 256);			
			currentFile = commandbuffer;
		}
		r = write();
	}
	if (strbcmp(commandbuffer,":s"))
	{
		scrollingmode = true;
	}	
	return r;
}

int
process_scrolling(void)
{
	raw();
	keypad(stdscr, true);
	noecho();
	int ch;
	ch = getch();
	if (ch == KEY_LEFT)
	{
		scrolloffset = 0;
	}
	if (ch == KEY_UP)
	{
		if (!(scrolloffset > 0))
			return -2;
		scrolloffset--;
	}
	if (ch == KEY_DOWN)
	{
		if (!(scrolloffset < buffer.size() - 1))
			return -2;	
		scrolloffset++;
	}
	if (ch == KEY_RIGHT)
	{
		scrolloffset = buffer.size() - 1;
	}
	if (ch == ':')
	{
		scrollingmode = false;
		feedback = "ok escape";
		echo();
		keypad(stdscr, false);
		noraw();
		return -2;
	}
	feedback = "Arrow keys to scroll. : to escape";
	return 0;
}

int
loop(void)
{
	if (buffer.size() == 0)
	{
		buffer = std::move(emptyfile);
	}

	move(1, 0);
	attroff(COLOR_PAIR(2));

	{
		int l;
		int ml = row - 3;
		for (l = 1;l <= ml;l++) {
			clearline(l);
		}
	}

	{
		int ln = 1 + scrolloffset;
		int l = 1;
		int currentrow = 1;
		int _;
		int maxdigits = count_digits(buffer.size());
		std::string line;
		for (int l = 1;l <= row - 3 && ln <= buffer.size();l++)
		{
			line = buffer[ln - 1];
			move(l,(maxdigits - count_digits(ln)));
			attron(A_DIM);
			printw("%d ", ln);
			attroff(A_DIM);
			printw("%s", line.data());
			getyx(stdscr, currentrow, _);
			l += currentrow - l;
			ln++;
		}
	}

	clearline(row - 1);

	int r;

	if (scrollingmode)
	{
		r = process_scrolling();
	}
	else
	{
		r = process_normal();
		if (r == -1)
			return -1;
	}

	printfeedback(feedback.data());

	refresh();

	return 0;
}

int
main(void)
{	
	initscr();

	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLACK, COLOR_WHITE);

	raw();
	
	getmaxyx(stdscr, row, col);
	
	mvprintw(0, center(col, titlelen), "%s", title.data());
	mvchgat(0, 0, -1, A_BOLD, 2, NULL);

	printfeedback(tip.data());
	refresh();

	while (loop() != -1) {}
	
	endwin();
	
	free(commandbuffer);

	return 0;
}
