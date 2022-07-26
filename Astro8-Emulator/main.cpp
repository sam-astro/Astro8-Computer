
#include <vector>
#include <algorithm> 
#include <string> 
#include <chrono>
#include <limits.h>
#include <SDL.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "colorprint.h"

#ifdef _WIN32
#define SYS_PAUSE system("pause")
#else
#define SYS_PAUSE system(                            \
	"echo \"Press any key to continue . . .\";"      \
	"(   trap \"stty $(stty -g;stty -icanon)\" EXIT" \
	"    LC_ALL=C dd bs=1 count=1 >/dev/null 2>&1"   \
	")   </dev/tty")
#endif

#define DEV_MODE false


using namespace std::chrono;
using namespace std;


vector<string> vars;
vector<string> labels;
vector<int> labelLineValues;
vector<string> compiledLines;

int AReg = 0;
int BReg = 0;
int CReg = 0;
int expansionPort = 0;
int InstructionReg = 0;
int flags[3] = { 0, 0, 0 };
int bus = 0;
int outputReg = 0;
int memoryIndex = 0;
int programCounter = 0;

int imgX = 0;
int imgY = 0;
int charPixX = 0;
int charPixY = 0;
int characterRamIndex = 0;
int pixelRamIndex = 0xefff;


// Frame limiter which keeps frames steady while also allowing computation time for CPU
int frameSpeed = 10; // ^ Higher = lower FPS, but faster instruction processing     (~60fps at 10)
					 // v Lower = higher FPS, but slower instruction processing

// autoFPS if true will dynamically change the frame speed above ^ to always be around 60 FPS.
//    Set this to false if you want manual control of frameSpeed
#define autoFPS true


float slowdownAmnt = 1;
int iterations = 0;

vector<int> memoryBytes;
vector<int> charRam;

string action = "";
vector<vector<bool>> microinstructionData;

vector<bool> characterRom;

SDL_Rect r;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The renderer we'll be rendering to
SDL_Renderer* gRenderer = NULL;

//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

// Function List
bool Update(float deltatime);
void DrawPixel(int x, int y, int r, int g, int b);
int InitGraphics(const std::string& windowTitle, int width, int height, int pixelScale);
string charToString(char* a);
unsigned BitRange(unsigned value, unsigned offset, unsigned n);
string DecToHexFilled(int input, int desiredSize);
string BinToHexFilled(const string& input, int desiredSize);
int BinToDec(const string& input);
string DecToBin(int input);
string DecToBinFilled(int input, int desiredSize);
string HexToBin(const string& s, int desiredSize);
int HexToDec(const string& hex);
vector<string> explode(const string& str, const char& ch);
vector<string> parseCode(const string& input);
static inline void ltrim(std::string& s);
static inline void rtrim(std::string& s);
static inline string trim(std::string s);
void GenerateMicrocode();
string SimplifiedHertz(float input);
int BinaryVecRangeToInt(const vector<bool>& vec, int min, int max);
string CompileCode(const string& inputcode);
vector<string> splitByComparator(string str);
int ParseValue(const string& input);
string MoveFromRegToReg(const string& from, const string& destination);
int GetLineNumber();
int ConvertAsciiToSdcii(int asciiCode);

SDL_Texture* texture;
std::vector< unsigned char > pixels(64 * 64 * 4, 0);


string instructions[] = { "NOP", "AIN", "BIN", "CIN", "LDIA", "LDIB", "RDEXP", "WREXP", "STA", "STC", "ADD", "SUB", "MULT", "DIV", "JMP", "JMPZ", "JMPC", "LDAIN", "STAOUT", "LDLGE", "STLGE", "SWP", "SWPC", "HLT", "OUT" };

string microinstructions[] = { "EO", "CE", "ST", "EI", "FL" };
string writeInstructionSpecialAddress[] = { "WA", "WB", "WC", "IW", "DW", "WM", "J", "AW", "WE" };
string readInstructionSpecialAddress[] = { "RA", "RB", "RC", "RM", "IR", "CR", "RE" };
string aluInstructionSpecialAddress[] = { "SU", "MU", "DI" };
string flagtypes[] = { "ZEROFLAG", "CARRYFLAG" };

string instructioncodes[] = {
		"fetch( 0=aw,cr & 1=rm,iw,ce & 2=ei", // Fetch
		"ain( 2=aw,ir & 3=wa,rm & 4=ei", // LoadA
		"bin( 2=aw,ir & 3=wb,rm & 4=ei", // LoadB
		"cin( 2=aw,ir & 3=wc,rm & 4=ei", // LoadC
		"ldia( 2=wa,ir & 3=ei", // Load immediate A <val>
		"ldib( 2=wb,ir & 3=ei", // Load immediate B <val>
		"rdexp( 2=wa,re & 3=ei", // Read from expansion port to register A
		"wrexp( 2=ra,we & 3=ei", // Write from reg A to expansion port
		"sta( 2=aw,ir & 3=ra,wm & 4=ei", // Store A <addr>
		"stc( 2=aw,ir & 3=rc,wm & 4=ei", // Store C <addr>
		"add( 2=wa,eo,fl & 3=ei", // Add
		"sub( 2=wa,eo,su,fl & 3=ei", // Subtract
		"mult( 2=wa,eo,mu,fl & 3=ei", // Multiply
		"div( 2=wa,eo,di,fl & 3=ei", // Divide
		"jmp( 2=ir,j & 3=ei", // Jump <addr>
		"jmpz( 2=ir,j | zeroflag & 3=ei", // Jump if zero <addr>
		"jmpc( 2=ir,j | carryflag & 3=ei", // Jump if carry <addr>
		"ldain( 2=ra,aw & 3=wa,rm & 4=ei", // Use reg A as memory address, then copy value from memory into A
		"staout( 2=ra,aw & 3=rb,wm & 4=ei", // Use reg A as memory address, then copy value from B into memory
		"ldlge( 2=cr,aw & 3=rm,aw & 4=rm,wa,ce & 5=ei", // Use value directly after counter as address, then copy value from memory to reg A and advance counter by 2
		"stlge( 2=cr,aw & 3=rm,aw & 4=ra,wm,ce & 5=ei", // Use value directly after counter as address, then copy value from reg A to memory and advance counter by 2
		"swp( 2=ra,wc & 3=wa,rb & 4=rc,wb & 5=ei", // Swap register A and register B (this will overwrite the contents of register C, using it as a temporary swap area)
		"swpc( 2=ra,wb & 3=wa,rc & 4=rb,wc & 5=ei", // Swap register A and register C (this will overwrite the contents of register B, using it as a temporary swap area)
		"hlt( 2=st & 3=ei", // Stop the computer clock
		"out( 2=ra,dw & 3=ei", // Output to decimal display and LCD screen
};


void apply_pixels(
	std::vector<unsigned char>& pixels,
	SDL_Texture* texture,
	unsigned int screen_width)
{
	SDL_UpdateTexture
	(
		texture,
		NULL,
		pixels.data(),
		screen_width * 4
	);
}

void DisplayTexture(SDL_Renderer* renderer, SDL_Texture* texture)
{
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void clear_buffers(SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderClear(renderer);
}

void set_pixel(
	std::vector<unsigned char>* pixels,
	int x, int y, int screen_width,
	Uint8 r, Uint8 g, Uint8 b, Uint8 a
)
{
	const unsigned int offset = (y * 4 * screen_width) + x * 4;
	(*pixels)[offset + 0] = r;        // b
	(*pixels)[offset + 1] = g;        // g
	(*pixels)[offset + 2] = b;        // r
	(*pixels)[offset + 3] = a;    // a
}

void destroy(SDL_Renderer* renderer, SDL_Window* window)
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int clamp(int x, int min, int max) {
	if (x < min)
		return min;
	if (x > max)
		return max;

	return x;
}


int main(int argc, char** argv)
{
	string code = "";

	// If no path is provided
	if (argc == 1)
	{
		// Gather user inputted code
		cout << ("v Emu. Code input v\n");
		string line;
		while (true) {
			getline(cin, line);
			if (line.empty()) {
				break;
			}
			code += line + "\n";
		}
	}
	// Otherwise it is a path
	else
		code = argv[1];

	if (code.empty()) {
		PrintColored("Error: No filename or '#AS' directive provided on the first line\n", redFGColor, "");
		cout<<"\n\nPress Enter to Exit...";
		cin.ignore();
		exit(1);
	}

	// If the input is a path to a file
	if (split(code, "\n")[0].find('/') != std::string::npos || split(code, "\n")[0].find("\\") != std::string::npos || split(code, "\n").size() < 3) {
		string path = trim(split(code, "\n")[0]);
		path.erase(std::remove(path.begin(), path.end(), '\''), path.end()); // Remove all single quotes
		path.erase(std::remove(path.begin(), path.end(), '\"'), path.end()); // Remove all double quotes
		code = "";

		// Open and read file
		string li;
		ifstream fileStr(path);
		if (fileStr.is_open())
		{
			while (getline(fileStr, li)) {
				code += li + "\n";
			}
			fileStr.close();
		}
		else {
			PrintColored("\nError: could not open file ", redFGColor, "");
			PrintColored("\"" + path + "\"\n", brightBlueFGColor, "");
			cout << "\n\nPress Enter to Exit...";
			cin.ignore();
			exit(1);
		}
	}
	else if (argc != 1) {
		PrintColored("\nError: could not open file ", redFGColor, "");
		PrintColored("\"" + code + "\"\n", brightBlueFGColor, "");
		cout << "\n\nPress Enter to Exit...";
		cin.ignore();
		exit(1);
	}

	// If the code inputted is marked as written in armstrong with #AS
	if (split(code, "\n")[0] == "#AS")
	{
		cout << "Compiling AS..." << endl;
		code = CompileCode(code);
		//code = CompileCode("#AS\nmult @C,0x2f -> 0xfff\nchange $variable = 6\nadd @A,$variable -> 0xfff\nchange $variable = @B\n#jmpHere\nchange @EX = @C\nchange $variable = 123\nchange $variable = 3\ndefine 400 3\n#anotherlabel\nchange $variable = 234\nchange $variable = 3\nif @A==@C:\nchange $variable = 0x2f\ngoto 0x0\nendif\ngoto #jmpHere\ngotoif @A==0x13,0x0\nchange $variable = @C\nchange @C = $variable\n");

		if (code != "") {
			cout << "Output:\n";
			ColorAndPrintAssembly(code);
			cout << "Compiling ";
			PrintColored("Done!\n\n", greenFGColor, "");
		}
		else
			exit(0);

		//exit(0);
	}

	// Generate character rom from existing generated file (generate first using C# assembler)
	cout << "Generating Character ROM...";
	string chline;

	// CWD should be "Astro8-Computer/Astro8-Emulator/linux-build"
	const string charsetFilename = "./char_set_memtape";
	ifstream charset(charsetFilename);

	if (charset.is_open())
	{
		getline(charset, chline);
		chline.erase(chline.find_last_not_of(" \n\r\t") + 1);
		for (int i = 0; i < chline.length(); i++)
		{
			characterRom.push_back(chline[i] == '1');
		}
		charset.close();
	}
	else {
		PrintColored("\nError: could not open file ", redFGColor, "");
		PrintColored("\"" + charsetFilename + "\"\n", brightBlueFGColor, "");
		cout << "\n\nPress Enter to Exit...";
		cin.ignore();
		exit(1);
	}
	PrintColored("  " + to_string(chline.length()) + "px  Done!\n\n", greenFGColor, "");

	// Attempt to parse code, will throw error if not proper assembly
	try
	{
		// Generate memory from code and convert from hex to decimal
		vector<string> mbytes = parseCode(code);
		for (int memindex = 0; memindex < mbytes.size(); memindex++)
			memoryBytes.push_back(HexToDec(mbytes[memindex]));
	}
	catch (const std::exception&)
	{
		PrintColored("\nError: failed to parse code. if you are trying to run Armstrong, make sure the first line of code contains  \"#AS\" ", redFGColor, "");
		cout << "\n\nPress Enter to Exit...";
		cin.ignore();
		exit(1);
	}


	// Generate microcode
	cout << "Generating microcode from instruction set...";
	GenerateMicrocode();
	PrintColored("  Done!\n\n", greenFGColor, "");

	cout << "\nStarting Emulation...\n";

	// Start graphics
	InitGraphics("Astro-8 Emulator", 64, 64, 9);


	int cyclesKeyPressed = 0;
	bool keyPress = false;

	float dt = 0;
	bool running = true;
	while (running)
	{
		auto startTime = std::chrono::high_resolution_clock::now();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
			else if (event.type == SDL_KEYDOWN) {

				// Keyboard support
				expansionPort = ConvertAsciiToSdcii((int)(event.key.keysym.scancode));
				//keyPress = true;
				cout << "  expansionPort: " << expansionPort << endl;
			}
			else if (event.type == SDL_KEYUP) {

				// Keyboard support
				expansionPort = 168; // Keyboard idle state is 168 (max value), since 0 is reserved for space
				//keyPress = false;
				//cyclesKeyPressed = 0;
			}
		}
		//if (cyclesKeyPressed >= 25)
		//	expansionPort = 168; // Keyboard idle state is 168 (max value), since 0 is reserved for space
		//if (keyPress)
		//	cyclesKeyPressed++;

			Update(dt);

		// Calculate frame time
		auto stopTime = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count() / 1000.0f;
	}


	//SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
	//SDL_RenderClear(gRenderer);

	destroy(gRenderer, gWindow);
	SDL_Quit();

	return 0;
}

steady_clock::time_point start;
float renderedFrameTime = 0;
bool Update(float deltatime)
{
	renderedFrameTime += deltatime;

	//cout << programCounter << ")  ";
	for (int step = 0; step < 16; step++)
	{

		//cout << "\n     step: " + to_string(step) << endl;

		// Quickly execute fetch
		if (step == 0)
		{
			// CR
			// AW
			memoryIndex = programCounter;
			//cout << ("CR AW ");
			// RM
			// IW
			InstructionReg = memoryBytes.at(clamp(memoryIndex, 0, 65534));
			// CE
			programCounter += 1;
			//cout << "\n     step: 1" << endl;
			//cout << ("RM IW CE ");
			step = 1;
			continue;
		}

		// Address in microcode ROM
		int microcodeLocation = (BitRange((unsigned)InstructionReg, 11, 5) * 64) + (step * 4) + (flags[0] * 2) + flags[1];
		vector<bool> mcode = microinstructionData.at(clamp(microcodeLocation, 0, 2047));

		//cout << "\n (";
		//for (size_t i = 0; i < mcode.size(); i++)
		//{
		//	cout << mcode[i];
		//}
		//cout<<")" << endl;
		//cout << ("\nmcLoc- " + DecToBinFilled(InstructionReg, 16).substr(0, 4) + DecToBinFilled(step, 4) + to_string(flags[0]) + to_string(flags[1])) << "  ==  " << microcodeLocation << endl;
		//cout << ("mcDat- " + mcode) << endl;


		// Check for any reads and execute if applicable
		int readInstr = BinaryVecRangeToInt(mcode, 9, 11);
		//cout << readInstr << "  " << DecToBinFilled(readInstr, 3) << endl;
		if (readInstr == 1)
		{ // RA
			//cout << ("RA ");
			bus = AReg;
		}
		else if (readInstr == 2)
		{ // RB
			//cout << ("RB ");
			bus = BReg;
		}
		else if (readInstr == 3)
		{ // RC
			//cout << ("RC ");
			bus = CReg;
		}
		else if (readInstr == 4)
		{ // RM
			//cout << ("RM ");
			bus = memoryBytes.at(clamp(memoryIndex, 0, 65534));

			//if (memoryIndex <= 16382 || memoryIndex > 16527)
			//	bus = memoryBytes[memoryIndex];
			//else
			//	bus = charRam.at(clamp(memoryIndex - 16383, 0, 143));
		}
		else if (readInstr == 5)
		{ // IR
			//cout << ("IR ");
			bus = BitRange(InstructionReg, 0, 11);
		}
		else if (readInstr == 6)
		{ // CR
			//cout << ("CR ");
			bus = programCounter;
		}
		else if (readInstr == 7)
		{ // RE
			//cout << ("RE ");
			bus = expansionPort;
		}


		// Standalone microinstruction (ungrouped)
		if (mcode[0] == 1)
		{ // EO
			//cout << ("EO ");

			// Find ALU modifications
			int aluMod = BinaryVecRangeToInt(mcode, 12, 13);

			if (aluMod == 1) // Subtract
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
			else if (aluMod == 2) // Multiply
			{
				flags[0] = 0;
				flags[1] = 0;
				if (AReg * BReg == 0)
					flags[0] = 1;
				bus = AReg * BReg;
				if (bus >= 65535)
				{
					bus = bus - 65535;
					flags[1] = 1;
				}
			}
			else if (aluMod == 3) // Divide
			{
				flags[0] = 0;
				flags[1] = 0;

				// Dont divide by zero
				if (BReg != 0) {
					if (AReg / BReg == 0)
						flags[0] = 1;
					bus = AReg / BReg;
				}
				else {
					flags[0] = 1;
					bus = 0;
				}

				if (bus >= 65535)
				{
					bus = bus - 65535;
					flags[1] = 1;
				}
			}
			else // Add
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


		// Check for any writes and execute if applicable
		int writeInstr = BinaryVecRangeToInt(mcode, 5, 8);
		//cout << "write:" << to_string(writeInstr) << " ";
		if (writeInstr == 1)
		{ // WA
			//cout << ("WA ");
			AReg = bus;
		}
		else if (writeInstr == 2)
		{ // WB
			//cout << ("WB ");
			BReg = bus;
		}
		else if (writeInstr == 3)
		{ // WC
			//cout << ("WC ");
			CReg = bus;
		}
		else if (writeInstr == 4)
		{ // IW
			//cout << ("IW ");
			InstructionReg = bus;
		}
		else if (writeInstr == 6)
		{ // WM
			//cout << ("WM ");
			memoryBytes.at(clamp(memoryIndex, 0, 65534)) = bus;
			//if (memoryIndex <= 16382 || memoryIndex > 16527)
			//	memoryBytes[memoryIndex] = bus;
			//else
			//	charRam.at(clamp(memoryIndex - 16383, 0, 143)) = bus;
		}
		else if (writeInstr == 7)
		{ // J
			//cout << ("J ");
			//cout<<Line(DecToBinFilled(InstructionReg, 16));
			//cout<<Line(DecToBinFilled(InstructionReg, 16).Substring(4, 12));
			programCounter = BitRange(InstructionReg, 0, 11);
		}
		else if (writeInstr == 8)
		{ // AW
			//cout << ("AW ");
			memoryIndex = bus;
		}
		else if (writeInstr == 9)
		{ // WE
			//cout << ("WE ");
			expansionPort = bus;
		}


		// Display current pixel
		if (iterations % frameSpeed == 0)
		{
			//PrintColored(to_string(pixelRamIndex )+ "\n", redFGColor, "");
			int characterRamValue = memoryBytes.at(clamp(characterRamIndex + 16382, 0, 65534));
			bool charPixRomVal = characterRom.at((characterRamValue * 64) + (charPixY * 8) + charPixX);

			int pixelVal = memoryBytes.at(clamp(pixelRamIndex, 0, 65534));
			int r, g, b;
			//r= g=b = 0;
			//b = 128;

			if (charPixRomVal == true && imgX < 60) {
				r = 255;
				g = 255;
				b = 255;
			}
			else {
				r = BitRange(pixelVal, 10, 5) * 8; // Get first 5 bits
				g = BitRange(pixelVal, 5, 5) * 8; // get middle bits
				b = BitRange(pixelVal, 0, 5) * 8; // Gets last 5 bits
			}
			//cout << "rgb: (" << r << ", " << g << ", " << b << ")" << endl;

			set_pixel(&pixels, imgX, imgY, 64, r, g, b, 255);
			//DrawPixel(imgX, imgY, r, g, b);


			imgX++;
			charPixX++;
			if (charPixX >= 6) {
				charPixX = 0;
				//if (imgX < 61)
				characterRamIndex++;
			}
			/*if (characterRamIndex >= 100)
				characterRamIndex = 0;*/

				// If x-coord is max, reset and increment y-coord
			if (imgX >= 64)
			{
				imgY++;
				charPixY++;
				charPixX = 0;
				imgX = 0;

				if (charPixY < 6)
					characterRamIndex -= 10;


				/*apply_pixels(pixels, texture, 64);
				DisplayTexture(gRenderer, texture);*/
			}

			if (charPixY >= 6) {
				charPixY = 0;
			}


			if (imgY >= 64) // The final layer is done, reset counter and render image
			{
				imgY = 0;

				characterRamIndex = 0;
				charPixY = 0;
				charPixX = 0;

				// Apply pixels and render
				//SDL_SetRenderDrawColor(gRenderer, 60, 60, 60, SDL_ALPHA_OPAQUE);
				//SDL_RenderClear(gRenderer);

				apply_pixels(pixels, texture, 64);
				DisplayTexture(gRenderer, texture);

				float fps = 1.0f / renderedFrameTime;
				cout << "\r                                                 " << "\r" << SimplifiedHertz(1.0f / deltatime) + "\tFPS: " + to_string(fps) << "  rval: " + to_string(pixelRamIndex);

				if (autoFPS) {
					if (fps > 65)
						frameSpeed++;
					else if (fps < 55)
						frameSpeed--;
					frameSpeed = clamp(frameSpeed, 1, 9999);
				}

				renderedFrameTime = 0;
			}

			pixelRamIndex++;

			if (pixelRamIndex >= 65535)
				pixelRamIndex = 61439;

		}

		// Standalone microinstructions (ungrouped)
		if (mcode[1] == 1)
		{ // CE
			//cout << ("CE ");
			programCounter += 1;
		}
		if (mcode[2] == 1)
		{ // ST
			//cout << ("ST ");
			cout << ("\n== PAUSED from HLT ==\n\n");
			cout << ("FINAL VALUES |=  o: " + to_string(outputReg) + " A: " + to_string(AReg) + " B: " + to_string(BReg) + " C: " + to_string(CReg) + " bus: " + to_string(bus) + " Ins: " + to_string(InstructionReg) + " img:(" + to_string(imgX) + ", " + to_string(imgY) + ")\n");
			cout << "\n\nPress Enter to Exit...";
			cin.ignore();
			exit(1);
		}
		if (mcode[3] == 1)
		{ // EI
			//cout << ("EI \n\n");
			break;
		}
	}

	iterations += 1;
	if (iterations >= 0xFFFFFFFF)
		iterations = 1;

	return true;
}

void DrawPixel(int x, int y, int r, int g, int b)
{
	SDL_SetRenderDrawColor(gRenderer, r, g, b, 255);
	SDL_RenderDrawPoint(gRenderer, x, y);
}

string SimplifiedHertz(float input) {
	if (input >= 1000000000) // GHz
		return to_string(round(input / 1000000000.0f * 10.0f) / 10.0f) + " GHz";
	if (input >= 1000000) // MHz
		return to_string(round(input / 1000000.0f * 10.0f) / 10.0f) + " MHz";
	if (input >= 1000) // KHz
		return to_string(round(input / 1000.0f * 10.0f) / 10.0f) + " KHz";

	return to_string(round(input * 10.0f) / 10.0f) + " KHz";
}

int InitGraphics(const std::string& windowTitle, int width, int height, int pixelScale)
{
	int WINDOW_WIDTH = width;
	int WINDOW_HEIGHT = height;
	int PIXEL_SCALE = pixelScale;

	// Initialize SDL components
	SDL_Init(SDL_INIT_VIDEO);

	gWindow = SDL_CreateWindow(windowTitle.c_str(), 40, 40, WINDOW_WIDTH * PIXEL_SCALE, WINDOW_HEIGHT * PIXEL_SCALE, SDL_WINDOW_SHOWN | SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
	SDL_RenderSetLogicalSize(gRenderer, WINDOW_WIDTH * PIXEL_SCALE, WINDOW_HEIGHT * PIXEL_SCALE);
	SDL_RenderSetScale(gRenderer, PIXEL_SCALE, PIXEL_SCALE);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);

	texture = SDL_CreateTexture(
		gRenderer,
		SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_STREAMING,
		WINDOW_WIDTH, WINDOW_HEIGHT);

	//Get window surface
	gScreenSurface = SDL_GetWindowSurface(gWindow);

	return 0;
}

string charToString(char* a)
{
	string s(a);
	return s;
}

int ConvertAsciiToSdcii(int asciiCode) {
	int conversionTable[600];  // [ascii] = sdcii
	for (size_t i = 0; i < sizeof(conversionTable) / sizeof(conversionTable[0]); i++)
		conversionTable[i] = -1;

	// Special characters
	conversionTable[44] = 0;	// space -> blank
	conversionTable[58] = 1;	// f1 -> smaller solid square
	conversionTable[59] = 2;	// f2 -> full solid square
	conversionTable[87] = 3;	// num+ -> +
	conversionTable[86] = 4;	// num- -> -
	conversionTable[85] = 5;	// num* -> *
	conversionTable[84] = 6;	// num/ -> /
	conversionTable[60] = 7;	// f3 -> full hollow square
	conversionTable[45] = 8;	// _ -> _
	conversionTable[80] = 9;	// l-arr -> <
	conversionTable[79] = 10;	// r-arr -> >
	conversionTable[82] = 71;	// u-arr -> u-arr
	conversionTable[81] = 72;	// d-arr -> d-arr
	conversionTable[49] = 11;	// | -> vertical line |
	conversionTable[66] = 12;	// f9 -> horizontal line --

	// Letters
	conversionTable[4] = 13;	// a -> a
	conversionTable[5] = 14;	// b -> b
	conversionTable[6] = 15;	// c -> c
	conversionTable[7] = 16;	// d -> d
	conversionTable[8] = 17;	// e -> e
	conversionTable[9] = 18;	// f -> f
	conversionTable[10] = 19;	// g -> g
	conversionTable[11] = 20;	// h -> h
	conversionTable[12] = 21;	// i -> i
	conversionTable[13] = 22;	// j -> j
	conversionTable[14] = 23;	// k -> k
	conversionTable[15] = 24;	// l -> l
	conversionTable[16] = 25;	// m -> m
	conversionTable[17] = 26;	// n -> n
	conversionTable[18] = 27;	// o -> o
	conversionTable[19] = 28;	// p -> p
	conversionTable[20] = 29;	// q -> q
	conversionTable[21] = 30;	// r -> r
	conversionTable[22] = 31;	// s -> s
	conversionTable[23] = 32;	// t -> t
	conversionTable[24] = 33;	// u -> u
	conversionTable[25] = 34;	// v -> v
	conversionTable[26] = 35;	// w -> w
	conversionTable[27] = 36;	// x -> x
	conversionTable[28] = 37;	// y -> y
	conversionTable[29] = 38;	// z -> z

	// Numbers
	conversionTable[39] = 39;	// 0 -> 0
	conversionTable[30] = 40;	// 1 -> 1
	conversionTable[31] = 41;	// 2 -> 2
	conversionTable[32] = 42;	// 3 -> 3
	conversionTable[33] = 43;	// 4 -> 4
	conversionTable[34] = 44;	// 5 -> 5
	conversionTable[35] = 45;	// 6 -> 6
	conversionTable[36] = 46;	// 7 -> 7
	conversionTable[37] = 47;	// 8 -> 8
	conversionTable[38] = 48;	// 9 -> 9



	conversionTable[42] = 70;	// backspace -> backspace

	int actualVal = conversionTable[asciiCode];
	if (actualVal == -1) // -1 Means unspecified value
		actualVal = 168;

	return actualVal;
}


vector<string> splitByComparator(string str) {
	vector<string>result;
	while (str.size()) {
		int charSizes[] = { 2, 2, 2, 2, 1, 1 };
		int indexes[6];
		indexes[0] = str.find(">=");
		indexes[1] = str.find("==");
		indexes[2] = str.find("<=");
		indexes[3] = str.find("!=");
		indexes[4] = str.find("<");
		indexes[5] = str.find(">");
		bool found = false;
		for (int i = 0; i < 6; i++)
		{
			if (indexes[i] != string::npos) {
				result.push_back(str.substr(0, indexes[i]));
				str = str.substr(indexes[i] + charSizes[i]);
				if (str.size() == 0)result.push_back(str);
				found = true;
				break;
			}
		}

		if (found == false) {
			result.push_back(str);
			str = "";
		}
	}
	return result;
}

// Gets range of bits inside of an integer <value> starting at <offset> inclusive for <n> range
unsigned BitRange(unsigned value, unsigned offset, unsigned n)
{
	const unsigned max_n = CHAR_BIT * sizeof(unsigned);
	if (offset >= max_n)
		return 0; /* value is padded with infinite zeros on the left */
	value >>= offset; /* drop offset bits */
	if (n >= max_n)
		return value; /* all  bits requested */
	const unsigned mask = (1u << n) - 1; /* n '1's */
	return value & mask;
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
int BinaryVecRangeToInt(const vector<bool>& vec, int min, int max)
{
	int result = 0;
	int base = 1;

	for (unsigned int i = max; i >= min; --i)
	{
		result += vec[i] * base;
		base *= 2;
	}

	return result;
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
	compiledLines.push_back("set " + to_string(GetLineNumber()) + " " + value);
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
			compiledLines.push_back("ain " + to_string(GetLineNumber() + 1));
			PutSetOnCurrentLine(followingValue);
		}
		else if (in == "@B") {
			compiledLines.push_back("bin " + to_string(GetLineNumber() + 1));
			PutSetOnCurrentLine(followingValue);
		}
		else if (in == "@C") {
			compiledLines.push_back("cin " + to_string(GetLineNumber() + 1));
			PutSetOnCurrentLine(followingValue);
		}
		else if (in == "@EX") {
			compiledLines.push_back("ain " + to_string(GetLineNumber() + 1));
			PutSetOnCurrentLine(followingValue);
			compiledLines.push_back("wrexp");
		}
	}
}

//string RegIdToMRead(string in, string followingValue) {
//	if (in == "@A")
//		return "ain " + followingValue;
//	if (in == "@B")
//		return "bin " + followingValue;
//	if (in == "@C")
//		return "cin " + followingValue;
//	if (in == "@EX")
//		return "ain " + followingValue + "\n" + "wrexp";
//
//	return "";
//}

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

int GetLineNumber() {
	string outStr = "";
	for (int i = 0; i < compiledLines.size(); i++)
	{
		outStr += trim(compiledLines[i]) + "\n";
	}

	compiledLines = split(outStr, "\n");
	int outInt = 0;
	for (int i = 0; i < compiledLines.size(); i++)
	{
		if (trim(compiledLines[i]) != "" && AccomodateSetInProgramRange(compiledLines[i], outInt) && split(compiledLines[i], " ")[0] != "endif" && compiledLines[i][0] != ',')
		{
			outInt++;
			//PrintColored(compiledLines[i] + "  ^ INC ^\n", redFGColor, "");
		}
		//else
		//	PrintColored(compiledLines[i] + "  - STAY -\n", blueFGColor, "");
	}

	return outInt;
}

int ActualLineNumFromNum(int x) {
	string outStr = "";
	for (int i = 0; i < compiledLines.size(); i++)
	{
		outStr += trim(compiledLines[i]) + "\n";
	}

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
	{
		if (id == vars[i])
			return i + 16528;
	}

	// Not found, add to list and return size-1
	vars.push_back(id);
	return 16528 + vars.size() - 1;
}

int FindLabelLine(const string& labelName, const vector<string>& labels, const vector<int>& labelLineValues) {
	for (int i = 0; i < labels.size(); i++)
	{
		if (labelName == labels[i]) {
			return labelLineValues[i];
		}
	}
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

string InvertExpression(const string& expression) {
	string valAPre = trim(splitByComparator(expression)[0]);
	string valBPre = trim(split(splitByComparator(expression)[1], ",")[0]);
	string comparer = trim(split(split(expression, valAPre)[1], valBPre)[0]);
	string newComparer = "";

	if (comparer == "<")
		newComparer = ">=";
	else if (comparer == "<=")
		newComparer = ">";
	else if (comparer == ">")
		newComparer = "<=";
	else if (comparer == ">=")
		newComparer = "<";
	else if (comparer == "==")
		newComparer = "!=";
	else if (comparer == "!=")
		newComparer = "==";

	return valAPre + newComparer + valBPre;
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
		compiledLines.at(compiledLines.size() - 1) += "sub\n";
	}

	// Check if A is greater than B
	if (comparer == ">" || comparer == ">=") {
		// Finally compare with a subtract, which will NOT activate the ZERO flag OR the CARRY flag if A is greater than B
		compiledLines.at(compiledLines.size() - 1) += "sub\n";
	}

	// Check if B is greater than A (A less than B <)
	if (comparer == "<" || comparer == "<=") {
		// Finally compare with a subtract, which WILL activate the CARRY flag if A is less than B
		compiledLines.at(compiledLines.size() - 1) += "sub\n";
	}

}

string CompileCode(const string& inputcode) {

	// Pre-process lines of code

	cout << "Preprocessing started...";
	vector<string> codelines = split(inputcode, "\n");
	codelines.erase(codelines.begin() + 0); // Remove the first line (the one containing the '#AS' indicator)

	// Remove line if it is blank or is just a comment
	auto isEmptyOrBlank = [](const std::string& s) {
		return s.find_first_not_of(" \t/") == std::string::npos;
	};
	auto isComment = [](const std::string& s) {
		if (trim(s).size() >= 2)
			return trim(s)[0] == '/' && trim(s)[1] == '/';
		return true;
	};
	codelines.erase(std::remove_if(codelines.begin(), codelines.end(), isEmptyOrBlank), codelines.end());
	codelines.erase(std::remove_if(codelines.begin(), codelines.end(), isComment), codelines.end());


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

		//cout << currentNumber << endl;
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
			if (openIfs == 0) {
				codelines[i] = "#__IF-ID" + to_string(ifID) + "__";
				//break;
			}
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

				// Make sure it is a jmp instruction, and replace if it contains a label that matches.
				if (splitBySpace[0].size() >= 3)
					if (splitBySpace[0][0] == 'j' && splitBySpace[0][1] == 'm' && splitBySpace[0][2] == 'p')
						if (splitBySpace[1] == command) // Check if matching label
							compiledLines[h] = splitBySpace[0] + " " + to_string(labelLineVal);// Replace
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
			if (IsHex(addrPre) && IsDec(valuePre)) {
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
				//compiledLines.at(compiledLines.size() - 1) += "ain " + to_string(value) + "\n" + "sta " + to_string(addr);
				LoadAddress("@A", to_string(value));
				StoreAddress("@A", to_string(addr));
			}
			// If changing a register value and setting to another memory location
			else if (IsReg(addrPre) && IsHex(valuePre)) {
				LoadAddress(addrPre, to_string(value));
				//compiledLines.at(compiledLines.size() - 1) += RegIdToMRead(addrPre, to_string(value));
			}
			// If changing a variable value and setting to another memory location
			else if (IsVar(addrPre) && IsHex(valuePre)) {
				//compiledLines.at(compiledLines.size() - 1) += "ain " + to_string(value) + "\n" + "sta " + to_string(GetVariableAddress(addrPre, vars));
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
				//compiledLines.at(compiledLines.size() - 1) += "ain " + to_string(GetVariableAddress(valuePre)) + "\n" + "sta " + to_string(addr);
				LoadAddress("@A", to_string(value));
				StoreAddress("@A", to_string(addr));
			}
			// If changing a register value and setting equal to a variable
			else if (IsReg(addrPre) && IsVar(valuePre)) {
				//compiledLines.at(compiledLines.size() - 1) += RegIdToMRead(addrPre, to_string(GetVariableAddress(valuePre, vars)));
				LoadAddress(addrPre, to_string(value));
			}
			// If changing a variable value and setting equal to a variable
			else if (IsVar(addrPre) && IsVar(valuePre)) {
				//compiledLines.at(compiledLines.size() - 1) += "ain " + to_string(GetVariableAddress(valuePre, vars)) + "\n" + "sta " + to_string(GetVariableAddress(addrPre));
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
				//compiledLines.at(compiledLines.size() - 1) += MoveFromRegToReg(valuePre, "@A");
				//compiledLines.at(compiledLines.size() - 1) += "sta " + to_string(addr);
				StoreAddress(valuePre, to_string(addr));
			}
			// If changing a register value and setting equal to a register
			else if (IsReg(addrPre) && IsReg(valuePre)) {
				compiledLines.push_back(MoveFromRegToReg(valuePre, addrPre));
			}
			// If changing a variable value and setting equal to a register
			else if (IsVar(addrPre) && IsReg(valuePre)) {
				//compiledLines.at(compiledLines.size() - 1) += MoveFromRegToReg(valuePre, "@A");
				//compiledLines.at(compiledLines.size() - 1) += "sta " + to_string(GetVariableAddress(addrPre));
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
				//compiledLines.at(compiledLines.size() - 1) += "bin " + to_string(valBProcessed) + "\n";
				LoadAddress("@B", to_string(valBProcessed));
				compiledLines.at(compiledLines.size() - 1) += "\n";
			}
			// If second argument is a register
			else if (IsReg(valBPre)) {
				compiledLines.at(compiledLines.size() - 1) += MoveFromRegToReg(valBPre, "@B");
			}
			// If second argument is a variable
			else if (IsVar(valBPre)) {
				//compiledLines.at(compiledLines.size() - 1) += "bin " + to_string(GetVariableAddress(valBPre)) + "\n";
				LoadAddress("@B", valBPre);
				compiledLines.at(compiledLines.size() - 1) += "\n";
			}
			// If second argument is a new decimal value
			else if (IsDec(valBPre)) {
				compiledLines.at(compiledLines.size() - 1) += "ldib " + to_string(valBProcessed) + "\n";
			}


			// If first argument is an address
			if (IsHex(valAPre)) {
				//compiledLines.at(compiledLines.size() - 1) += "ain " + to_string(valAProcessed) + "\n";
				LoadAddress("@A", to_string(valAProcessed));
				compiledLines.at(compiledLines.size() - 1) += "\n";
			}
			// If first argument is a register
			else if (IsReg(valAPre)) {
				compiledLines.at(compiledLines.size() - 1) += MoveFromRegToReg(valAPre, "@A");
			}
			// If first argument is a variable
			else if (IsVar(valAPre)) {
				//compiledLines.at(compiledLines.size() - 1) += "ain " + to_string(GetVariableAddress(valAPre)) + "\n";
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
				//compiledLines.at(compiledLines.size() - 1) += "sta " + to_string(outLocProcessed) + "\n";
				StoreAddress("@A", to_string(outLocProcessed));
			}
			// If output argument is a register
			else if (IsReg(outLoc)) {
				compiledLines.at(compiledLines.size() - 1) += MoveFromRegToReg("@A", outLoc);
			}
			// If output argument is a variable
			else if (IsVar(outLoc)) {
				//compiledLines.at(compiledLines.size() - 1) += "sta " + to_string(GetVariableAddress(outLoc)) + "\n";
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
			compiledLines.push_back("");


			compiledLines.at(compiledLines.size() - 1) += "jmp " + addrProcessed;


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
				compiledLines.push_back("jmpz " + addrProcessed);
			}
			// If using not equal to '!=' comparer
			else if (comparer == "!=") {
				int lineNum = GetLineNumber();
				compiledLines.push_back("jmpz " + to_string(lineNum + 2) + "\njmp " + addrProcessed); // Jump past jump to endif if false
			}
			// If using greater than '>' comparer
			else if (comparer == ">") {
				int lineNum = GetLineNumber();
				compiledLines.push_back("jmpz " + to_string(lineNum + 2) + "\njmpc " + addrProcessed); // Jump past jump to endif if false
			}
			// If using greater equal to '>=' comparer
			else if (comparer == ">=") {
				int lineNum = GetLineNumber();
				compiledLines.push_back("jmpz " + addrProcessed + "\njmpc " + addrProcessed); // Jump past jump to endif if false
			}
			// If using less than '<' comparer
			else if (comparer == "<") {
				int lineNum = GetLineNumber();
				compiledLines.push_back("jmpz " + to_string(lineNum + 3) + "\njmpc " + to_string(lineNum + 3) + "\njmp " + addrProcessed); // Jump past jump to endif if false
			}
			// If using less equal to '<=' comparer
			else if (comparer == "<=") {
				int lineNum = GetLineNumber();
				compiledLines.push_back("jmpz " + addrProcessed + "\njmpc " + to_string(lineNum + 3) + "\njmp " + addrProcessed); // Jump past jump to endif if false
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

		// 'stop' statement
		else if (command == "stop")
		{
			PrintColored("ok.	", greenFGColor, "");
			cout << "stop:\n";

			compiledLines.push_back(",\n, " + string("stop"));
			compiledLines.push_back("hlt");
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

vector<string> parseCode(const string& input)
{
	vector<string> outputBytes;
	for (int i = 0; i < 65535; i++)
		outputBytes.push_back("0000");

	string icopy = input;
	transform(icopy.begin(), icopy.end(), icopy.begin(), ::toupper);
	vector<string> splitcode = explode(icopy, '\n');

#if DEV_MODE
	cout << endl;
#endif

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
#if DEV_MODE
			cout << ("-\t" + splitcode[i] + "\n");
#endif
			continue;
		}
		if (splitBySpace[0] == "SET")
		{
			int addr = stoi(splitBySpace[1]);
			string hVal = DecToHexFilled(stoi(splitBySpace[2]), 4);
			if (addr <= 16382)
				outputBytes[addr] = hVal;
			else
				charRam[clamp(addr - 16383, 0, 143)] = stoi(splitBySpace[2]);
#if DEV_MODE
			cout << ("-\t" + splitcode[i] + "\t  ~   ~\n");
#endif
			continue;
		}

		// Memory address is already used, skip.
		if (outputBytes[memaddr] != "0000") {
			memaddr += 1;
		}

#if DEV_MODE
		cout << (to_string(memaddr) + " " + splitcode[i] + "   \t  =>  ");
#endif

		// Find index of instruction
		for (int f = 0; f < sizeof(instructions) / sizeof(instructions[0]); f++)
		{
			if (instructions[f] == splitBySpace[0])
			{
#if DEV_MODE
				cout << DecToBinFilled(f, 5);
#endif
				outputBytes[memaddr] = DecToBinFilled(f, 5);
			}
		}

		// Check if any args are after the command
		if (splitcode[i] != splitBySpace[0])
		{
#if DEV_MODE
			cout << DecToBinFilled(stoi(splitBySpace[1]), 11);
#endif
			outputBytes[memaddr] += DecToBinFilled(stoi(splitBySpace[1]), 11);
		}
		else
		{
#if DEV_MODE
			cout << " 00000000000";
#endif
			outputBytes[memaddr] += "00000000000";
		}
#if DEV_MODE
		cout << "  " + BinToHexFilled(outputBytes[memaddr], 4) + "\n";
#endif
		outputBytes[memaddr] = BinToHexFilled(outputBytes[memaddr], 4); // Convert from binary to hex
		memaddr += 1;
	}


	// Print the output
	string processedOutput = "";
	processedOutput += "\nv3.0 hex words addressed\n";
	processedOutput += "000: ";
	for (int outindex = 0; outindex < outputBytes.size(); outindex++)
	{
		if (outindex % 8 == 0 && outindex != 0)
		{
			string locationTmp = DecToHexFilled(outindex, 3);
			transform(locationTmp.begin(), locationTmp.end(), locationTmp.begin(), ::toupper);
			processedOutput += "\n" + DecToHexFilled(outindex, 3) + ": ";
		}
		processedOutput += outputBytes[outindex] + " ";

		string ttmp = outputBytes[outindex];
		transform(ttmp.begin(), ttmp.end(), ttmp.begin(), ::toupper);
	}
#if DEV_MODE
	cout << processedOutput << endl << endl;
#endif

	// Save the data to ./program_machine_code
	fstream myStream;
	myStream.open("./program_machine_code", ios::out);
	myStream << processedOutput;

	return outputBytes;
}

void ComputeStepInstructions(const string& stepContents, char* stepComputedInstruction) {

	for (int mins = 0; mins < sizeof(microinstructions) / sizeof(microinstructions[0]); mins++)
	{
		// Check if microinstruction matches at index
		if (stepContents.find(microinstructions[mins]) != std::string::npos)
		{
			stepComputedInstruction[mins] = '1'; // activate
		}

		// Check if microinstruction requires special code
		for (int minsother = 0; minsother < sizeof(writeInstructionSpecialAddress) / sizeof(writeInstructionSpecialAddress[0]); minsother++)
		{ // Check all write instruction types
			if (stepContents.find(writeInstructionSpecialAddress[minsother]) != std::string::npos)
			{
				string binaryval = DecToBinFilled(minsother + 1, 4);
				stepComputedInstruction[5] = binaryval[0];
				stepComputedInstruction[6] = binaryval[1];
				stepComputedInstruction[7] = binaryval[2];
				stepComputedInstruction[8] = binaryval[3];
			}
		}

		// Check if microinstruction requires special code
		for (int minsother = 0; minsother < sizeof(readInstructionSpecialAddress) / sizeof(readInstructionSpecialAddress[0]); minsother++)
		{ // Check all read instruction types
			if (stepContents.find(readInstructionSpecialAddress[minsother]) != std::string::npos)
			{
				string binaryval = DecToBinFilled(minsother + 1, 3);
				stepComputedInstruction[9] = binaryval[0];
				stepComputedInstruction[10] = binaryval[1];
				stepComputedInstruction[11] = binaryval[2];
			}
		}

		// Check if microinstruction requires special code
		for (int minsother = 0; minsother < sizeof(aluInstructionSpecialAddress) / sizeof(aluInstructionSpecialAddress[0]); minsother++)
		{ // Check all ALU instruction types
			if (stepContents.find(aluInstructionSpecialAddress[minsother]) != std::string::npos)
			{
				string binaryval = DecToBinFilled(minsother + 1, 2);
				stepComputedInstruction[12] = binaryval[0];
				stepComputedInstruction[13] = binaryval[1];
			}
		}

	}
}

void GenerateMicrocode()
{
	// Generate zeros in data
	vector<string> output;
	vector<bool> ii;
	for (int osind = 0; osind < 2048; osind++) { output.push_back("00000"); microinstructionData.push_back(ii); }

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
#if DEV_MODE
		cout << (newStr) << " ." << endl;
#endif
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
#if DEV_MODE
	cout << "\n\ngenerate fetch... \n";
#endif
	for (int ins = 0; ins < 32; ins++) // Iterate through all definitions of instructions
	{
		int correctedIndex = ins;

		string startaddress = DecToBinFilled(correctedIndex, 5);

		vector<string> instSteps = explode(instructioncodes[0], '&');
		for (int step = 0; step < instSteps.size(); step++) // Iterate through every step
		{
			int actualStep = stoi(explode(instSteps[step], '=')[0]);
			string stepContents = explode(explode(instSteps[step], '=')[1], '|')[0];

			string midaddress = DecToBinFilled(actualStep, 4);

			char stepComputedInstruction[14] = { '0','0', '0','0', '0','0', '0','0', '0','0', '0','0', '0','0' };
			ComputeStepInstructions(stepContents, stepComputedInstruction);


			// Compute flags combinations
			for (int flagcombinations = 0; flagcombinations < (sizeof(flagtypes) / sizeof(flagtypes[0])) * (sizeof(flagtypes) / sizeof(flagtypes[0])); flagcombinations++)
			{
				char endaddress[] = { '0', '0' };
				// Look for flags
				if (instSteps[step].find("|") != std::string::npos)
				{
					vector<string> inststepFlags = explode(explode(instSteps[step], '|')[1], ',');
					for (int flag = 0; flag < inststepFlags.size(); flag++) // Iterate through all flags in step
					{
						for (int checkflag = 0; checkflag < (sizeof(flagtypes) / sizeof(flagtypes[0])); checkflag++) // What is the index of the flag
						{
							if (inststepFlags[flag] == flagtypes[checkflag])
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

#if DEV_MODE
				cout << ("\t& " + startaddress + " " + midaddress + " " + charToString(newendaddress) + "  =  " + BinToHexFilled(stepComputedInstruction, 4) + "\n");
#endif
				output[BinToDec(startaddress + midaddress + charToString(newendaddress))] = BinToHexFilled(stepComputedInstruction, 5);
			}
		}

	}

	// Do actual processing
#if DEV_MODE
	cout << "\n\ngenerate general... \n";
#endif
	for (int ins = 1; ins < (sizeof(instructioncodes) / sizeof(instructioncodes[0])); ins++) // Iterate through all definitions of instructions
	{
		int correctedIndex = instIndexes[ins];

#if DEV_MODE
		cout << (instructioncodes[correctedIndex] + "\n");
#endif

		string startaddress = DecToBinFilled(correctedIndex, 5);

		vector<string> instSteps = explode(instructioncodes[correctedIndex], '&');
		for (int step = 0; step < instSteps.size(); step++) // Iterate through every step
		{
			int actualStep = stoi(explode(instSteps[step], '=')[0]);
			string stepContents = explode(explode(instSteps[step], '=')[1], '|')[0];

			string midaddress = DecToBinFilled(actualStep, 4);

			char stepComputedInstruction[14] = { '0','0', '0','0', '0','0', '0','0', '0','0', '0','0', '0','0' };
			ComputeStepInstructions(stepContents, stepComputedInstruction);


			// Compute flags combinations
			for (int flagcombinations = 0; flagcombinations < (sizeof(flagtypes) / sizeof(flagtypes[0])) * (sizeof(flagtypes) / sizeof(flagtypes[0])); flagcombinations++)
			{
				char endaddress[] = { '0', '0' };
				int stepLocked[] = { 0, 0 };
				// If flags are specified in current step layer, set them to what is specified and lock that bit
				if (instSteps[step].find("|") != std::string::npos)
				{
					vector<string> inststepFlags = explode(explode(instSteps[step], '|')[1], ',');
					for (int flag = 0; flag < inststepFlags.size(); flag++) // Iterate through all flags in step
					{
						for (int checkflag = 0; checkflag < (sizeof(flagtypes) / sizeof(flagtypes[0])); checkflag++) // What is the index of the flag
						{
							if (inststepFlags[flag].find(flagtypes[checkflag]) != std::string::npos)
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

#if DEV_MODE
				cout << ("\t& " + startaddress + " " + midaddress + " " + charToString(newendaddress) + "  =  " + BinToHexFilled(stepComputedInstruction, 4));
				cout << endl;
#endif
				output[BinToDec(startaddress + midaddress + charToString(newendaddress))] = BinToHexFilled(stepComputedInstruction, 5);
			}
		}
	}

	// Print the output
	string processedOutput = "";
	processedOutput += "\nv3.0 hex words addressed\n";
	processedOutput += "000: ";
	for (int outindex = 0; outindex < output.size(); outindex++)
	{
		if (outindex % 8 == 0 && outindex != 0)
		{
			string locationTmp = DecToHexFilled(outindex, 3);
			transform(locationTmp.begin(), locationTmp.end(), locationTmp.begin(), ::toupper);
			processedOutput += "\n" + DecToHexFilled(outindex, 3) + ": ";
		}
		processedOutput += output[outindex] + " ";

		string ttmp = output[outindex];
		transform(ttmp.begin(), ttmp.end(), ttmp.begin(), ::toupper);

		string binversion = HexToBin(ttmp, 14);
		for (int i = 0; i < binversion.size(); i++)
		{
			microinstructionData[outindex].push_back(binversion[i] == '1');
		}
	}
	//cout << processedOutput << endl << endl;

	// Save the data to ./microinstructions_cpu_v1
	fstream myStream;
	myStream.open("./microinstructions_cpu", ios::out);
	myStream << processedOutput;
}
