
#include <vector>
#include <algorithm> 
#include <string> 
#include <chrono>
#include <limits.h>
#include <SDL.h>
#include <sstream>
#include <fstream>
#include <codecvt>

#include "armstrong-compiler.h"


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


#if UNIX
#include <unistd.h>
#include <filesystem>
#elif WINDOWS
#include <windows.h>
#endif


using namespace std;


int AReg = 0;
int BReg = 0;
int CReg = 0;
int expansionPort = 0;
int InstructionReg = 0;
int flags[3] = { 0, 0, 0 };
int bus = 0;
int outputReg = 0;
uint16_t memoryIndex = 0;
uint64_t programCounter = 0;

int imgX = 0;
int imgY = 0;
int charPixX = 0;
int charPixY = 0;
int characterRamIndex = 0;
int pixelRamIndex = 0xefff;


// 10000000 = 10.0MHz
#define TARGET_CPU_FREQ 10000000
#define TARGET_RENDER_FPS 60.0

vector<int> memoryBytes;
vector<int> charRam;


std::string projectDirectory;


// Refer to https://sam-astro.github.io/Astro8-Computer/docs/Architecture/Micro%20Instructions.html

#define MICROINSTR_SIZE 16
using MicroInstruction = uint16_t;
static_assert(sizeof(MicroInstruction) * 8 >= MICROINSTR_SIZE,
	"Size of MicroInstruction is too small, increase its width...");

MicroInstruction microinstructionData[2048];

enum ALUInstruction : MicroInstruction {
	ALU_SU =   0b0000000000000001,
	ALU_MU =   0b0000000000000010,
	ALU_DI =   0b0000000000000011,
	ALU_SL =   0b0000000000000100,
	ALU_SR =   0b0000000000000101,
	ALU_AND =  0b0000000000000110,
	ALU_OR =   0b0000000000000111,
	ALU_NOT =  0b0000000000001000,
	ALU_MASK = 0b0000000000001111,
};

enum ReadInstruction : MicroInstruction {
	READ_RA = 0b0000000000010000,
	READ_RB = 0b0000000000100000,
	READ_RC = 0b0000000000110000,
	READ_RM = 0b0000000001000000,
	READ_IR = 0b0000000001010000,
	READ_CR = 0b0000000001100000,
	READ_RE = 0b0000000001110000,
	READ_MASK = 0b0000000001110000,
};

enum WriteInstruction : MicroInstruction {
	WRITE_WA = 0b0000000010000000,
	WRITE_WB = 0b0000000100000000,
	WRITE_WC = 0b0000000110000000,
	WRITE_IW = 0b0000001000000000,
	WRITE_DW = 0b0000001010000000,
	WRITE_WM = 0b0000001100000000,
	WRITE_J = 0b0000001110000000,
	WRITE_AW = 0b0000010000000000,
	WRITE_WE = 0b0000010010000000,
	WRITE_MASK = 0b0000011110000000,
};

enum StandaloneInstruction : MicroInstruction {
	STANDALONE_FL = 0b0000100000000000,
	STANDALONE_EI = 0b0001000000000000,
	STANDALONE_ST = 0b0010000000000000,
	STANDALONE_CE = 0b0100000000000000,
	STANDALONE_EO = 0b1000000000000000,
};


vector<bool> characterRom;

SDL_Rect r;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The renderer we'll be rendering to
SDL_Renderer* gRenderer = NULL;

//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

// Function List
void Update();
void Draw();
void DrawPixel(int x, int y, int r, int g, int b);
int InitGraphics(const std::string& windowTitle, int width, int height, int pixelScale);
unsigned BitRange(unsigned value, unsigned offset, unsigned n);
vector<std::string> parseCode(const std::string& input);
void GenerateMicrocode();
std::string SimplifiedHertz(float input);
int ConvertAsciiToSdcii(int asciiCode);

SDL_Texture* texture;
std::vector< unsigned char > pixels(64 * 64 * 4, 0);


std::string instructions[] = { "NOP", "AIN", "BIN", "CIN", "LDIA", "LDIB", "RDEXP", "WREXP", "STA", "STC", "ADD", "SUB", "MULT", "DIV", "JMP", "JMPZ","JMPC", "JREG", "LDAIN", "STAOUT", "LDLGE", "STLGE", "LDW", "SWP", "SWPC", "PCR", "BSL", "BSR", "AND", "OR", "NOT" };

std::string microinstructions[] = { "EO", "CE", "ST", "EI", "FL" };
std::string writeInstructionSpecialAddress[] = { "WA", "WB", "WC", "IW", "DW", "WM", "J", "AW", "WE" };
std::string readInstructionSpecialAddress[] = { "RA", "RB", "RC", "RM", "IR", "CR", "RE" };
std::string aluInstructionSpecialAddress[] = { "SU", "MU", "DI", "SL", "SR", "AND","OR","NOT"};
std::string flagtypes[] = { "ZEROFLAG", "CARRYFLAG" };

std::string instructioncodes[] = {
		"fetch( 0=cr,aw & 1=rm,iw,ce & 2=ei", // Fetch
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
		"jmp( 2=cr,aw & 3=rm,j & 4=ei", // Jump to address following instruction
		"jmpz( 2=cr,aw & 3=ce,rm & 4=j | zeroflag & 5=ei", // Jump if zero to address following instruction
		"jmpc( 2=cr,aw & 3=ce,rm & 4=j | carryflag & 5=ei", // Jump if carry to address following instruction
		"jreg( 2=ra,j & 3=ei", // Jump to the address stored in Reg A
		"ldain( 2=ra,aw & 3=wa,rm & 4=ei", // Use reg A as memory address, then copy value from memory into A
		"staout( 2=ra,aw & 3=rb,wm & 4=ei", // Use reg A as memory address, then copy value from B into memory
		"ldlge( 2=cr,aw & 3=ce,rm,aw & 4=rm,wa & 5=ei", // Use value directly after counter as address, then copy value from memory to reg A and advance counter by 2
		"stlge( 2=cr,aw & 3=ce,rm,aw & 4=ra,wm & 5=ei", // Use value directly after counter as address, then copy value from reg A to memory and advance counter by 2
		"ldw( 2=cr,aw & 3=ce,rm,wa & 4=ei", // Load value directly after counter into A, and advance counter by 2
		"swp( 2=ra,wc & 3=wa,rb & 4=rc,wb & 5=ei", // Swap register A and register B (this will overwrite the contents of register C, using it as a temporary swap area)
		"swpc( 2=ra,wb & 3=wa,rc & 4=rb,wc & 5=ei", // Swap register A and register C (this will overwrite the contents of register B, using it as a temporary swap area)
		"pcr( 2=cr,wa & 3=ei", // Program counter read, get the current program counter value and put it into register A
		"bsl( 2=sl,wa,eo,fl & 3=ei", // Bit shift left A register, the number of bits to shift determined by the value in register B
		"bsr( 2=sr,wa,eo,fl & 3=ei", // Bit shift left A register, the number of bits to shift determined by the value in register B
		"and( 2=and,wa,eo,fl & 3=ei", // Logical AND operation on register A and register B, with result put back into register A
		"or( 2=or,wa,eo,fl & 3=ei", // Logical OR operation on register A and register B, with result put back into register A
		"not( 2=not,wa,eo,fl & 3=ei", // Logical NOT operation on register A, with result put back into register A
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
	(*pixels)[offset + 0] = r;
	(*pixels)[offset + 1] = g;
	(*pixels)[offset + 2] = b;
	(*pixels)[offset + 3] = a;
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


// Convert vector of strings into single std::string separated by \n character
std::string VecToString(const vector<std::string>& vec) {
	std::string newStr;

	for (int i = 0; i < (int)vec.size(); i++)
	{
		newStr += vec[i];
		if (i != (int)vec.size() - 1)
			newStr += "\n";
	}

	return newStr;
}


int main(int argc, char** argv)
{
	std::string code = "";

	// If no path is provided
	if (argc == 1)
	{
		// Gather user inputted code
		cout << ("v Emu. Code input v\n");
		std::string line;
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
		cout << "\n\nPress Enter to Exit...";
		cin.ignore();
		exit(1);
	}

	// If the input is a path to a file
	if (split(code, "\n")[0].find('/') != std::string::npos || split(code, "\n")[0].find("\\") != std::string::npos || split(code, "\n").size() < 3) {
		std::string path = trim(split(code, "\n")[0]);
		path.erase(std::remove(path.begin(), path.end(), '\''), path.end()); // Remove all single quotes
		path.erase(std::remove(path.begin(), path.end(), '\"'), path.end()); // Remove all double quotes
		code = "";

		// Open and read file
		std::string li;
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

		projectDirectory = path.substr(0, path.find_last_of("/\\"));
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
		vector<std::string> codelines = PreProcess(code);

		for (int i = 0; i < codelines.size(); i++)
		{
			// If including another file:    #include "./path/to/file.arm"
			if (split(codelines[i], " ")[0] == "#include") {
				std::string clCpy = codelines[i];

				codelines.erase(codelines.begin() + i); // Remove the #include

				std::string path = trim(split(clCpy, " ")[1]);
				path.erase(std::remove(path.begin(), path.end(), '\''), path.end()); // Remove all single quotes
				path.erase(std::remove(path.begin(), path.end(), '\"'), path.end()); // Remove all double quotes

				// If the path is relative, append the known project path to make it absolute.
				if (path[0] == '.')
					path = projectDirectory + path;

				// Open and read file, appending code onto it after
				std::string codeTmp = "";
				std::string li;
				ifstream fileStr(path);
				if (fileStr.is_open())
				{
					//int randNum = rand() % 9999;
					//codeTmp += "\ngoto #" + path + to_string(randNum);
					while (getline(fileStr, li)) {
						if (li != "#AS") // We don't need another Armstrong label, so we can remove it
							codeTmp += li + "\n";
					}
					//codeTmp += "#" + path + to_string(randNum);
					fileStr.close();
				}
				else {
					PrintColored("\nError: could not open file ", redFGColor, "");
					PrintColored("\"" + path + "\"\n", brightBlueFGColor, "");
					PrintColored("as used with include here:  ", redFGColor, "");
					PrintColored(clCpy + "\n", yellowFGColor, "");
					cout << "\n\nPress Enter to Exit...";
					cin.ignore();
					exit(1);
				}

				code = codeTmp + "\n\n" + VecToString(codelines);

				// Reprocess and go back to beginning
				codelines = PreProcess(code);
				i = 0;
			}
		}




		// Compile
		cout << "Compiling AS..." << endl;
		code = CompileCode(code);

		if (code != "") {
			cout << "Output:\n";
			ColorAndPrintAssembly(code);
			cout << "Compiling ";
			PrintColored("Done!\n\n", greenFGColor, "");
		}
		else
			exit(0);
	}
	else
		ColorAndPrintAssembly(code);

	// Generate character rom from existing generated file (generate first using C# assembler)
	cout << "Generating Character ROM...";
	std::string chline;

	// CWD should be "Astro8-Computer/Astro8-Emulator/linux-build"
	const std::string charsetFilename = "./char_set_memtape";
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
		vector<std::string> mbytes = parseCode(code);
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


	bool keyPress = false;
	bool running = true;
	SDL_Event event;

	int updateCount = 0;
	int frameCount = 0;
	auto lastSecond = std::chrono::high_resolution_clock::now();
	auto lastFrame = lastSecond;
	auto lastTick = lastSecond;

	while (running)
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		double secondDiff = std::chrono::duration<double, std::chrono::milliseconds::period>(startTime - lastSecond).count();
		double frameDiff = std::chrono::duration<double, std::chrono::milliseconds::period>(startTime - lastFrame).count();
		double tickDiff = std::chrono::duration<double, std::chrono::milliseconds::period>(startTime - lastTick).count();

		// Every second
		if (secondDiff > 1000.0) {
			lastSecond = startTime;
			cout << "\r                                                       \r"
				<< SimplifiedHertz(updateCount)
				<< "\tFPS: " << to_string(frameCount)
				<< "  programCounter: " << to_string(programCounter)
				<< std::flush;
			updateCount = 0;
			frameCount = 0;
		}

		// CPU tick
		// Update 1000 times, otherwise chrono becomes a bottleneck
		// Chrono is slow in any case, so it may be replaced later
		// SDL_GetTick is probably not precise enough
		const int numUpdates = 1000;
		if (tickDiff > (numUpdates * 1000.0 / TARGET_CPU_FREQ)) {
			lastTick = startTime;
			for (int i = 0; i < numUpdates; ++i)
				Update();
			updateCount += numUpdates;
		}

		// Frame
		if (frameDiff > (1000.0 / TARGET_RENDER_FPS)) {
			lastFrame = startTime;
			Draw();
			++frameCount;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
				{
					running = false;
				}
				else if (event.type == SDL_KEYDOWN) {

					// Keyboard support
					expansionPort = ConvertAsciiToSdcii((int)(event.key.keysym.scancode));

					cout << "  expansionPort: " << (int)(event.key.keysym.scancode) << endl;
				}
				else if (event.type == SDL_KEYUP) {

					// Keyboard support
					expansionPort = 168; // Keyboard idle state is 168 (max value), since 0 is reserved for space
				}
			}
		}
	}

	destroy(gRenderer, gWindow);
	SDL_Quit();

	return 0;
}


void Update()
{

	for (int step = 0; step < 16; step++)
	{

		// Execute fetch in single step
		if (step == 0)
		{
			// CR
			// AW
			// RM
			// IW
			InstructionReg = memoryBytes[programCounter];
			// CE
			programCounter += 1;
			step = 2;
		}

		// Address in microcode ROM
		int microcodeLocation = ((InstructionReg >> 5) & 0b11111000000) + (step * 4) + (flags[0] * 2) + flags[1];
		MicroInstruction mcode = microinstructionData[microcodeLocation];


		// Check for any reads and execute if applicable
		MicroInstruction readInstr = mcode & READ_MASK;
		switch (readInstr) [[likely]]
		{
		case READ_RA:
			bus = AReg;
			break;
		case READ_RB:
			bus = BReg;
			break;
		case READ_RC:
			bus = CReg;
			break;
		case READ_RM:
			bus = memoryBytes[memoryIndex];
			break;
		case READ_IR:
			bus = InstructionReg & ((1 << 11) - 1);
			break;
		case READ_CR:
			bus = programCounter;
			break;
		case READ_RE:
			bus = expansionPort;
			break;
		}


			// Find ALU modifiers
		MicroInstruction aluInstr = mcode & ALU_MASK;

		// Standalone microinstruction (ungrouped)
		if (mcode & STANDALONE_EO) [[unlikely]]
		{
			flags[0] = 0;
			flags[1] = 0;
			switch (aluInstr)
			{
			case ALU_SU: // Subtract
				flags[1] = 1;
				if (AReg - BReg == 0)
					flags[0] = 1;
				bus = AReg - BReg;
				if (bus < 0)
				{
					bus = 65534 + bus;
					flags[1] = 0;
				}
				break;

			case ALU_MU: // Multiply
				if (AReg * BReg == 0)
					flags[0] = 1;
				bus = AReg * BReg;
				if (bus >= 65534)
				{
					bus = bus - 65534;
					flags[1] = 1;
				}
				break;

			case ALU_DI: // Divide
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

				if (bus >= 65534)
				{
					bus = bus - 65534;
					flags[1] = 1;
				}
				break;

			case ALU_SL: // Logical bit shift left
				bus = AReg << (BReg & 0b1111);

				if (bus == 0)
					flags[0] = 1;

				if (bus >= 65534)
				{
					bus = bus - 65534;
					flags[1] = 1;
				}
				break;

			case ALU_SR: // Logical bit shift right
				bus = AReg >> (BReg & 0b1111);

				if (bus == 0)
					flags[0] = 1;

				if (bus >= 65534)
				{
					bus = bus - 65534;
					flags[1] = 1;
				}
				break;

			case ALU_AND: // Logical AND
				bus = AReg & BReg;

				if (bus == 0)
					flags[0] = 1;

				if (bus >= 65534)
				{
					bus = bus - 65534;
					flags[1] = 1;
				}
				break;

			case ALU_OR: // Logical OR
				bus = AReg | BReg;

				if (bus == 0)
					flags[0] = 1;

				if (bus >= 65534)
				{
					bus = bus - 65534;
					flags[1] = 1;
				}
				break;

			case ALU_NOT: // Logical NOT
				bus = ~AReg;

				if (bus == 0)
					flags[0] = 1;

				if (bus >= 65534)
				{
					bus = bus - 65534;
					flags[1] = 1;
				}
				break;

			default: // Add
				if (AReg + BReg == 0)
					flags[0] = 1;
				bus = AReg + BReg;
				if (bus >= 65534)
				{
					bus = bus - 65534;
					flags[1] = 1;
				}
				break;
			}
		}


			// Check for any writes and execute if applicable
		MicroInstruction writeInstr = mcode & WRITE_MASK;
		switch (writeInstr)
		{
		[[likely]] default: break;
		case WRITE_WA:
			AReg = bus;
			break;
		case WRITE_WB:
			BReg = bus;
			break;
		case WRITE_WC:
			CReg = bus;
			break;
		case WRITE_IW:
			InstructionReg = bus;
			break;
		case WRITE_WM:
			memoryBytes[memoryIndex] = bus;
			break;
		case WRITE_J:
			programCounter = bus;
			break;
		case WRITE_AW:
			memoryIndex = bus;
			break;
		case WRITE_WE:
			expansionPort = bus;
			break;
		}


		// Standalone microinstructions (ungrouped)
		if (mcode & STANDALONE_CE) [[unlikely]]
		{
			programCounter++;
		}

			if (mcode & STANDALONE_EI)
			{
				break;
			}
	}
}

void DrawNextPixel() {
	int characterRamValue = memoryBytes[characterRamIndex + 16382];
	bool charPixRomVal = characterRom[(characterRamValue * 64) + (charPixY * 8) + charPixX];

	int pixelVal = memoryBytes[pixelRamIndex];
	int r, g, b;

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

	set_pixel(&pixels, imgX, imgY, 64, r, g, b, 255);


	imgX++;
	charPixX++;
	if (charPixX >= 6) {
		charPixX = 0;
		characterRamIndex++;
	}

	// If x-coord is max, reset and increment y-coord
	if (imgX >= 64)
	{
		imgY++;
		charPixY++;
		charPixX = 0;
		imgX = 0;

		if (charPixY < 6)
			characterRamIndex -= 10;
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

		apply_pixels(pixels, texture, 64);
		DisplayTexture(gRenderer, texture);
	}

	pixelRamIndex++;
}

void Draw() {
	while (true) {
		DrawNextPixel();
		if (pixelRamIndex >= 65535) {
			pixelRamIndex = 61439;
			break;
		}
	}
}

void DrawPixel(int x, int y, int r, int g, int b)
{
	SDL_SetRenderDrawColor(gRenderer, r, g, b, 255);
	SDL_RenderDrawPoint(gRenderer, x, y);
}

std::string SimplifiedHertz(float input) {
	if (input == INFINITY)
		input = FLT_MAX;

	if (input >= 1000000000.0) // GHz
		return to_string(floor(input / 100000000.0f) / 10.0f) + " GHz";
	if (input >= 1000000.0) // MHz
		return to_string(floor(input / 100000.0f) / 10.0f) + " MHz";
	if (input >= 1000.0) // KHz
		return to_string(floor(input / 100.0f) / 10.0f) + " KHz";

	return to_string(round(input * 10.0f) / 10.0f) + " KHz";
}

int InitGraphics(const std::string& windowTitle, int width, int height, int pixelScale)
{
	int WINDOW_WIDTH = width;
	int WINDOW_HEIGHT = height;
	int PIXEL_SCALE = pixelScale;

	// Initialize SDL components
	SDL_Init(SDL_INIT_VIDEO);

	gWindow = SDL_CreateWindow(windowTitle.c_str(), 40, 40, WINDOW_WIDTH * PIXEL_SCALE, WINDOW_HEIGHT * PIXEL_SCALE, SDL_WINDOW_SHOWN | SDL_RENDERER_ACCELERATED);
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

std::string charToString(char* a)
{
	std::string s(a);
	return s;
}

// Gets range of bits inside of an integer <value> starting at <offset> inclusive for <n> range
unsigned BitRange(unsigned value, unsigned offset, unsigned n)
{
	return(value >> offset) & ((1u << n) - 1);
}

vector<std::string> explode(const std::string& str, const char& ch) {
	std::string next;
	vector<std::string> result;

	// For each character in the std::string
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++) {
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


// Convert assembly into bytes
vector<std::string> parseCode(const std::string& input)
{
	vector<std::string> outputBytes;
	for (int i = 0; i < 65535; i++)
		outputBytes.push_back("0000");

	std::string icopy = input;
	transform(icopy.begin(), icopy.end(), icopy.begin(), ::toupper);
	vector<std::string> splitcode = explode(icopy, '\n');

#if DEV_MODE
	cout << endl;
#endif

	int memaddr = 0;
	for (int i = 0; i < splitcode.size(); i++)
	{
		splitcode[i] = trim(splitcode[i]);
		if (splitcode[i] == "")
			continue;

		vector<std::string> splitBySpace = explode(splitcode[i], ' ');

		if (splitBySpace[0][0] == ',')
		{
#if DEV_MODE
			cout << ("-\t" + splitcode[i] + "\n");
#endif
			continue;
		}

		// Sets the specified memory location to a value:  set <addr> <val>
		if (splitBySpace[0] == "SET")
		{
			int addr = stoi(splitBySpace[1]);
			std::string hVal = DecToHexFilled(stoi(splitBySpace[2]), 4);
			if (addr <= 16382)
				outputBytes[addr] = hVal;
			else
				charRam[clamp(addr - 16383, 0, 143)] = stoi(splitBySpace[2]);
#if DEV_MODE
			cout << ("-\t" + splitcode[i] + "\t  ~   ~\n");
#endif
			continue;
		}

		// Set the current location in memory equal to a value: here <value>
		if (splitBySpace[0] == "HERE")
		{
			int addr = memaddr;
			std::string hVal = DecToHexFilled(stoi(splitBySpace[1]), 4);
			if (addr <= 16382)
				outputBytes[addr] = hVal;
			else
				charRam[clamp(addr - 16383, 0, 143)] = stoi(splitBySpace[1]);
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
	std::string processedOutput = "";
	processedOutput += "\nv3.0 hex words addressed\n";
	processedOutput += "000: ";
	for (int outindex = 0; outindex < outputBytes.size(); outindex++)
	{
		if (outindex % 8 == 0 && outindex != 0)
		{
			std::string locationTmp = DecToHexFilled(outindex, 3);
			transform(locationTmp.begin(), locationTmp.end(), locationTmp.begin(), ::toupper);
			processedOutput += "\n" + DecToHexFilled(outindex, 3) + ": ";
		}
		processedOutput += outputBytes[outindex] + " ";

		std::string ttmp = outputBytes[outindex];
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

void ComputeStepInstructions(const std::string& stepContents, char* stepComputedInstruction) {

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
				std::string binaryval = DecToBinFilled(minsother + 1, 4);
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
				std::string binaryval = DecToBinFilled(minsother + 1, 3);
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
				std::string binaryval = DecToBinFilled(minsother + 1, 4);
				stepComputedInstruction[12] = binaryval[0];
				stepComputedInstruction[13] = binaryval[1];
				stepComputedInstruction[14] = binaryval[2];
				stepComputedInstruction[15] = binaryval[3];
			}
		}

	}
}

// Generate microcode rom from instructioncodes array
void GenerateMicrocode()
{
	// Generate zeros in data
	vector<std::string> output;
	for (int osind = 0; osind < 2048; osind++) {
		output.push_back("00000");
	}

	// Remove spaces from instruction codes and make uppercase
	for (int cl = 0; cl < sizeof(instructioncodes) / sizeof(instructioncodes[0]); cl++)
	{
		std::string newStr = "";
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
	vector<std::string> seenNames;
	for (int cl = 0; cl < sizeof(instructioncodes) / sizeof(instructioncodes[0]); cl++)
	{
		std::string instName = explode(instructioncodes[cl], '(')[0];
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
	for (int ins = 0; ins < sizeof(instructioncodes) / sizeof(instructioncodes[0]); ins++) // Iterate through all definitions of instructions
	{
		int correctedIndex = ins;

		std::string startaddress = DecToBinFilled(correctedIndex, 5);

		vector<std::string> instSteps = explode(instructioncodes[0], '&');
		for (int step = 0; step < instSteps.size(); step++) // Iterate through every step
		{
			int actualStep = stoi(explode(instSteps[step], '=')[0]);
			std::string stepContents = explode(explode(instSteps[step], '=')[1], '|')[0];

			std::string midaddress = DecToBinFilled(actualStep, 4);

			char stepComputedInstruction[MICROINSTR_SIZE] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };
			ComputeStepInstructions(stepContents, stepComputedInstruction);


			// Compute flags combinations
			for (int flagcombinations = 0; flagcombinations < (sizeof(flagtypes) / sizeof(flagtypes[0])) * (sizeof(flagtypes) / sizeof(flagtypes[0])); flagcombinations++)
			{
				char endaddress[] = { '0', '0' };
				// Look for flags
				if (instSteps[step].find("|") != std::string::npos)
				{
					vector<std::string> inststepFlags = explode(explode(instSteps[step], '|')[1], ',');
					for (int flag = 0; flag < inststepFlags.size(); flag++) // Iterate through all flags in step
					{
						for (int checkflag = 0; checkflag < (sizeof(flagtypes) / sizeof(flagtypes[0])); checkflag++) // What is the index of the flag
						{
							if (inststepFlags[flag] == flagtypes[checkflag])
								endaddress[checkflag] = '1';
						}
					}
				}
				std::string tmpFlagCombos = DecToBinFilled(flagcombinations, 2);
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

		std::string startaddress = DecToBinFilled(correctedIndex, 5);

		vector<std::string> instSteps = explode(instructioncodes[correctedIndex], '&');
		for (int step = 0; step < instSteps.size(); step++) // Iterate through every step
		{
			int actualStep = stoi(explode(instSteps[step], '=')[0]);
			std::string stepContents = explode(explode(instSteps[step], '=')[1], '|')[0];

			std::string midaddress = DecToBinFilled(actualStep, 4);

			char stepComputedInstruction[MICROINSTR_SIZE] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };
			ComputeStepInstructions(stepContents, stepComputedInstruction);


			// Compute flags combinations
			for (int flagcombinations = 0; flagcombinations < (sizeof(flagtypes) / sizeof(flagtypes[0])) * (sizeof(flagtypes) / sizeof(flagtypes[0])); flagcombinations++)
			{
				char endaddress[] = { '0', '0' };
				int stepLocked[] = { 0, 0 };
				// If flags are specified in current step layer, set them to what is specified and lock that bit
				if (instSteps[step].find("|") != std::string::npos)
				{
					vector<std::string> inststepFlags = explode(explode(instSteps[step], '|')[1], ',');
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
				std::string tmpFlagCombos = DecToBinFilled(flagcombinations, 2);
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
	std::string processedOutput = "";
	processedOutput += "\nv3.0 hex words addressed\n";
	processedOutput += "000: ";
	for (int outindex = 0; outindex < output.size(); outindex++)
	{
		if (outindex % 8 == 0 && outindex != 0)
		{
			std::string locationTmp = DecToHexFilled(outindex, 3);
			transform(locationTmp.begin(), locationTmp.end(), locationTmp.begin(), ::toupper);
			processedOutput += "\n" + DecToHexFilled(outindex, 3) + ": ";
		}
		processedOutput += output[outindex] + " ";

		std::string ttmp = output[outindex];
		transform(ttmp.begin(), ttmp.end(), ttmp.begin(), ::toupper);

		std::string binversion = HexToBin(ttmp, MICROINSTR_SIZE);
		for (int i = 0; i < binversion.size(); i++)
		{
			microinstructionData[outindex] |= (binversion[i] == '1') << ((MICROINSTR_SIZE - 1) - i);
		}
	}


	// Save the data to ./microinstructions_cpu_v1
	fstream myStream;
	myStream.open("./microinstructions_cpu", ios::out);
	myStream << processedOutput;
}
