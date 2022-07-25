#ifndef BUILTIN_H
#define BUILTIN_H

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <limits>
#include <algorithm>

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

using namespace std;

// Foreground colors
const std::string blackFGColor = "\x1B[30m";
const std::string redFGColor = "\x1B[31m";
const std::string greenFGColor = "\x1B[32m";
const std::string yellowFGColor = "\x1B[33m";
const std::string blueFGColor = "\x1B[34m";
const std::string magentaFGColor = "\x1B[35m";
const std::string cyanFGColor = "\x1B[36m";
const std::string whiteFGColor = "\x1B[37m";
const std::string brightBlackFGColor = "\x1B[90m";
const std::string brightRedFGColor = "\x1B[91m";
const std::string brightGreenFGColor = "\x1B[92m";
const std::string brightYellowFGColor = "\x1B[93m";
const std::string brightBlueFGColor = "\x1B[94m";
const std::string brightMagentaFGColor = "\x1B[95m";
const std::string brightCyanFGColor = "\x1B[96m";
const std::string brightWhiteFGColor = "\x1B[97m";
//Background colors
const std::string blackBGColor = "\x1B[40m";
const std::string redBGColor = "\x1B[41m";
const std::string greenBGColor = "\x1B[42m";
const std::string yellowBGColor = "\x1B[43m";
const std::string blueBGColor = "\x1B[44m";
const std::string magentaBGColor = "\x1B[45m";
const std::string cyanBGColor = "\x1B[46m";
const std::string whiteBGColor = "\x1B[47m";
const std::string brightBlackBGColor = "\x1B[100m";
const std::string brightRedBGColor = "\x1B[101m";
const std::string brightGreenBGColor = "\x1B[102m";
const std::string brightYellowBGColor = "\x1B[103m";
const std::string brightBlueBGColor = "\x1B[104m";
const std::string brightMagentaBGColor = "\x1B[105m";
const std::string brightCyanBGColor = "\x1B[106m";
const std::string brightWhiteBGColor = "\x1B[107m";
// Reset color
const std::string resetColor = "\033[0m";


// trim from start (in place)
static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
static inline string trim(std::string s) {
	string ss = s;
	ltrim(ss);
	rtrim(ss);
	return ss;
}

vector<string> split(string str, string token) {
	vector<string>result;
	while (str.size()) {
		int index = str.find(token);
		if (index != string::npos) {
			result.push_back(str.substr(0, index));
			str = str.substr(index + token.size());
			if (str.size() == 0)result.push_back(str);
		}
		else {
			result.push_back(str);
			str = "";
		}
	}
	return result;
}

string IndentText(string text) {
	vector<string> nstr = split(text, "\n");
	string outStr = "";
	for (size_t i = 0; i < nstr.size(); i++)
	{
		outStr += "	" + nstr[i] + "\n";
	}

	return outStr;
}

string JoinRange(vector<string> strs, int start, int max) {
	string outStr = "";
	for (size_t i = start; i < strs.size()&&i<=max; i++)
	{
		outStr += strs[i]+" ";
	}

	return outStr;
}


bool AccomodateSetInProgramRange(string entireLine, int currentLineCount) {
	string command = split(entireLine, " ")[0];
	if (trim(command) != "set") // Not 'set', passes test
		return true;

	int memAddr = stoi(trim(split(entireLine, " ")[1]));
	//PrintColored(entireLine, yellowFGColor, "");
	//cout  << endl;

	if (memAddr <= currentLineCount + 1) // If it is 'set', then it will increment counter IF the memory location is in program mem
		return true;
	else
		return false; // If just a normal 'set', passes and doesn't increment counter.
}

void PrintColored(std::string text, std::string fgColor, std::string bgColor)
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

void ColorAndPrintAssembly(std::string asmb) {
	vector<string> nstr = split(asmb, "\n");
	int actualNum = 0;
	for (size_t i = 0; i < nstr.size(); i++)
	{
		if (nstr[i] == "")
			continue;

		// If line is a comment
		if (nstr[i][0] == ',')
			PrintColored("\t\t"+nstr[i] + "\n", brightBlackFGColor, "");
		// Else if uncounted set
		else if(!AccomodateSetInProgramRange(nstr[i], actualNum)) {
				//PrintColored("\t"+ to_string(actualNum), yellowFGColor, "");
				PrintColored("\t\t"+split(nstr[i], " ")[0], greenFGColor, "");
				PrintColored(" " + JoinRange(split(nstr[i], " "), 1, 9999) + "\n", brightMagentaFGColor, "");
		}
		// Else
		else {
			if (split(nstr[i], " ").size() > 1) {
				PrintColored("\t"+ to_string(actualNum), yellowFGColor, "");
				PrintColored("\t"+split(nstr[i], " ")[0], cyanFGColor, "");
				PrintColored(" " + JoinRange(split(nstr[i], " "), 1, 9999) + "\n", brightMagentaFGColor, "");
			}
			else {
				PrintColored("\t" + to_string(actualNum), yellowFGColor, "");
				PrintColored("\t" + nstr[i] + "\n", cyanFGColor, "");
			}
			actualNum++;
		}
	}
}

#endif
