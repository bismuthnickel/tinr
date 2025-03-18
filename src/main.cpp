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
		goto cancel;\
	}\
}

#define markchange() {\
	lastbuffer = std::move(buffer);\
}

#define printfeedback(f) {_printfeedback((f),currentFile);}

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
main(void)
{
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
		":o! - open"
	};

	char* commandbuffer = (char*)malloc(1024);
	int row, col;

	std::string title = "TINR - TIN Reborn";
	int titlelen = strlen(title.data());

	std::string tip = "Type :h to open internal help file";

	std::string feedback;

	std::string currentFile;

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

	bool saved = false;

	while (true)
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
			for (l = 1;l < ml;l++) {
				clearline(l);
			}
		}

		{
			int ln = 1;
			int l = 1;
			int cr = 1;
			int _;
			for (std::string line : buffer)
			{
				move(l,0);
				attron(A_DIM);
				printw("%d ", ln);
				attroff(A_DIM);
				printw("%s", line.data());
				getyx(stdscr, cr, _);
				l += cr - l + 1;
				ln++;
			}
		}

		clearline(row - 1);
		
		getnstr(commandbuffer, 25);	

		feedback = commandbuffer;

		if (strbcmp(commandbuffer, ":q"))
		{
			if (!saved)
			{
				confirmation();
			}
			break;
		}
		if (strbcmp(commandbuffer, ":q!"))
		{
			break;
		}
		if (strbcmp(commandbuffer, ":n"))
		{
			printfeedback((char*)"Enter line contents:");
			clearline(row - 1);
			getnstr(commandbuffer, 1024);
			buffer.push_back(std::string(commandbuffer));
			feedback = "Added 1 line";
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
			printfeedback((char*)"Enter line number:");
			clearline(row - 1);
			getnstr(commandbuffer, 8);
			buffer.erase(buffer.begin() + (stringToInt(commandbuffer) - 1));
			feedback = "Removed 1 line";
		}
		if (strbcmp(commandbuffer, ":na"))
		{
			printfeedback((char*)"Enter line number:");
			clearline(row - 1);
			getnstr(commandbuffer, 8);
			int ln = stringToInt(commandbuffer);
			printfeedback((char*)"Enter line contents:");
			clearline(row - 1);
			getnstr(commandbuffer, 1024);
			buffer.insert(buffer.begin() + (ln - 1), commandbuffer);
			feedback = "Inserted 1 line";
		}
		if (strbcmp(commandbuffer, ":e"))
		{
			printfeedback((char*)"Enter line number:");
			clearline(row - 1);
			getnstr(commandbuffer, 8);
			int ln = stringToInt(commandbuffer);
			printfeedback((char*)"Enter line contents:");
			clearline(row - 1);
			getnstr(commandbuffer, 1024);
			buffer[ln - 1] = commandbuffer;
			feedback = "Changed 1 line";
		}
		if (strbcmp(commandbuffer, ":o!"))
		{
			printfeedback((char*)"Enter file name:");
			clearline(row - 1);
			getnstr(commandbuffer, 256);
			currentFile = commandbuffer;

			buffer = std::move(std::vector<std::string>());

			std::ifstream file(commandbuffer);

			if (!file.is_open())
			{
				goto cancel;
			}
			
			std::string line;

			while (std::getline(file, line))
			{
        		buffer.push_back(line);
    		}

			file.close();

			feedback = "Opened file";
		}
		if (strbcmp(commandbuffer, ":w"))
		{
			if (currentFile.empty())
			{
				printfeedback((char*)"Enter file name:");
				clearline(row - 1);
				getnstr(commandbuffer, 256);
				currentFile = commandbuffer;
			}
			std::ofstream outputFile;

			outputFile.open(currentFile);

			if (!outputFile.is_open())
			{
				feedback = "fate";
				goto cancel;
			}

			for (std::string line : buffer)
			{
				outputFile << line << std::endl;
			}

			saved = true;
			feedback = "Saved.";
		}
		
		goto itsawrap;
cancel:
		feedback = "ok then";
itsawrap:
		printfeedback(feedback.data());

		refresh();
	}
	
	endwin();
	
	free(commandbuffer);

	return 0;
}
