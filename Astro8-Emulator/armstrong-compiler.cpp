#pragma once


#include "armstrong-compiler.h"


vector<string> vars;
vector<string> labels;
vector<int> labelLineValues;
vector<string> compiledLines;



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
string BinToHexFilled(const string& input, int desiredSize)
{
	int dec = BinToDec(input);
	string output = DecToHexFilled(dec, 0);

	while (output.length() < desiredSize)
	{
		output = "0" + output;
	}

	return output;
}
int BinToDec(const string& input)
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
string HexToBin(const string& s, int desiredSize)
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

int HexToDec(const string& hex)
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

bool IsHex(const string& in) {
	if (in.size() > 2)
		if (in[0] == '0' && in[1] == 'x')
			return true;
	return false;
}
bool IsBin(const string& in) {
	if (in.size() > 2)
		if (in[0] == '0' && in[1] == 'b')
			return true;
	return false;
}
bool IsReg(const string& in) {
	if (in.size() > 1)
		if (in[0] == '@')
			return true;
	return false;
}
bool IsVar(const string& in) {
	if (in.size() > 1)
		if (in[0] == '$')
			return true;
	return false;
}
bool IsLabel(const string& in) {
	if (in.size() > 1)
		if (in[0] == '#')
			return true;
	return false;
}
bool IsPointer(const string& in) {
	if (in.size() > 1)
		if (in[0] == '*')
			return true;
	return false;
}
bool IsDec(const string& in) {
	if (!IsHex(in) && !IsBin(in) && !IsReg(in) && !IsVar(in) && !IsLabel(in) && !IsPointer(in))
		return true;
	return false;
}

void PutSetOnCurrentLine(const string& value) {
	compiledLines.push_back("here " + value);
}

// Loading of memory value into register, automatically allowing large addressing as needed
void LoadAddress(const string& reg, const string& address) {
	int actualVal = ParseValue(address);
	string addrInWord = "ain ";
	int actualLineNum = GetLineNumber();

	if (reg == "@A")
		addrInWord = "ain ";
	else if (reg == "@B")
		addrInWord = "bin ";
	else if (reg == "@C")
		addrInWord = "cin ";
	else if (reg == "@EX")
		addrInWord = "ain ";

	// Value is small enough to be accessible through normal r/w instructions
	if (actualVal <= 2047) {
		compiledLines.push_back(addrInWord + to_string(actualVal));
		if (reg == "@EX")
			compiledLines.push_back("wrexp");
	}
	// Value is too large to be accessible through normal r/w instructions, use LGE style
	else if (actualVal > 2047) {
		compiledLines.push_back("ldlge");
		PutSetOnCurrentLine(to_string(actualVal));
		compiledLines.push_back(MoveFromRegToReg("@A", reg));
	}

}

// Storing of register into memory, automatically allowing large addressing as needed
void StoreAddress(const string& reg, const string& address) {
	int actualLineNum = GetLineNumber();
	int actualVal = ParseValue(address);
	string addrOutWord = "ain ";

	// Value is small enough to be accessible through normal r/w instructions
	if (actualVal <= 2047)
		compiledLines.push_back(MoveFromRegToReg(reg, "@A") + "sta " + to_string(actualVal));
	// Value is too large to be accessible through normal r/w instructions, use LGE style
	else if (actualVal > 2047) {
		compiledLines.push_back(MoveFromRegToReg(reg, "@A"));
		compiledLines.push_back("stlge");
		PutSetOnCurrentLine(to_string(actualVal));
	}
}

void RegIdToLDI(const string& in, const string& followingValue) {
	int actualValue = ParseValue(followingValue);

	if (actualValue < 2047) {
		if (in == "@A") {
			compiledLines.push_back("ldia " + to_string(actualValue));
		}
		else if (in == "@B") {
			compiledLines.push_back("ldib " + to_string(actualValue));
		}
		else if (in == "@C") {
			compiledLines.push_back("ldia " + to_string(actualValue));
			compiledLines.push_back("swpc");
		}
		else if (in == "@EX") {
			compiledLines.push_back("ldia " + to_string(actualValue));
			compiledLines.push_back("wrexp");
		}
	}
	else {
		if (in == "@A") {
			compiledLines.push_back("ldw");
			PutSetOnCurrentLine(followingValue);
		}
		else if (in == "@B") {
			compiledLines.push_back("ldw");
			PutSetOnCurrentLine(followingValue);
			compiledLines.push_back("swp");
		}
		else if (in == "@C") {
			compiledLines.push_back("ldw");
			PutSetOnCurrentLine(followingValue);
			compiledLines.push_back("swpc");
		}
		else if (in == "@EX") {
			compiledLines.push_back("ldw");
			PutSetOnCurrentLine(followingValue);
			compiledLines.push_back("wrexp");
		}
	}
}

string MoveFromRegToReg(const string& from, const string& destination) {
	if (from == destination)
		return "";

	if (destination == "@A" && from == "@B")
		return "swp\n";
	if (destination == "@A" && from == "@C")
		return "swpc\n";
	if (destination == "@A" && from == "@EX")
		return "rdexp\n";

	if (destination == "@B" && from == "@A")
		return "swp\n";
	if (destination == "@B" && from == "@C")
		return "swpc\nswp\n";
	if (destination == "@B" && from == "@EX")
		return "rdexp\nswp\n";

	if (destination == "@C" && from == "@A")
		return "swpc\n";
	if (destination == "@C" && from == "@B")
		return "swp\nswpc\n";
	if (destination == "@C" && from == "@EX")
		return "rdexp\nswpc\n";

	if (destination == "@EX" && from == "@A")
		return "wrexp\n";
	if (destination == "@EX" && from == "@B")
		return "swp\nwrexp\n";
	if (destination == "@EX" && from == "@C")
		return "swpc\nwrexp\n";

	return "";
}

int ActualLineNumFromNum(int x) {
	string outStr = "";
	for (int i = 0; i < compiledLines.size(); i++)
		outStr += trim(compiledLines[i]) + "\n";

	compiledLines = split(outStr, "\n");
	int outInt = 1;
	int i = 0;
	while (i < compiledLines.size() && i <= x)
	{
		if (trim(compiledLines[i]) != "" && AccomodateSetInProgramRange(compiledLines[i], outInt) && split(compiledLines[i], " ")[0] != "endif" && split(compiledLines[i], " ")[0][0] != ',')
			outInt++;

		i++;
	}
	if (trim(compiledLines[i]) == "" || !AccomodateSetInProgramRange(compiledLines[i], outInt) || split(compiledLines[i], " ")[0] == "endif" || split(compiledLines[i], " ")[0][0] == ',')
		outInt += 1;

	return outInt;
}

int GetVariableAddress(const string& id) {
	// Search all variable names to get index
	for (int i = 0; i < vars.size(); i++)
		if (id == vars[i])
			return i + 16528;

	// Not found, add to list and return size-1
	vars.push_back(id);
	return 16528 + vars.size() - 1;
}

int FindLabelLine(const string& labelName, const vector<string>& labels, const vector<int>& labelLineValues) {
	for (int i = 0; i < labels.size(); i++)
		if (labelName == labels[i])
			return labelLineValues[i];

	// Not found return -1
	return -1;
}

int ParseValue(const string& input) {
	if (input.size() > 2) {
		if (IsHex(input))      // If preceded by '0x', then it is a hex number
			return HexToDec(split(input, "0x")[1]);
		else if (IsBin(input)) // If preceded by '0b', then it is a binary number
			return BinToDec(split(input, "0b")[1]);
	}
	if (IsVar(input)) // If a variable
		return GetVariableAddress(input);
	if (IsDec(input)) // If a decimal number
		return stoi(input);
	if (IsLabel(input)) // If a label
		return FindLabelLine(input, labels, labelLineValues);
	if (IsPointer(input)) // If a pointer
		return ParseValue(split(input, "*")[1]);

	return -1;
}

// Reads from mem at the address stored in pointer, into REG A
void LoadPointer(const string& str) {
	LoadAddress("@A", split(str, "*")[1]);
	compiledLines.push_back("ldain");
}

// Writes from REG B to mem at the address stored in pointer
void StoreIntoPointer(const string& str) {
	LoadAddress("@A", split(str, "*")[1]);
	compiledLines.push_back("staout");
}

void CompareValues(const string& valA, const string& comparer, const string& valB, const vector<string>& vars) {
	int procA = ParseValue(valA);
	int procB = ParseValue(valB);

	// Get into B reg

	// If B is pointer to a memory address
	if (IsPointer(valB)) {
		LoadPointer(valB);
		compiledLines.push_back("swp\n");
	}
	// If B is memory address
	else if (IsHex(valB)) {
		LoadAddress("@B", valB);
	}
	// If B is register
	else if (IsReg(valB)) {
		compiledLines.push_back(MoveFromRegToReg(valB, "@B"));
	}
	// If B is variable
	else if (IsVar(valB)) {
		LoadAddress("@B", to_string(GetVariableAddress(valB)));
	}
	// If B is decimal
	else if (IsDec(valB)) {
		RegIdToLDI("@B", to_string(procB));
	}


	// Get into A reg

	// If A is pointer to a memory address
	if (IsPointer(valA)) {
		LoadPointer(valA);
	}
	// If A is memory address
	else if (IsHex(valA)) {
		LoadAddress("@A", valA);
	}
	// If A is register
	else if (IsReg(valA)) {
		compiledLines.push_back(MoveFromRegToReg(valA, "@A"));
	}
	// If A is variable
	else if (IsVar(valA)) {
		LoadAddress("@A", to_string(GetVariableAddress(valA)));
	}
	// If A is decimal
	else if (IsDec(valA)) {
		RegIdToLDI("@A", to_string(procA));
	}

	// Check if two values are equal
	if (comparer == "==" || comparer == "!=") {
		// Finally compare with a subtract, which will activate the ZERO flag if A and B are equal
		compiledLines.push_back("sub\n");
	}

	// Check if A is greater than B
	if (comparer == ">" || comparer == ">=") {
		// Finally compare with a subtract, which will NOT activate the ZERO flag OR the CARRY flag if A is greater than B
		compiledLines.push_back("sub\n");
	}

	// Check if B is greater than A (A less than B <)
	if (comparer == "<" || comparer == "<=") {
		// Finally compare with a subtract, which WILL activate the CARRY flag if A is less than B
		compiledLines.push_back("sub\n");
	}

}

int GetLineNumber() {
	string outStr = "";
	for (int i = 0; i < compiledLines.size(); i++)
		outStr += trim(compiledLines[i]) + "\n";

	compiledLines = split(outStr, "\n");
	int outInt = 0;
	for (int i = 0; i < compiledLines.size(); i++)
		if (trim(compiledLines[i]) != "" && AccomodateSetInProgramRange(compiledLines[i], outInt) && split(compiledLines[i], " ")[0] != "endif" && compiledLines[i][0] != ',')
			outInt++;

	return outInt;
}

// Compile Armstrong into assembly
string CompileCode(const string& inputcode) {

	vector<string> codelines = PreProcess(inputcode);

	cout << endl;
	// Remove comments from end of lines and trim whitespace
	for (int i = 0; i < codelines.size(); i++) {
		codelines[i] = trim(split(codelines[i], "//")[0]);
		PrintColored(codelines[i] + "\n", brightBlackFGColor, "");
	}

	// Replace 'if' statements with 'gotoif' alternatives,
	// and replace 'endif' with custom label to jump to
	int ifID = 0;
	int openIfs = 0;
	int foundIfs = 0;
	int currentNumber = 0;
	int i = 0;
	for (int i = 0; i < codelines.size(); i++)
	{
		if (codelines[i] == "")
			continue;

		if (trim(split(codelines[i], " ")[0]) == "if") {
			openIfs++;
			foundIfs++;
			if (openIfs == 1) {
				ifID++;
				codelines[i] = "gotoif " + InvertExpression(split(split(codelines[i], " ")[1], ":")[0]) + ",#__IF-ID" + to_string(ifID) + "__";
			}
		}
		if (trim(codelines[i]) == "endif") {
			openIfs--;
			// found matching, get location and remove endif
			if (openIfs == 0)
				codelines[i] = "#__IF-ID" + to_string(ifID) + "__";
		}

		// If there are still more 'if' statements, restart
		if (i == codelines.size() - 1 && foundIfs > 0) {
			if (openIfs > 0) {
				PrintColored("Missing matching 'endif'\n", redFGColor, "");
				exit(0);
			}

			openIfs = 0;
			foundIfs = 0;
			i = 0;
			currentNumber = 0;
		}
	}

	PrintColored("  Done!\n", brightGreenFGColor, "");


	int issues = 0; // Number of problems faced while compiling

	// Begin actual parsing and compilation

	cout << "\nParsing started...\n";
	for (int i = 0; i < codelines.size(); i++)
	{
		string command = trim(split(codelines[i], " ")[0]);


		// "#" label marker ex. #JumpToHere
		if (command[0] == '#')
		{
			int labelLineVal = GetLineNumber();
			labels.push_back(command);
			labelLineValues.push_back(labelLineVal);
			PrintColored("ok.	", greenFGColor, "");
			cout << "label:      ";
			PrintColored("'" + command + "'", brightBlueFGColor, "");
			PrintColored(" line: ", brightBlackFGColor, "");
			PrintColored("'" + to_string(labelLineValues.at(labelLineValues.size() - 1)) + "'\n", brightBlueFGColor, "");

			compiledLines.push_back(",\n, == " + command + " ==");

			// Replace any uses of this label with the labelLineVal
			for (int h = 0; h < compiledLines.size(); h++)
			{
				vector<string> splitBySpace = split(trim(compiledLines[h]), " ");

				// No second argument, skip
				if (splitBySpace.size() <= 1)
					continue;

				// Make sure it is a set instruction, and replace if it contains a label that matches.
				if (splitBySpace[0].size() >= 3) {
					if (splitBySpace[0] == "set") { // If a "set" followed by label placeholder
						if (splitBySpace[2] == command) // Check if matching label
							compiledLines[h] = splitBySpace[0] + " " + splitBySpace[1] + " " + to_string(labelLineVal); // Replace
					}
					if (splitBySpace[0] == "here") { // If a "here" followed by label placeholder
						if (splitBySpace[1] == command) // Check if matching label
							compiledLines[h] = splitBySpace[0] + " " + to_string(labelLineVal); // Replace
					}
				}
			}

			continue;
		}

		// "set" command (set <addr> <value>)
		if (command == "define")
		{
			string addrPre = trim(split(codelines[i], " ")[1]);
			string valuePre = trim(split(codelines[i], " ")[2]);
			PrintColored("ok.	", greenFGColor, "");
			cout << "define:     ";
			PrintColored("'" + addrPre + "'", brightBlueFGColor, "");
			PrintColored(" as ", brightBlackFGColor, "");
			PrintColored("'" + valuePre + "'\n", brightBlueFGColor, "");

			int addr = ParseValue(addrPre);
			int value = ParseValue(valuePre);

			compiledLines.push_back(",\n, " + string("define:  '") + addrPre + "' as '" + valuePre + "'");
			compiledLines.push_back("set " + to_string(addr) + " " + to_string(value));
			continue;
		}

		// "change" command (change <location> = <value or location>)
		else if (command == "change")
		{
			string addrPre = trim(split(split(codelines[i], "change ")[1], " = ")[0]);
			string valuePre = trim(split(split(codelines[i], "change ")[1], " = ")[1]);
			PrintColored("ok.	", greenFGColor, "");
			cout << "change:     ";
			PrintColored("'" + addrPre + "'", brightBlueFGColor, "");
			PrintColored(" to ", brightBlackFGColor, "");
			PrintColored("'" + valuePre + "'\n", brightBlueFGColor, "");

			int addr = ParseValue(addrPre);
			int value = ParseValue(valuePre);

			compiledLines.push_back(",\n, " + string("change:  '") + addrPre + "' to '" + valuePre + "'");
			compiledLines.push_back("");


			//
			// If the value is an INT
			//

			// If changing a value pointed to by a pointer to a new integer value
			if (IsPointer(addrPre) && IsDec(valuePre)) {
				RegIdToLDI("@B", to_string(value));
				StoreIntoPointer(addrPre);
			}
			// If changing memory value at an address and setting to a new integer value
			else if (IsHex(addrPre) && IsDec(valuePre)) {
				RegIdToLDI("@A", to_string(value));
				StoreAddress("@A", addrPre);
			}
			// If changing a register value and setting to a new integer value
			else if (IsReg(addrPre) && IsDec(valuePre)) {
				RegIdToLDI(addrPre, to_string(value));
			}
			// If changing a variable value and setting to a new integer value
			else if (IsVar(addrPre) && IsDec(valuePre)) {
				RegIdToLDI("@A", to_string(value));
				StoreAddress("@A", addrPre);
			}


			//
			// If the value is a MEMORY LOCATION
			//

			// If changing a value pointed to by a pointer to another memory location
			else if (IsPointer(addrPre) && IsHex(valuePre)) {
				LoadAddress("@B", to_string(value));
				StoreIntoPointer(addrPre);
			}
			// If changing memory value at an address and setting to another memory location
			else if (IsHex(addrPre) && IsHex(valuePre)) {
				LoadAddress("@A", to_string(value));
				StoreAddress("@A", to_string(addr));
			}
			// If changing a register value and setting to another memory location
			else if (IsReg(addrPre) && IsHex(valuePre)) {
				LoadAddress(addrPre, to_string(value));
			}
			// If changing a variable value and setting to another memory location
			else if (IsVar(addrPre) && IsHex(valuePre)) {
				LoadAddress("@A", to_string(value));
				StoreAddress("@A", to_string(GetVariableAddress(addrPre)));
			}


			//
			// If the value is a VARIABLE
			//

			// If changing a value pointed to by a pointer to a variable
			else if (IsPointer(addrPre) && IsVar(valuePre)) {
				LoadAddress("@B", to_string(value));
				StoreIntoPointer(addrPre);
			}
			// If changing memory value at an address and setting equal to a variable
			else if (IsHex(addrPre) && IsVar(valuePre)) {
				LoadAddress("@A", to_string(value));
				StoreAddress("@A", to_string(addr));
			}
			// If changing a register value and setting equal to a variable
			else if (IsReg(addrPre) && IsVar(valuePre)) {
				LoadAddress(addrPre, to_string(value));
			}
			// If changing a variable value and setting equal to a variable
			else if (IsVar(addrPre) && IsVar(valuePre)) {
				LoadAddress("@A", to_string(GetVariableAddress(valuePre)));
				StoreAddress("@A", to_string(GetVariableAddress(addrPre)));
			}


			//
			// If the value is a REGISTER
			//

			// If changing a value pointed to by a pointer to a register
			else if (IsPointer(addrPre) && IsReg(valuePre)) {
				compiledLines.push_back(MoveFromRegToReg(valuePre, "@B"));
				StoreIntoPointer(addrPre);
			}
			// If changing memory value at an address and setting equal to a register
			else if (IsHex(addrPre) && IsReg(valuePre)) {
				StoreAddress(valuePre, to_string(addr));
			}
			// If changing a register value and setting equal to a register
			else if (IsReg(addrPre) && IsReg(valuePre)) {
				compiledLines.push_back(MoveFromRegToReg(valuePre, addrPre));
			}
			// If changing a variable value and setting equal to a register
			else if (IsVar(addrPre) && IsReg(valuePre)) {
				StoreAddress(valuePre, to_string(GetVariableAddress(addrPre)));
			}


			//
			// If the value is a POINTER
			//

			// If changing a value pointed to by a pointer to a pointer		(＝ω＝.)
			else if (IsPointer(addrPre) && IsPointer(valuePre)) {
				LoadPointer(valuePre); // Load pointer val to change TO into A
				compiledLines.push_back("swp"); // Move val into B for writing
				StoreIntoPointer(addrPre); // Store B into other pointer
			}
			// If changing memory value at an address and setting equal to a pointer
			else if (IsHex(addrPre) && IsPointer(valuePre)) {
				LoadPointer(valuePre); // Load pointer val to change TO into A
				StoreAddress("@A", to_string(addr));
			}
			// If changing a register value and setting equal to a pointer
			else if (IsReg(addrPre) && IsPointer(valuePre)) {
				LoadPointer(valuePre); // Load pointer val to change TO into A
				compiledLines.push_back(MoveFromRegToReg("@A", addrPre));
			}
			// If changing a variable value and setting equal to a pointer
			else if (IsVar(addrPre) && IsPointer(valuePre)) {
				LoadPointer(valuePre); // Load pointer val to change TO into A
				StoreAddress("@A", to_string(GetVariableAddress(addrPre)));
			}


			continue;
		}

		// arithmetic commands add, sub, div, mult  ex. (add <val>,<val> -> <location>)
		else if (command == "add" || command == "sub" || command == "mult" || command == "div")
		{
			string valAPre = trim(split(split(codelines[i], command + " ")[1], ",")[0]);
			string valBPre = trim(split(split(trim(split(codelines[i], command + " ")[1]), ",")[1], "->")[0]);
			string outLoc = trim(split(split(trim(split(codelines[i], command + " ")[1]), ",")[1], "->")[1]);
			PrintColored("ok.	", greenFGColor, "");
			cout << "arithmetic: ";
			PrintColored("'" + command + "'  ", brightBlueFGColor, "");
			PrintColored("'" + valAPre + "' ", brightBlueFGColor, "");
			PrintColored("with ", brightBlackFGColor, "");
			PrintColored("'" + valBPre + "' ", brightBlueFGColor, "");
			PrintColored("-> ", brightBlackFGColor, "");
			PrintColored("'" + outLoc + "'\n", brightBlueFGColor, "");

			int valAProcessed = ParseValue(valAPre);
			int valBProcessed = ParseValue(valBPre);
			int outLocProcessed = ParseValue(outLoc);

			compiledLines.push_back(",\n, " + command + "'  '" + valAPre + "' with '" + valBPre + "' into '" + outLoc + "'");
			compiledLines.push_back("");



			// If second argument is an address
			if (IsHex(valBPre)) {
				LoadAddress("@B", to_string(valBProcessed));
				compiledLines.at(compiledLines.size() - 1) += "\n";
			}
			// If second argument is a register
			else if (IsReg(valBPre)) {
				compiledLines.at(compiledLines.size() - 1) += MoveFromRegToReg(valBPre, "@B");
			}
			// If second argument is a variable
			else if (IsVar(valBPre)) {
				LoadAddress("@B", valBPre);
				compiledLines.at(compiledLines.size() - 1) += "\n";
			}
			// If second argument is a new decimal value
			else if (IsDec(valBPre)) {
				compiledLines.at(compiledLines.size() - 1) += "ldib " + to_string(valBProcessed) + "\n";
			}


			// If first argument is an address
			if (IsHex(valAPre)) {
				LoadAddress("@A", to_string(valAProcessed));
				compiledLines.at(compiledLines.size() - 1) += "\n";
			}
			// If first argument is a register
			else if (IsReg(valAPre)) {
				compiledLines.at(compiledLines.size() - 1) += MoveFromRegToReg(valAPre, "@A");
			}
			// If first argument is a variable
			else if (IsVar(valAPre)) {
				LoadAddress("@A", valAPre);
				compiledLines.at(compiledLines.size() - 1) += "\n";
			}
			// If first argument is a new decimal value
			else if (IsDec(valAPre)) {
				compiledLines.at(compiledLines.size() - 1) += "ldia " + to_string(valAProcessed) + "\n";
			}


			// Add instruction
			compiledLines.at(compiledLines.size() - 1) += command + "\n";


			// If output argument is an address
			if (IsHex(outLoc)) {
				StoreAddress("@A", to_string(outLocProcessed));
			}
			// If output argument is a register
			else if (IsReg(outLoc)) {
				compiledLines.at(compiledLines.size() - 1) += MoveFromRegToReg("@A", outLoc);
			}
			// If output argument is a variable
			else if (IsVar(outLoc)) {
				StoreAddress("@A", outLoc);
			}


			continue;
		}

		// 'goto' command  ex. (goto <addr>)
		else if (command == "goto")
		{
			string addrPre = trim(split(split(codelines[i], command + " ")[1], ",")[0]);
			PrintColored("ok.	", greenFGColor, "");
			cout << "goto:       ";
			PrintColored("'" + addrPre + "'\n", brightBlueFGColor, "");

			string addrProcessed = to_string(ParseValue(addrPre));

			// If label has not been defined yet, write after jump to go back to later.
			if (addrProcessed == "-1") {
				addrProcessed = addrPre;
			}

			compiledLines.push_back(",\n, " + string("goto:    '") + command + "' '" + addrProcessed + "'");


			compiledLines.push_back("jmp"); // Jump to v
			compiledLines.push_back("here " + addrProcessed);


			continue;
		}

		// 'gotoif' command  ex. (gotoif <valA>==<valB>,<addr>)
		else if (command == "gotoif")
		{
			string valAPre = trim(splitByComparator(split(codelines[i], command + " ")[1])[0]);
			string valBPre = trim(split(splitByComparator(split(codelines[i], command + " ")[1])[1], ",")[0]);
			string addrPre = trim(split(split(codelines[i], command + " ")[1], ",")[1]);
			string comparer = trim(split(split(split(codelines[i], command + " ")[1], valAPre)[1], valBPre)[0]);
			PrintColored("ok.	", greenFGColor, "");
			cout << "gotoif:     ";
			PrintColored("'" + valAPre + " " + comparer + " " + valBPre + "'", brightBlueFGColor, "");
			PrintColored(" -> ", brightBlackFGColor, "");
			PrintColored("'" + addrPre + "'\n", brightBlueFGColor, "");

			compiledLines.push_back(",\n, " + string("gotoif:   '") + valAPre + " " + comparer + " " + valBPre + "' -> '" + addrPre + "'\n");
			CompareValues(valAPre, comparer, valBPre, vars);

			string addrProcessed = to_string(ParseValue(addrPre));

			// If label has not been defined yet, write after jump to go back to later.
			if (addrProcessed == "-1") {
				addrProcessed = addrPre;
			}

			// If using equal to '==' comparer
			if (comparer == "==") {
				compiledLines.push_back("jmpz");
				compiledLines.push_back("here " + addrProcessed);
			}
			// If using not equal to '!=' comparer
			else if (comparer == "!=") {
				compiledLines.push_back("jmpz"); // Jump past jump to endif if false
				compiledLines.push_back("here " + to_string(GetLineNumber() + 3));
				compiledLines.push_back("jmp");
				compiledLines.push_back("here " + addrProcessed);
			}
			// If using greater than '>' comparer
			else if (comparer == ">") {
				compiledLines.push_back("jmpz"); // Jump past jump to endif if false
				compiledLines.push_back("here " + to_string(GetLineNumber() + 3));
				compiledLines.push_back("jmpc");
				compiledLines.push_back("here " + addrProcessed);
			}
			// If using greater equal to '>=' comparer
			else if (comparer == ">=") {
				compiledLines.push_back("jmpz"); // Jump past jump to endif if false
				compiledLines.push_back("here " + addrProcessed);
				compiledLines.push_back("jmpc");
				compiledLines.push_back("here " + addrProcessed);
			}
			// If using less than '<' comparer
			else if (comparer == "<") {
				compiledLines.push_back("jmpz"); // Jump past jump to endif if false
				compiledLines.push_back("here " + to_string(GetLineNumber() + 5));
				compiledLines.push_back("jmpc");
				compiledLines.push_back("here " + to_string(GetLineNumber() + 3));
				compiledLines.push_back("jmp");
				compiledLines.push_back("here " + addrProcessed);
			}
			// If using less equal to '<=' comparer
			else if (comparer == "<=") {
				compiledLines.push_back("jmpz"); // Jump past jump to endif if false
				compiledLines.push_back("here " + addrProcessed);
				compiledLines.push_back("jmpc");
				compiledLines.push_back("here " + to_string(GetLineNumber() + 3));
				compiledLines.push_back("jmp");
				compiledLines.push_back("here " + addrProcessed);
			}


			continue;
		}

		// 'if' command  ex. (if <valA>==<valB>: )
		else if (command == "if")
		{
			string valAPre = trim(splitByComparator(split(codelines[i], command + " ")[1])[0]);
			string valBPre = trim(split(split(splitByComparator(split(codelines[i], command + " ")[1])[1], ",")[0], ":")[0]);
			string comparer = trim(split(split(split(codelines[i], command + " ")[1], valAPre)[1], valBPre)[0]);
			PrintColored("ok.	", greenFGColor, "");
			cout << "if:         ";
			PrintColored("'" + valAPre + " " + comparer + " " + valBPre + "'\n", brightBlueFGColor, "");

			compiledLines.push_back(",\n, " + string("if:   '") + valAPre + " " + comparer + " " + valBPre + "'");
			compiledLines.push_back(codelines[i]);
			continue;
		}

		// 'endif' statement
		else if (command == "endif")
		{
			PrintColored("ok.	", greenFGColor, "");
			cout << "endif:\n";

			compiledLines.push_back(",\n, " + string("endif"));
			compiledLines.push_back(codelines[i]);
			continue;
		}

		// 'asm' inline assembly
		else if (trim(split(codelines[i], "\"")[0]) == "asm")
		{
			PrintColored("ok.	", greenFGColor, "");
			cout << "asm:\n";

			compiledLines.push_back(",\n, " + string("inline assembly"));

			compiledLines.push_back(split(codelines[i], "\"")[1]);
			for (i = i + 1; i < codelines.size(); i++)
			{
				if (std::count(codelines[i].begin(), codelines[i].end(), '\"') >= 1)
					break;
				compiledLines.push_back(split(codelines[i], "\"")[0]);
			}

			continue;
		}

		// Invalid syntax or command
		else {
			PrintColored("?	unknown:    ", redFGColor, "");
			PrintColored("'" + command + "'\n", brightBlueFGColor, "");
			issues++;
		}
	}

	string formattedstr = "";
	for (int l = 0; l < compiledLines.size(); l++)
	{
		formattedstr += trim(compiledLines[l]) + "\n";
	}
	compiledLines = split(formattedstr, "\n");
	cout << compiledLines.size() << endl;


	cout << "Parsing ";
	if (issues == 0) {
		PrintColored("Done!\n\n", greenFGColor, "");

		string outStr = "";
		for (int i = 0; i < compiledLines.size(); i++)
		{
			outStr += trim(compiledLines[i]) + "\n";
		}
		return outStr;
	}
	else {
		PrintColored("Issues found, please review.\n\n", yellowFGColor, "");

		return "";
	}
}
