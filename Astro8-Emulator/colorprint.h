#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <regex>
#include <string>

#include "strops.h"

#if defined(__unix__)
	#define UNIX true
	#define WINDOWS false
#elif defined(_MSC_VER)
	#define UNIX false
	#define WINDOWS true
#endif


#if WINDOWS
	#include "color.hpp"
#endif

#include "processing.h"

using namespace std;

// Foreground colors
static const std::string blackFGColor = "\x1B[30m";
static const std::string redFGColor = "\x1B[31m";
static const std::string greenFGColor = "\x1B[32m";
static const std::string yellowFGColor = "\x1B[33m";
static const std::string blueFGColor = "\x1B[34m";
static const std::string magentaFGColor = "\x1B[35m";
static const std::string cyanFGColor = "\x1B[36m";
static const std::string whiteFGColor = "\x1B[37m";
static const std::string brightBlackFGColor = "\x1B[90m";
static const std::string brightRedFGColor = "\x1B[91m";
static const std::string brightGreenFGColor = "\x1B[92m";
static const std::string brightYellowFGColor = "\x1B[93m";
static const std::string brightBlueFGColor = "\x1B[94m";
static const std::string brightMagentaFGColor = "\x1B[95m";
static const std::string brightCyanFGColor = "\x1B[96m";
static const std::string brightWhiteFGColor = "\x1B[97m";
//Background colors
static const std::string blackBGColor = "\x1B[40m";
static const std::string redBGColor = "\x1B[41m";
static const std::string greenBGColor = "\x1B[42m";
static const std::string yellowBGColor = "\x1B[43m";
static const std::string blueBGColor = "\x1B[44m";
static const std::string magentaBGColor = "\x1B[45m";
static const std::string cyanBGColor = "\x1B[46m";
static const std::string whiteBGColor = "\x1B[47m";
static const std::string brightBlackBGColor = "\x1B[100m";
static const std::string brightRedBGColor = "\x1B[101m";
static const std::string brightGreenBGColor = "\x1B[102m";
static const std::string brightYellowBGColor = "\x1B[103m";
static const std::string brightBlueBGColor = "\x1B[104m";
static const std::string brightMagentaBGColor = "\x1B[105m";
static const std::string brightCyanBGColor = "\x1B[106m";
static const std::string brightWhiteBGColor = "\x1B[107m";
// Reset color
static const std::string resetColor = "\033[0m";


static bool AccomodateSetInProgramRange(std::string entireLine, int currentLineCount)
{
	std::string command = split(entireLine, " ")[0];
	if (trim(command) != "set")	 // Not 'set', passes test
		return true;

	int memAddr = stoi(trim(split(entireLine, " ")[1]));
	//PrintColored(entireLine, yellowFGColor, "");
	//cout  << endl;

	if (split(entireLine, " ").size() > 3)
		if (stoi(trim(split(entireLine, " ")[3])) != 0)
			return false;

	if (memAddr <= currentLineCount + 1)  // If it is 'set', then it will increment counter IF the memory location is in program mem
		return true;
	else
		return false;  // If just a normal 'set', passes and doesn't increment counter.
}

static void PrintColored(std::string text, std::string fgColor, std::string bgColor)
{
#if WINDOWS
	// Normal FG colors
	if (fgColor == blackFGColor)
		cout << hue::black;
	else if (fgColor == redFGColor)
		cout << hue::red;
	else if (fgColor == greenFGColor)
		cout << hue::green;
	else if (fgColor == yellowFGColor)
		cout << hue::yellow;
	else if (fgColor == blueFGColor)
		cout << hue::blue;
	else if (fgColor == magentaFGColor)
		cout << hue::purple;
	else if (fgColor == cyanFGColor)
		cout << hue::aqua;
	else if (fgColor == whiteFGColor)
		cout << hue::white;
	// Check if bright FG colors
	else if (fgColor == brightBlackFGColor)
		cout << hue::grey;
	else if (fgColor == brightRedFGColor)
		cout << hue::light_red;
	else if (fgColor == brightGreenFGColor)
		cout << hue::light_green;
	else if (fgColor == brightYellowFGColor)
		cout << hue::light_yellow;
	else if (fgColor == brightBlueFGColor)
		cout << hue::light_blue;
	else if (fgColor == brightMagentaFGColor)
		cout << hue::light_purple;
	else if (fgColor == brightCyanFGColor)
		cout << hue::light_aqua;
	else if (fgColor == brightWhiteFGColor)
		cout << hue::bright_white;

	// Normal BG colors
	if (bgColor == blackBGColor)
		cout << hue::on_black;
	else if (bgColor == redBGColor)
		cout << hue::on_red;
	else if (bgColor == greenBGColor)
		cout << hue::on_green;
	else if (bgColor == yellowBGColor)
		cout << hue::on_yellow;
	else if (bgColor == blueBGColor)
		cout << hue::on_blue;
	else if (bgColor == magentaBGColor)
		cout << hue::on_purple;
	else if (bgColor == cyanBGColor)
		cout << hue::on_aqua;
	else if (bgColor == whiteBGColor)
		cout << hue::on_white;
	// Check if bright BG colors
	else if (bgColor == brightBlackBGColor)
		cout << hue::on_grey;
	else if (bgColor == brightRedBGColor)
		cout << hue::on_light_red;
	else if (bgColor == brightGreenBGColor)
		cout << hue::on_light_green;
	else if (bgColor == brightYellowBGColor)
		cout << hue::on_light_yellow;
	else if (bgColor == brightBlueBGColor)
		cout << hue::on_light_blue;
	else if (bgColor == brightMagentaBGColor)
		cout << hue::on_light_purple;
	else if (bgColor == brightCyanBGColor)
		cout << hue::on_light_aqua;
	else if (bgColor == brightWhiteBGColor)
		cout << hue::on_bright_white;

	std::cout << text << hue::reset;
#else
	cout << fgColor + bgColor + text + resetColor;
#endif
}

static void ColorAndPrintAssembly(std::string asmb, vector<std::string> instructions)
{
	cout << instructions.size() << endl;

	vector<std::string> nstr = split(asmb, "\n");
	int actualNum = 0;
	for (size_t i = 0; i < nstr.size(); i++) {
		if (nstr[i] == "")
			continue;

		// If line is a comment
		if (nstr[i][0] == ',')
			PrintColored("\t\t" + nstr[i] + "\n", brightBlackFGColor, "");
		// Else if uncounted set
		else if (!AccomodateSetInProgramRange(nstr[i], actualNum)) {
			//PrintColored("\t"+ to_string(actualNum), yellowFGColor, "");
			PrintColored("\t\t" + split(nstr[i], " ")[0], greenFGColor, "");
			PrintColored(" " + JoinRange(split(nstr[i], " "), 1, 9999) + "\n", brightMagentaFGColor, "");
		}
		else if (StringStartsWith(nstr[i], "const") || nstr[i][0] == '#') {
			PrintColored("\t\t" + split(nstr[i], " ")[0], greenFGColor, "");
			PrintColored(" " + JoinRange(split(nstr[i], " "), 1, 9999) + "\n", brightMagentaFGColor, "");
		}
		// Else
		else {
			bool matchesCommand = false;

			std::string instruction = split(nstr[i], " ")[0];
			transform(instruction.begin(), instruction.end(), instruction.begin(), ::toupper);

			for (int i = 0; i < instructions.size(); i++) {
				if (instruction == instructions[i]) {
					matchesCommand = true;
					break;
				}
			}

			// If a valid instruction, print normally
			if (matchesCommand || instruction == "SET" || instruction == "HERE" || instruction == "ALLOC")
				if (split(nstr[i], " ").size() > 1) {
					PrintColored("\t" + to_string(actualNum), yellowFGColor, "");
					PrintColored("\t" + split(nstr[i], " ")[0], cyanFGColor, "");
					try {
						stoi(split(nstr[i], " ")[1]);
						PrintColored(" " + split(nstr[i], " ")[1], brightMagentaFGColor, "");
					}
					catch (exception) {	 // If the argument is not an integer, it is a variable
						PrintColored(" " + split(nstr[i], " ")[1], greenFGColor, "");
					}
					if (split(nstr[i], " ").size() > 2)
						try {
							stoi(split(nstr[i], " ")[2]);
							PrintColored(" " + split(nstr[i], " ")[2], brightMagentaFGColor, "");
						}
						catch (exception) {	 // If the argument is not an integer, it is a variable
							PrintColored(" " + split(nstr[i], " ")[2], greenFGColor, "");
						}
					PrintColored("\n", cyanFGColor, "");
				}
				else {
					PrintColored("\t" + to_string(actualNum), yellowFGColor, "");
					PrintColored("\t" + nstr[i] + "\n", cyanFGColor, "");
				}
			else if (instruction == "CONST")
				if (split(nstr[i], " ").size() > 1) {
					PrintColored("\t" + to_string(actualNum), yellowFGColor, "");
					PrintColored("\t" + split(nstr[i], " ")[0], cyanFGColor, "");
					PrintColored(" " + split(nstr[i], " ")[1], greenFGColor, "");
					PrintColored(" " + JoinRange(split(nstr[i], " "), 2, 9999) + "\n", brightMagentaFGColor, "");
				}
				else {
					PrintColored(" !!", redFGColor, "");
					PrintColored("\t" + to_string(actualNum), yellowFGColor, "");
					PrintColored("\t" + instruction + "\n", redFGColor, "");
				}
			else if (instruction.at(instruction.size() - 1) == ':') {
				PrintColored("\t" + to_string(actualNum), yellowFGColor, "");
				PrintColored("\t" + split(nstr[i], " ")[0], greenFGColor, "");
				PrintColored(" " + JoinRange(split(nstr[i], " "), 1, 9999) + "\n", brightMagentaFGColor, "");
			}
			// If not a valid instruction, print in red
			else {
				PrintColored(" !!", redFGColor, "");
				PrintColored("\t" + to_string(actualNum), yellowFGColor, "");
				PrintColored("\t" + instruction + "\n", redFGColor, "");
				/*cout << "\n\nPress Enter to Exit...";
				cin.ignore();
				exit(1);*/
			}

			actualNum++;
		}
	}
}
