#pragma once


#include <string> 
#include <vector>
#include <stdio.h>
#include <iostream>
#include "colorprint.h"
#include <sstream>

#include "processing.h"

using namespace std;



string DecToHexFilled(int input, int desiredSize);
string BinToHexFilled(const string& input, int desiredSize);
int BinToDec(const string& input);
string DecToBin(int input);
string DecToBinFilled(int input, int desiredSize);
string HexToBin(const string& s, int desiredSize);
int HexToDec(const string& hex);
bool IsHex(const string& in);
bool IsBin(const string& in);
bool IsReg(const string& in);
bool IsVar(const string& in);
bool IsLabel(const string& in);
bool IsPointer(const string& in);
bool IsDec(const string& in);
void PutSetOnCurrentLine(const string& value);
void LoadAddress(const string& reg, const string& address);
void StoreAddress(const string& reg, const string& address);
void RegIdToLDI(const string& in, const string& followingValue);
string MoveFromRegToReg(const string& from, const string& destination);
int ActualLineNumFromNum(int x);
int GetVariableAddress(const string& id);
int FindLabelLine(const string& labelName, const vector<string>& labels, const vector<int>& labelLineValues);
int ParseValue(const string& input);
void LoadPointer(const string& str);
void StoreIntoPointer(const string& str);
void CompareValues(const string& valA, const string& comparer, const string& valB, const vector<string>& vars);
int GetLineNumber();
string CompileCode(const string& inputcode);

