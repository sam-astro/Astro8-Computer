
#include "strops.h"

std::string JoinArrayPieces(std::string input[])
{
	std::string outStr = "";
	for (int i = 0; i < sizeof(input) / sizeof(input[0]); i++) {
		outStr += input[i];
	}
	return outStr;
}

std::string SplitGetLastAfterChar(const std::string& str, std::string delim)
{
	std::vector<std::string> splittedString;
	int startIndex = 0;
	int endIndex = 0;
	while ((endIndex = str.find(delim, startIndex)) < str.size()) {
		//std::string val = str.substr(startIndex, endIndex - startIndex);
		//splittedString.push_back(val);
		startIndex = endIndex + delim.size();
	}
	if (startIndex < str.size()) {
		std::string val = str.substr(startIndex);
		//splittedString.push_back(val);
		return val;
	}
	return "";
}

std::string JoinArrayPieces(std::vector<std::string>& input)
{
	std::string outStr = "";
	for (int i = 0; i < input.size(); i++) {
		outStr += input[i];
	}
	return outStr;
}


std::string CommaLargeNumber(unsigned long long int num)
{
	unsigned long long int v = num;
	std::string s = std::to_string(v);

	unsigned long long int n = s.length() - 3;
	unsigned long long int end = (v >= 0) ? 0 : 1;	// Support for negative numbers
	while (n > end) {
		s.insert(n, ",");
		n -= 3;
	}

	return s;
}

std::string CommaLargeNumber(long num)
{
	long v = num;
	std::string s = std::to_string(v);

	long n = s.length() - 3;
	long end = (v >= 0) ? 0 : 1;  // Support for negative numbers
	while (n > end) {
		s.insert(n, ",");
		n -= 3;
	}

	return s;
}

std::string CommaLargeNumberF(float num)
{
	int v = (int)num;
	auto s = std::to_string(v);

	int n = s.length() - 3;
	int end = (v >= 0) ? 0 : 1;	 // Support for negative numbers
	while (n > end) {
		s.insert(n, ",");
		n -= 3;
	}

	std::string numStr = std::to_string(num);
	s += "." + SplitString(numStr, ".")[1];

	return s;
}

std::string CommaLargeNumberF(double num)
{
	long v = (long)num;
	auto s = std::to_string(v);

	long n = s.length() - 3;
	long end = (v >= 0) ? 0 : 1;  // Support for negative numbers
	while (n > end) {
		s.insert(n, ",");
		n -= 3;
	}

	std::string numStr = std::to_string(num);
	s += "." + SplitString(numStr, ".")[1];

	return s;
}

// Function to pad the front of a string with a character to make it a certain length
std::string PadString(const std::string& input, char padChar, size_t desiredLength)
{
	std::string result = input;
	while (result.length() < desiredLength) {
		result.insert(result.begin(), padChar);
	}
	return result;
}

// Function to pad the back of a string with a character to make it a certain length
std::string PadStringRight(const std::string& input, char padChar, size_t desiredLength)
{
	std::string result = input;
	while (result.length() < desiredLength) {
		result += padChar;
	}
	return result;
}

// Function to extract the padded characters from the front of a string until another character is found
std::string ExtractPaddedChars(const std::string& input, char padChar)
{
	std::string result;
	size_t index = 0;

	// Extract the padded characters until a non-padded character is found
	while (index < input.length() && input[index] == padChar) {
		result += input[index];
		index++;
	}

	return result;
}

// Split a string <str> by a delimiter <delim>, and return a vector of strings
std::vector<std::string> SplitString(std::string& str, std::string delim)
{
	std::vector<std::string> splittedString;
	int startIndex = 0;
	int endIndex = 0;
	while ((endIndex = str.find(delim, startIndex)) < str.size()) {
		std::string val = str.substr(startIndex, endIndex - startIndex);
		splittedString.push_back(val);
		startIndex = endIndex + delim.size();
	}
	if (startIndex < str.size()) {
		std::string val = str.substr(startIndex);
		splittedString.push_back(val);
	}
	return splittedString;
}

// See if a string <str> contains another <check>
bool StringContains(std::string& str, std::string check)
{
	if (str.find(check, 0) < str.size()) {
		return true;
	}
	return false;
}
// See if a string <str> contains another <check>
bool StringContains(std::string& str, char check)
{
	if (str.find(check, 0) < str.size()) {
		return true;
	}
	return false;
}

// trim from start (in place)
void ltrim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
}

// trim from end (in place)
void rtrim(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(),
		s.end());
}
// Overrided by boost::trim
// trim from both ends (in place)
std::string TrimString(std::string s)
{
	std::string sN = s;
	ltrim(sN);
	rtrim(sN);
	return sN;
}

// Convert string to uppercase
std::string ToUpper(std::string s)
{
	std::string sN = s;
	for (auto& c : sN)
		c = toupper(c);
	return sN;
}

// Convert string to lowercase
std::string ToLower(std::string s)
{
	std::string sN = s;
	for (auto& c : sN)
		c = tolower(c);
	return sN;
}

bool StringStartsWith(std::string str, std::string substr)
{
	for (int i = 0; i < substr.length(); i++) {
		if (str[i] != substr[i])
			return false;
	}
	return true;
}

char toHexChar(int value)
{
	if (value < 10) {
		return static_cast<char>('0' + value);
	}
	else {
		return static_cast<char>('a' + value - 10);
	}
}

void stringToHex(char* input, char* output, int len)
{
	for (int i = 0; i < len; i++) {
		output[0] = toHexChar((static_cast<unsigned char>(*input) >> 4) & 0x0F);
		output[1] = toHexChar(static_cast<unsigned char>(*input) & 0x0F);
		input++;
		output += 2;
	}
	*output = '\0';	 // Add null terminator at the end
}

char d[30];	 // Buffer for the CharStrStartsWith() function check
// Check if the unsigned char* <str> starts with another char* <substr>
bool CharStrStartsWith(unsigned char* str, char* substr, int len)
{
	char* c = d;
	//for (int i = 0; i < len; i++)
	//{
	//	sprintf(c + (i * 2), "%02x", str[i]);
	//}
	stringToHex((char*)str, c, len);
	for (int i = 0; i < len; i++) {
		if (c[i] != substr[i])
			return false;
	}
	return true;
}

// Function to compare two char* representing numbers. Returns true if a > b
bool CompareCharNumbers(const unsigned char* number1, const unsigned char* number2)
{
	//// Skip leading zeros
	//while (*number1 == '0' && *(number1 + 1) != '\0') {
	//	number1++;
	//}
	//while (*number2 == '0' && *(number2 + 1) != '\0') {
	//	number2++;
	//}

	uint8_t it = 0;
	// Compare the remaining digits
	while (it < 60) {
		if (number1[it] < number2[it]) {
			return false;
		}
		else if (number1[it] > number2[it]) {
			return true;
		}
		else
			//number1++;
			//number2++;
			it++;
	}

	// If one number has more digits, the shorter one is considered smaller
	return true;
}

// Replace all instances of the escape symbol '\n' with the string "\\n"
std::string ReplaceEscapeSymbols(std::string s)
{
	std::string out = "";
	for (size_t i = 0; i < s.size(); i++) {
		if (s[i] == '\n')
			out += "\\n";
		else
			out += s[i];
	}
	return out;
}

// Function to multiply a large hexadecimal number by an integer
std::string multiplyHexByInteger(const std::string& hexNumber, int multiplier)
{
	// Convert the multiplier to hexadecimal string
	std::string multiplierHex = std::to_string(multiplier);
	std::string resultHex;

	// Perform multiplication digit by digit
	int carry = 0;
	for (int i = hexNumber.length() - 1; i >= 0; --i) {
		char hexDigit = hexNumber[i];

		// Convert the hexadecimal digit to its decimal value
		int digitValue;
		if (hexDigit >= '0' && hexDigit <= '9') {
			digitValue = hexDigit - '0';
		}
		else if (hexDigit >= 'A' && hexDigit <= 'F') {
			digitValue = hexDigit - 'A' + 10;
		}
		else if (hexDigit >= 'a' && hexDigit <= 'f') {
			digitValue = hexDigit - 'a' + 10;
		}
		else {
			// Invalid character in the hexadecimal string
			return "";
		}

		// Perform the multiplication and add the carry
		int product = digitValue * multiplier + carry;
		carry = product / 16;

		// Convert the product back to hexadecimal digit
		int remainder = product % 16;
		char hexResult;
		if (remainder < 10) {
			hexResult = '0' + remainder;
		}
		else {
			hexResult = 'A' + remainder - 10;
		}

		// Add the hexadecimal digit to the result string
		resultHex = hexResult + resultHex;
	}

	// Add the carry if any
	if (carry > 0) {
		char hexCarry = '0' + carry;
		resultHex = hexCarry + resultHex;
	}

	return resultHex;
}

void subOneFromHex(std::string& hexNumber, int index)
{
	for (int i = index; i >= 0; --i) {
		char hexDigit = hexNumber[i];
		//hexNumber[i] = (char)hexNumber[i] - 1;
		if (hexDigit > '0' && hexDigit <= '9') {
			hexNumber[i] = (char)hexNumber[i] - 1;
		}
		else if (hexDigit > 'A' && hexDigit <= 'F') {
			hexNumber[i] = (char)hexNumber[i] - 1;
		}
		else if (hexDigit == 'A') {
			hexNumber[i] = '9';
		}
		else if (hexDigit == '0') {
			hexNumber[i] = 'F';
			continue;
		}
		break;
	}
}


void addOneToHexStr(char* num, int len)
{
	for (int i = len - 1; i >= 0; i--) {
		num[i] = num[i] + 1;
		if ((num[i] >= '0' && num[i] <= '9') || (num[i] >= 'a' && num[i] <= 'z')) {
			break;
		}
		else if (num[i] > '9' && num[i] < 'a') {
			num[i] = 'a';
			break;
		}
		else if (num[i] < '0') {
			num[i] = '0';
			break;
		}
		else if (num[i] > 'z') {
			num[i] = '0';
		}
	}
}


void addOneToHexStrInRange(char* num, int start, int len)
{
	for (int i = start + len - 1; i >= start; i--) {
		num[i] = num[i] + 1;
		if ((num[i] >= '0' && num[i] <= '9') || (num[i] >= 'a' && num[i] <= 'z')) {
			break;
		}
		else if (num[i] > '9' && num[i] < 'a') {
			num[i] = 'a';
			break;
		}
		else if (num[i] < '0') {
			num[i] = '0';
			break;
		}
		else if (num[i] > 'z') {
			num[i] = '0';
		}
	}
}

// A function to perform division of large numbers
std::string longDivision(const std::string& number, uint32_t divisor)
{
	// As result can be very large store it in string also
	std::string ans;

	// Find prefix of number that is larger
	// than divisor.
	int idx = 0;
	int temp = number[0] - '0';
	while (idx < (number.size() - 1) && temp < divisor)
		temp = temp * 10 + (number[++idx] - '0');

	// Repeatedly divide divisor with temp. After
	// every division, update temp to include one
	// more digit.
	while ((number.size() - 1) >= idx) {
		// Store result in answer i.e. temp / divisor
		ans += (temp / divisor) + '0';

		// Take next digit of number
		temp = (temp % divisor) * 10 + number[++idx] - '0';
	}

	// If divisor is greater than number
	if (ans.length() == 0)
		return "0";

	// else return ans
	return ans;
}


// Function to divide a large hexadecimal number by a float
std::string divideHexByFloat(const std::string& hexNumber, float divisor)
{
	std::string quotientHex;
	std::string hexNum = hexNumber;
	//int dividend = 0;
	int carry = 0;
	bool nonZeroFound = false;

	// Iterate over each digit in the hexadecimal number
	int i = 0;
	for (char hexDigit : hexNumber) {

		int dividendDigit;
		if (hexDigit >= '0' && hexDigit <= '9') {
			dividendDigit = hexDigit - '0';
		}
		else if (hexDigit >= 'A' && hexDigit <= 'F') {
			dividendDigit = hexDigit - 'A' + 10;
		}
		else if (hexDigit >= 'a' && hexDigit <= 'f') {
			dividendDigit = hexDigit - 'a' + 10;
		}
		else {
			// Invalid character in the hexadecimal string
			return "";
		}

		//dividend = dividend * 16 + dividendDigit;

		int dividend = (carry) + dividendDigit;
		if (std::round(((float)dividend / divisor) - (std::round(((float)dividend / divisor)))) != 0) {	 // if dividend has fraction (ie, has remainder), carry
			carry = (dividend % (int)std::round(divisor)) << 4;
			//if (carry == 0)
			//	carry = 1;
			//subOneFromHex(hexNum, i);
			dividend--;
		}
		else
			carry = 0;
		int quotient = std::round((float)dividend / divisor);
		//carry = (dividendDigit % (int)std::round(divisor))<<4;
		//std::cout << dividend << " / " << divisor << " = " << quotient << std::endl;

		//quotientHex.push_back(quotient);
		char hexQuotient;
		if (quotient < 10) {
			hexQuotient = '0' + quotient;
		}
		else {
			hexQuotient = 'A' + quotient - 10;
		}

		//if (!quotientHex.empty() || quotient != 0) {
		quotientHex += hexQuotient;
		//}
		//if (dividend >= divisor) {
		//	int quotient = dividend / divisor;
		//	char hexQuotient;
		//	if (quotient < 10) {
		//		hexQuotient = '0' + quotient;
		//	}
		//	else {
		//		hexQuotient = 'A' + quotient - 10;
		//	}
		//	quotientHex += hexQuotient;
		//	nonZeroFound = true;
		//	dividend = dividend % static_cast<int>(divisor);
		//}
		//else if (nonZeroFound) {
		//	quotientHex += '0';
		//}
		i++;
	}

	//if (quotientHex.empty())
	//	return "0";
	//else
	return quotientHex;
}

std::string shiftHexNumber(const std::string& hexNumber, int digits)
{
	std::string outNum = hexNumber;
	if (digits < 0) {
		outNum = outNum.substr(abs(digits), outNum.length() + digits);
		for (digits < 0; digits++;) {
			outNum += '0';
		}
	}
	else if (digits > 0) {
		outNum = outNum.substr(0, outNum.length() - digits);
	}
	return outNum;
}


float clampf(float x, float min, float max)
{
	if (x < min)
		return min;
	else if (x > max)
		return max;
	else [[likely]]
		return x;
}

// Format the hashes per second float <input> into a readable and shortened string
std::string FormatHPS(float input)
{
	if (input > 1000000000.0f)
		return std::to_string(round(input / 1000000000.0f, 3)) + " gH/s";
	else if (input > 1000000.0f)
		return std::to_string(round(input / 1000000.0f, 3)) + " mH/s";
	else if (input > 1000.0f)
		return std::to_string(round(input / 1000.0f, 3)) + " kH/s";
	else
		return std::to_string(round(input, 3)) + " H/s";
}

// Round a float to <decimal_places> number of decimal places
double round(float value, int decimal_places)
{
	const double multiplier = std::pow(10.0, decimal_places);
	return std::round(value * multiplier) / multiplier;
}

void csubstr(char* arr, char* outArr, int begin, int len, int totalLen, int& actualSize)
{
	if (begin + len >= totalLen)
		len = totalLen - begin;
	actualSize = len;
	for (int i = 0; i < len; i++)
		outArr[i] = *(arr + begin + i);
	outArr[len] = 0;
}

void cConcatInt(char* arr, char* outArr, int len, int x)
{
	std::string sVal = std::to_string(x);
	strcpy(outArr, arr);
	for (int i = 0; i < sVal.size(); i++)
		outArr[i + len - 1] = sVal[i];
	outArr[len + sVal.size() - 1] = 0;
}

// Returns true if <a> is greater than or equal to <b>
bool IsVersionGreaterOrEqual(std::string a, std::string b)
{
	if (a == b)
		return true;

	a = SplitString(a, "v")[1];
	b = SplitString(b, "v")[1];

	std::string stageA = SplitString(a, "-")[1];
	std::string stageB = SplitString(a, "-")[1];

	if (stageA == "alpha" && stageB != "alpha")
		return false;
	if (stageA == "beta" && stageB != "alpha" && stageB != "beta")
		return false;

	int majorA = stoi(SplitString(a, ".")[0]);
	int majorB = stoi(SplitString(b, ".")[0]);

	if (majorA < majorB)
		return false;

	int minorA = stoi(SplitString(a, ".")[1]);
	int minorB = stoi(SplitString(b, ".")[1]);

	if (minorA < minorB)
		return false;

	int patchA = stoi(SplitString(a, ".")[2]);
	int patchB = stoi(SplitString(b, ".")[2]);

	if (patchA < patchB)
		return false;

	return true;
}

inline static const std::string roundFloat(const double input, const int decimal_places)
{
	std::ostringstream str;
	str << std::fixed << std::setprecision(decimal_places);
	str << input;
	return str.str();
}

std::string truncateMetricNum(double x)
{
	if (x > 1000000000000)
		return roundFloat((float)((long long)x / 10000000000) / 100.0, 2) + " T";
	else if (x > 1000000000)
		return roundFloat((float)((long)x / 10000000) / 100.0, 2) + " G";
	else if (x > 1000000)
		return roundFloat((float)((long)x / 10000) / 100.0, 2) + " M";
	else if (x > 1000)
		return roundFloat((float)((long)x / 10) / 100.0, 2) + " K";
	else
		return std::to_string(x);
}
