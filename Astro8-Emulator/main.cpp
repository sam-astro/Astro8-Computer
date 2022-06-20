#define OLC_PGE_APPLICATION

#include "olcPixelGameEngine.h"

#include <vector>
#include <algorithm> 
#include <string> 
#include <chrono>

using namespace std::chrono;
using namespace std;

// Override base class with your custom functionality
class Emulator : public olc::PixelGameEngine
{
public:
	Emulator()
	{
		sAppName = "Astro8 Emulator";
	}

public:


	int AReg = 0;
	int BReg = 0;
	int InstructionReg = 0;
	int flags[3] = { 0, 0, 0 };
	int bus = 0;
	int outputReg = 0;
	int memoryIndex = 0;
	int programCounter = 0;

	int imgX = 0;
	int imgY = 0;

	float slowdownAmnt = 1;
	int iterations = 0;

	vector<int> memoryBytes;

	string instructions[16] = { "NOP", "LODA", "LODB", "ADD", "SUB", "OUT", "JMP", "STA", "LDI", "JMPZ", "JMPC", "HLT", "LDAIN", "", "", "" };
	string action = "";
	vector<string> microinstructionData;

	bool OnUserCreate() override
	{
		// Called once at the start, so create things here

			// Gather user inputted code
		cout << ("v Emu. Code input v\n");
		string code = "";
		string line;
		while (true) {
			getline(cin, line);
			if (line.empty()) {
				break;
			}
			code += line + "\n";
		}

		// Generate memory from code and convert from hex to decimal
		vector<string> mbytes = parseCode(code);
		for (int memindex = 0; memindex < mbytes.size(); memindex++)
			memoryBytes.push_back(HexToDec(mbytes[memindex]));

		GenerateMicrocode();

		cout << endl;
		cout << DecToBin(16) << endl;
		cout << HexToDec("ffff") << endl;
		cout << HexToBin("ffff", 17) << endl;
		cout << endl;


		return true;
	}

	steady_clock::time_point start;
	bool OnUserUpdate(float fElapsedTime) override
	{
		if (iterations % 1000 == 0)
			start = high_resolution_clock::now();

		//cout << programCounter << ")  ";
		for (int step = 0; step < 7; step++)
		{
			//cout << "step:" << step << endl;
			int microcodeLocation = BinToDec(DecToBinFilled(InstructionReg, 16).substr(0, 4) + DecToBinFilled(step, 4) + to_string(flags[0]) + to_string(flags[1]));
			char* mcode = (char*)microinstructionData[microcodeLocation].c_str();

			//cout<<("     microcode: " + mcode)<<endl;

			if (step == 0)
			{
				// CR
				// AW
				memoryIndex = programCounter;
				// RM
				// IW
				InstructionReg = memoryBytes[memoryIndex];
				// CE
				programCounter += 1;
				step = 1;
				continue;
			}

			//cout << ("\nmcLoc- " + DecToBinFilled(InstructionReg, 16).substr(0, 4) + DecToBinFilled(step, 4) + to_string(flags[0]) + to_string(flags[1])) << "  ==  " << microcodeLocation << endl;
			//cout << ("mcDat- " + mcode) << endl;

			//while (memoryIndex >= 4000)
			//    memoryIndex -= 4000;
			//if (memoryIndex < 0)
			//    memoryIndex = -memoryIndex;

			//cout<<("ftmem=" + memoryIndex);
			// 0-su  1-iw  2-dw  3-st  4-ce  5-cr  6-wm  7-ra  8-eo  9-fl  10-j  11-wb  12-wa  13-rm  14-aw  15-ir  16-ei
			// Execute microinstructions
			if (mcode[8] == '1')
			{ // EO
				//cout << ("EO ");
				if (mcode[0] == '1') // SU
				{
					flags[0] = 0;
					flags[1] = 1;
					if (AReg - BReg == 0)
						flags[0] = 1;
					bus = AReg - BReg;
					if (bus < 0)
					{
						bus = 65535 + bus;
						flags[1] = 0;
					}
				}
				else
				{
					flags[0] = 0;
					flags[1] = 0;
					if (AReg + BReg == 0)
						flags[0] = 1;
					bus = AReg + BReg;
					if (bus >= 65535)
					{
						bus = bus - 65535;
						flags[1] = 1;
					}
				}
			}
			if (mcode[5] == '1')
			{ // CR
				//cout << ("CR ");
				bus = programCounter;
			}
			if (mcode[7] == '1')
			{ // RA
				//cout << ("RA ");
				bus = AReg;
			}
			if (mcode[13] == '1')
			{ // RM
				//cout << ("RM ");
				//cout << "\nmemread: " + to_string(memoryIndex )+ " " + to_string(memoryBytes[memoryIndex] )+ "\n";
				bus = memoryBytes[memoryIndex];
			}
			if (mcode[15] == '1')
			{ // IR
				//cout << ("IR ");
				bus = BinToDec(DecToBinFilled(InstructionReg, 16).substr(4, 12));
			}
			if (mcode[1] == '1')
			{ // IW
				//cout << ("IW ");
				InstructionReg = bus;
			}
			if (mcode[2] == '1')
			{ // DW
				//cout << ("DW ");
				outputReg = bus;
				//cout << ("\no: " + to_string(outputReg) + " A: " + to_string(AReg) + " B: " + to_string(BReg) + " bus: " + to_string(bus) + " Ins: " + to_string(InstructionReg) + " img:(" + to_string(imgX) + ", " + to_string(imgY) + ")\n");

				// Write to LED screen
				int r = BinToDec(DecToBinFilled(bus, 16).substr(1, 5)) * 8;
				int g = BinToDec(DecToBinFilled(bus, 16).substr(6, 5)) * 8;
				int b = BinToDec(DecToBinFilled(bus, 16).substr(11, 5)) * 8;
				Draw(imgX, imgY, olc::Pixel(r, g, b));

				imgX++;
				if (imgX >= 64)
				{
					imgY++;
					imgX = 0;
				}
				if (imgY >= 64)
				{
					imgY = 0;
				}

			}
			if (mcode[4] == '1')
			{ // CE
				//cout << ("CE ");
				programCounter += 1;
			}
			if (mcode[6] == '1')
			{ // WM
				//cout << ("WM ");
				memoryBytes[memoryIndex] = bus;
			}
			if (mcode[10] == '1')
			{ // J
				//cout << ("J ");
				//cout<<Line(DecToBinFilled(InstructionReg, 16));
				//cout<<Line(DecToBinFilled(InstructionReg, 16).Substring(4, 12));
				programCounter = BinToDec(DecToBinFilled(InstructionReg, 16).substr(4, 12));
			}
			if (mcode[11] == '1')
			{ // WB
				//cout << ("WB ");
				BReg = bus;
			}
			if (mcode[12] == '1')
			{ // WA
				//cout << ("WA ");
				AReg = bus;
			}
			if (mcode[14] == '1')
			{ // AW
				//cout << ("AW ");
				memoryIndex = BinToDec(DecToBinFilled(bus, 16).substr(4, 12));
			}
			if (mcode[3] == '1')
			{ // ST
				//cout<<("ST ");
				cout << ("\n== PAUSED from HLT ==\n\n");
				cout << ("FINAL VALUES |=  o: " + to_string(outputReg) + " A: " + to_string(AReg) + " B: " + to_string(BReg) + " bus: " + to_string(bus) + " Ins: " + to_string(InstructionReg) + " img:(" + to_string(imgX) + ", " + to_string(imgY) + ")\n");
				system("pause");
				exit(1);
			}

			if (mcode[16] == '1')
			{ // EI
				//cout<<("EI ");
				//cout<<endl;
				break;
			}
			//else
			//	cout<<endl;
		}

		if (iterations % 1000 == 0) {
			auto stop = high_resolution_clock::now();
			cout << "\r                " << "\r" << round((1.0f / fElapsedTime) * 10.0f) / 10.0f << "\thz";
		}

		//cout << ("o: " + to_string(outputReg) + " A: " + to_string(AReg) + " B: " + to_string(BReg) + " bus: " + to_string(bus) + " Ins: " + to_string(InstructionReg) + " img:(" + to_string(imgX) + ", " + to_string(imgY) + ")\n");
	//}
		iterations += 1;

		return true;
	}

	string charToString(char* a)
	{
		string s(a);
		return s;
	}

	string DecToHexFilled(int input, int desiredSize)
	{
		stringstream ss;
		ss << hex << input;
		string output(ss.str());

		while (output.length() < desiredSize)
		{
			output = "0" + output;
		}

		return output;
	}
	string BinToHexFilled(string input, int desiredSize)
	{
		int dec = BinToDec(input);
		string output = DecToHexFilled(dec, 0);

		while (output.length() < desiredSize)
		{
			output = "0" + output;
		}

		return output;
	}
	int BinToDec(string input)
	{
		return stoi(input, nullptr, 2);
	}
	string DecToBin(int input)
	{
		string r;
		int n = input;
		while (n != 0) { r = (n % 2 == 0 ? "0" : "1") + r; n /= 2; }
		return r;
	}
	string DecToBinFilled(int input, int desiredSize)
	{
		string output = DecToBin(input);

		while (output.length() < desiredSize)
		{
			output = "0" + output;
		}

		return output;
	}
	string HexToBin(string s, int desiredSize)
	{
		string out;
		for (auto i : s) {
			uint8_t n;
			if (i <= '9' and i >= '0')
				n = i - '0';
			else
				n = 10 + i - 'A';
			for (int8_t j = 3; j >= 0; --j)
				out.push_back((n & (1 << j)) ? '1' : '0');
		}

		// Fill
		while (out.length() < desiredSize)
		{
			out = "0" + out;
		}
		if (out.length() > desiredSize)
			out = out.substr(out.length() - desiredSize);
		return out;
	}

	int HexToDec(string hex)
	{
		unsigned long result = 0;
		for (int i = 0; i < hex.length(); i++) {
			if (hex[i] >= 48 && hex[i] <= 57)
			{
				result += (hex[i] - 48) * pow(16, hex.length() - i - 1);
			}
			else if (hex[i] >= 65 && hex[i] <= 70) {
				result += (hex[i] - 55) * pow(16, hex.length() - i - 1);
			}
			else if (hex[i] >= 97 && hex[i] <= 102) {
				result += (hex[i] - 87) * pow(16, hex.length() - i - 1);
			}
		}
		return result;
	}

	vector<string> explode(const string& str, const char& ch) {
		string next;
		vector<string> result;

		// For each character in the string
		for (string::const_iterator it = str.begin(); it != str.end(); it++) {
			// If we've hit the terminal character
			if (*it == ch) {
				// If we have some characters accumulated
				if (!next.empty()) {
					// Add them to the result vector
					result.push_back(next);
					next.clear();
				}
			}
			else {
				// Accumulate the next character into the sequence
				next += *it;
			}
		}
		if (!next.empty())
			result.push_back(next);
		return result;
	}

	vector<string> parseCode(string input)
	{
		vector<string> outputBytes;
		for (int i = 0; i < 4000; i++)
			outputBytes.push_back("0000");

		string icopy = input;
		transform(icopy.begin(), icopy.end(), icopy.begin(), ::toupper);
		vector<string> splitcode = explode(icopy, '\n');

		cout << endl;

		int memaddr = 0;
		for (int i = 0; i < splitcode.size(); i++)
		{
			if (trim(splitcode[i]) == "")
			{
				continue;
			}

			vector<string> splitBySpace = explode(splitcode[i], ' ');

			if (splitBySpace[0][0] == ',')
			{
				cout << ("-\t" + splitcode[i] + "\n");
				continue;
			}
			if (splitBySpace[0] == "SET")
			{
				string hVal = DecToHexFilled(stoi(splitBySpace[2]), 4);
				outputBytes[stoi(splitBySpace[1])] = hVal;
				cout << ("-\t" + splitcode[i] + "\t  ~   ~\n");
				continue;
			}

			cout << (to_string(memaddr) + " " + splitcode[i] + "   \t  =>  ");

			// Find index of instruction
			for (int f = 0; f < sizeof(instructions) / sizeof(instructions[0]); f++)
			{
				if (instructions[f] == splitBySpace[0])
				{
					cout << (DecToHexFilled(f, 1));
					outputBytes[memaddr] = DecToHexFilled(f, 1);
				}
			}

			// Check if any args are after the command
			if (splitcode[i] != splitBySpace[0])
			{
				cout << (DecToHexFilled(stoi(splitBySpace[1]), 3));
				outputBytes[memaddr] += DecToHexFilled(stoi(splitBySpace[1]), 3);
			}
			else
			{
				cout << ("0");
				outputBytes[memaddr] += "000";
			}
			cout << ("\n");
			memaddr += 1;
		}

		/*
		cout << ("\n000:");
		for (int outindex = 0; outindex < outputBytes.size(); outindex++)
		{
			if (outindex % 8 == 0 && outindex != 0)
			{
				string locationTmp = DecToHexFilled(outindex, 3);
				transform(locationTmp.begin(), locationTmp.end(), locationTmp.begin(), ::toupper);
				cout << ("\n" + locationTmp + ":");
			}
			string bytetmp = (" " + outputBytes[outindex]);
			transform(bytetmp.begin(), bytetmp.end(), bytetmp.begin(), ::toupper);
			cout << bytetmp;
		}*/
		return outputBytes;
	}

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

	void GenerateMicrocode()
	{
		// Generate zeros in data
		vector<string> output;
		for (int osind = 0; osind < 1024; osind++) { output.push_back("00000"); microinstructionData.push_back(""); }

		string microinstructions[] = { "SU", "IW", "DW", "ST", "CE", "CR", "WM", "RA", "EO", "FL", "J", "WB", "WA", "RM", "AW", "IR", "EI" };
		string flags[] = { "ZEROFLAG", "CARRYFLAG" };
		string instructioncodes[] = {
				"fetch( 0=aw,cr & 1=rm,iw,ce & 2=ei", // Fetch
				"loda( 2=aw,ir & 3=wa,rm & 4=ei", // LoadA
				"lodb( 2=aw,ir & 3=wb,rm & 4=ei", // LoadB
				"add( 2=aw,ir & 3=wb,rm & 4=wa,eo,fl & 5=ei", // Add <addr>
				"sub( 2=aw,ir & 3=rm,wb & 4=wa,eo,su,fl & 5=ei", // Subtract <addr>
				"out( 2=ra,dw & 3=ei", // Output to decimal display and LCD screen
				"jmp( 2=ir,j & 3=ei", // Jump <addr>
				"sta( 2=aw,ir & 3=ra,wm & 4=ei", // Store A <addr>
				"ldi( 2=wa,ir & 3=ei", // Load immediate A <val>
				"jmpz( 2=ir,j | zeroflag & 3=ei", // Jump if zero <addr>
				"jmpc( 2=ir,j | carryflag & 3=ei", // Jump if carry <addr>
				"hlt( 2=st & 3=ei", // Stop the computer clock
				"ldain( 2=ra,aw & 3=wa,rm & 4=ei", // Load from reg A as memory address, then copy value from memory into A
		};

		// Remove spaces from instruction codes and make uppercase
		for (int cl = 0; cl < sizeof(instructioncodes) / sizeof(instructioncodes[0]); cl++)
		{
			string newStr = "";
			for (int clc = 0; clc < instructioncodes[cl].length(); clc++)
			{
				if (instructioncodes[cl][clc] != ' ')
					newStr += instructioncodes[cl][clc];
			}
			transform(newStr.begin(), newStr.end(), newStr.begin(), ::toupper);
			cout << (newStr) << endl;
			instructioncodes[cl] = newStr;
		}

		// Create indexes for instructions, which allows for duplicates to execute differently for different parameters
		int instIndexes[sizeof(instructioncodes) / sizeof(instructioncodes[0])];
		vector<string> seenNames;
		for (int cl = 0; cl < sizeof(instructioncodes) / sizeof(instructioncodes[0]); cl++)
		{
			string instName = explode(instructioncodes[cl], '(')[0];
			bool foundInList = false;
			for (int clc = 0; clc < seenNames.size(); clc++)
			{
				if (instName == seenNames[clc])
				{
					instIndexes[cl] = clc;
					foundInList = true;
					break;
				}
			}
			if (!foundInList)
			{
				seenNames.push_back(instName);
				instIndexes[cl] = seenNames.size() - 1;
			}
			instructioncodes[cl] = explode(instructioncodes[cl], '(')[1];
		}

		// Special process fetch instruction
		cout << ("\n" + instructioncodes[0] + "\n");
		for (int ins = 0; ins < sizeof(instructioncodes) / sizeof(instructioncodes[0]); ins++) // Iterate through all definitions of instructions
		{
			int correctedIndex = instIndexes[ins];

			string startaddress = DecToBinFilled(correctedIndex, 4);

			vector<string> instSteps = explode(instructioncodes[0], '&');
			for (int step = 0; step < instSteps.size(); step++) // Iterate through every step
			{
				int actualStep = stoi(explode(instSteps[step], '=')[0]);
				string stepContents = explode(explode(instSteps[step], '=')[1], '|')[0];

				string midaddress = DecToBinFilled(actualStep, 4);

				string stepComputedInstruction = "";
				for (int mins = 0; mins < sizeof(microinstructions) / sizeof(microinstructions[0]); mins++)
				{
					if (stepContents.find(microinstructions[mins]) != std::string::npos)
						stepComputedInstruction += "1";
					else
						stepComputedInstruction += "0";
				}

				// Compute flags combinations
				for (int flagcombinations = 0; flagcombinations < (sizeof(flags) / sizeof(flags[0])) * (sizeof(flags) / sizeof(flags[0])); flagcombinations++)
				{
					char endaddress[] = { '0', '0' };
					// Look for flags
					if (instSteps[step].find("|") != std::string::npos)
					{
						vector<string> inststepFlags = explode(explode(instSteps[step], '|')[1], ',');
						for (int flag = 0; flag < inststepFlags.size(); flag++) // Iterate through all flags in step
						{
							for (int checkflag = 0; checkflag < (sizeof(flags) / sizeof(flags[0])); checkflag++) // What is the index of the flag
							{
								if (inststepFlags[flag] == flags[checkflag])
									endaddress[checkflag] = '1';
							}
						}
					}
					string tmpFlagCombos = DecToBinFilled(flagcombinations, 2);
					char* newendaddress = (char*)tmpFlagCombos.c_str();

					bool doesntmatch = false;
					for (int i = 0; i < (sizeof(endaddress) / sizeof(endaddress[0])); i++)
					{
						if (endaddress[i] == '1')
						{
							if (newendaddress[i] != '1')
								doesntmatch = true;
						}
					}
					if (doesntmatch)
						continue;

					cout << ("\t& " + startaddress + " " + midaddress + " " + charToString(newendaddress) + "  =  " + BinToHexFilled(stepComputedInstruction, 4) + "\n");
					output[BinToDec(startaddress + midaddress + charToString(newendaddress))] = BinToHexFilled(stepComputedInstruction, 5);
				}
			}

			//cout<<Line();
		}

		// Do actual processing
		for (int ins = 1; ins < (sizeof(instructioncodes) / sizeof(instructioncodes[0])); ins++) // Iterate through all definitions of instructions
		{
			int correctedIndex = instIndexes[ins];

			cout << (instructioncodes[correctedIndex] + "\n");

			string startaddress = DecToBinFilled(correctedIndex, 4);

			vector<string> instSteps = explode(instructioncodes[correctedIndex], '&');
			for (int step = 0; step < instSteps.size(); step++) // Iterate through every step
			{
				int actualStep = stoi(explode(instSteps[step], '=')[0]);
				string stepContents = explode(explode(instSteps[step], '=')[1], '|')[0];

				string midaddress = DecToBinFilled(actualStep, 4);

				string stepComputedInstruction = "";
				for (int mins = 0; mins < (sizeof(microinstructions) / sizeof(microinstructions[0])); mins++)
				{
					if (stepContents.find(microinstructions[mins]) != std::string::npos)
						stepComputedInstruction += "1";
					else
						stepComputedInstruction += "0";
				}

				// Compute flags combinations
				for (int flagcombinations = 0; flagcombinations < (sizeof(flags) / sizeof(flags[0])) * (sizeof(flags) / sizeof(flags[0])); flagcombinations++)
				{
					char endaddress[] = { '0', '0' };
					int stepLocked[] = { 0, 0 };
					// If flags are specified in current step layer, set them to what is specified and lock that bit
					if (instSteps[step].find("|") != std::string::npos)
					{
						vector<string> inststepFlags = explode(explode(instSteps[step], '|')[1], ',');
						for (int flag = 0; flag < inststepFlags.size(); flag++) // Iterate through all flags in step
						{
							for (int checkflag = 0; checkflag < (sizeof(flags) / sizeof(flags[0])); checkflag++) // What is the index of the flag
							{
								if (inststepFlags[flag].find(flags[checkflag]) != std::string::npos)
								{
									if (inststepFlags[flag][0] == '!')
										endaddress[checkflag] = '0';
									else
										endaddress[checkflag] = '1';
									stepLocked[checkflag] = 1;
								}
							}
						}
					}
					string tmpFlagCombos = DecToBinFilled(flagcombinations, 2);
					char* newendaddress = (char*)tmpFlagCombos.c_str();

					// Make sure the current combination doesn't change the locked bits, otherwise go to next step
					bool doesntmatch = false;
					for (int i = 0; i < (sizeof(endaddress) / sizeof(endaddress[0])); i++)
					{
						if (stepLocked[i] == 1)
						{
							if (newendaddress[i] != endaddress[i])
								doesntmatch = true;
						}
					}
					if (doesntmatch)
						continue;

					cout << ("\t& " + startaddress + " " + midaddress + " " + charToString(newendaddress) + "  =  " + BinToHexFilled(stepComputedInstruction, 5));
					cout << endl;
					output[BinToDec(startaddress + midaddress + charToString(newendaddress))] = BinToHexFilled(stepComputedInstruction, 5);
				}
			}

			//cout<<Line();
		}


		string processedOutput = "";

		// Print the output
		cout << ("\nv3.0 hex words addressed\n");
		processedOutput += "\nv3.0 hex words addressed\n";
		cout << ("000: ");
		processedOutput += "000: ";
		for (int outindex = 0; outindex < output.size(); outindex++)
		{
			if (outindex % 8 == 0 && outindex != 0)
			{
				string locationTmp = DecToHexFilled(outindex, 3);
				transform(locationTmp.begin(), locationTmp.end(), locationTmp.begin(), ::toupper);
				cout << ("\n" + locationTmp + ": ");
				processedOutput += "\n" + DecToHexFilled(outindex, 3) + ": ";
			}
			cout << (output[outindex] + " ");
			processedOutput += output[outindex] + " ";

			string ttmp = output[outindex];
			transform(ttmp.begin(), ttmp.end(), ttmp.begin(), ::toupper);
			microinstructionData[outindex] = HexToBin(ttmp, 17);
		}
	}
};

int main()
{
	Emulator displayWindow;
	if (displayWindow.Construct(64, 64, 5, 5))
		displayWindow.Start();
	return 0;
}
