
#include <vector>
#include <algorithm> 
#include <string> 
#include <chrono>
#include <limits.h>
//#include <SDL.h>
//#include <SDL_mixer.h>
#include <sstream>
#include <fstream>
#include <codecvt>
#include "processing.h"
#include <filesystem>
#include <map>
#include <queue>
#include <time.h>

#include "VSSynth/VSSynth.h"

#include "armstrong-compiler.h"
//#include "synth.cpp"

using namespace VSSynth;
using namespace Generators;

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

std::string VERSION = "Astro-8 VERSION: v3.4.2-alpha";


#if UNIX
#include <unistd.h>
#elif WINDOWS
//#include <windows.h>
#include "escapi.h"
#endif

using namespace std;

bool compileOnly, assembleOnly, runAstroExecutable, verbose, superVerbose, usingWebcam, imageOnlyMode;

bool usingKeyboard = true, usingMouse = true, performanceMode = true, usingFileSystem = true;

uint16_t imageOnlyModeFrames = 10;
uint16_t imageOnlyModeFrameCount = 10;

uint16_t AReg = 0;
uint16_t BReg = 0;
uint16_t CReg = 0;
uint8_t BankReg = 0;
uint16_t InstructionReg = 0;
uint8_t flags[2] = { 0, 0 };
int bus = 0;
uint16_t outputReg = 0;
uint16_t memoryIndex = 0;
uint16_t programCounter = 0;

uint8_t imgX = 0;
uint8_t imgY = 0;
uint8_t charPixX = 0;
uint8_t charPixY = 0;
uint16_t characterRamIndex = 0;
uint16_t pixelRamIndex = 0;

// The register which stores the index of video buffer getting written to, and the video card reads and uses the opposite one.
bool VideoBufReg = false;


// 16000000 = 16.0MHz
uint32_t target_cpu_freq = 16000000;
#define TARGET_RENDER_FPS 60.0

vector<vector<uint16_t>> memoryBytes;
vector<vector<uint16_t>> videoBuffer;

uint8_t asciiToSdcii[600];
uint8_t ascToSdcii[600];
uint8_t sdciiToAscii[600];

std::string projectDirectory;
std::string executableDirectory;


// Refer to https://sam-astro.github.io/Astro8-Computer/docs/Architecture/Micro%20Instructions.html

#define MICROINSTR_SIZE 16
using MicroInstruction = uint16_t;
static_assert(sizeof(MicroInstruction) * 8 >= MICROINSTR_SIZE,
	"Size of MicroInstruction is too small, increase its width...");

MicroInstruction microinstructionData[2048];

enum ALUInstruction : MicroInstruction {
	ALU_SU = 0b0000000000000001,
	ALU_MU = 0b0000000000000010,
	ALU_DI = 0b0000000000000011,
	ALU_SL = 0b0000000000000100,
	ALU_SR = 0b0000000000000101,
	ALU_AND = 0b0000000000000110,
	ALU_OR = 0b0000000000000111,
	ALU_NOT = 0b0000000000001000,
	ALU_MASK = 0b0000000000001111,
};

enum ReadInstruction : MicroInstruction {
	READ_RA = 0b0000000000010000,
	READ_RB = 0b0000000000100000,
	READ_RC = 0b0000000000110000,
	READ_RM = 0b0000000001000000,
	READ_IR = 0b0000000001010000,
	READ_CR = 0b0000000001100000,
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
	WRITE_BNK = 0b0000010010000000,
	WRITE_VBUF = 0b0000010100000000,
	WRITE_MASK = 0b0000011110000000,
};

enum StandaloneInstruction : MicroInstruction {
	STANDALONE_FL = 0b0000100000000000,
	STANDALONE_EI = 0b0001000000000000,
	STANDALONE_ST = 0b0010000000000000,
	STANDALONE_CE = 0b0100000000000000,
	STANDALONE_EO = 0b1000000000000000,
};

using FullInstruction = uint16_t;
enum AllInstructions : FullInstruction {
	NOP = 0,
	AIN = 1,
	BIN,
	CIN,
	LDIA,
	LDIB,
	STA,
	ADD,
	SUB,
	MULT,
	DIV,
	JMP,
	JMPZ,
	JMPC,
	JREG,
	LDAIN,
	STAOUT,
	LDLGE,
	STLGE,
	LDW,
	SWP,
	SWPC,
	PCR,
	BSL,
	BSR,
	AND,
	OR,
	NOT,
	BNK,
	VBUF,
	BNKC,
	LDWB,
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
vector<vector<std::string>> parseCode(const std::string& input);
void GenerateMicrocode();
std::string SimplifiedHertz(float input);
int ConvertAsciiToSdcii(int asciiCode);
void Save_Frame(const ::std::string& name, vector<unsigned char> img_vals);
static void write_samples(int16_t* s_byteStream, long begin, long end, long length);
uint16_t ConvertNoteIndexToFrequency(uint8_t index);
uint16_t GetMem(uint16_t bank, uint16_t address);
void SetMem(uint16_t bank, uint16_t address, uint16_t data);
void GenerateAsciiSdciiTables();
int ConvertSdciiToAscii(int sdciiCode);

SDL_Texture* texture;
std::vector< unsigned char > pixels(108 * 108 * 4, 0);


//Mix_Chunk* waveforms[4];

float lastFreq[4] = { 440, 440, 440, 440 };
bool isPlaying[4] = { false, false, false, false };
Tone* tone[4];


vector<std::string> instructions = { "NOP", "AIN", "BIN", "CIN", "LDIA", "LDIB", "STA", "ADD", "SUB", "MULT", "DIV", "JMP", "JMPZ","JMPC", "JREG", "LDAIN", "STAOUT", "LDLGE", "STLGE", "LDW", "SWP", "SWPC", "PCR", "BSL", "BSR", "AND", "OR", "NOT", "BNK", "VBUF", "BNKC", "LDWB" };

std::string microinstructions[] = { "EO", "CE", "ST", "EI", "FL" };
std::string writeInstructionSpecialAddress[] = { "WA", "WB", "WC", "IW", "DW", "WM", "J", "AW", "BNK", "VBF" };
std::string readInstructionSpecialAddress[] = { "RA", "RB", "RC", "RM", "IR", "CR" };
std::string aluInstructionSpecialAddress[] = { "SU", "MU", "DI", "SL", "SR", "AND","OR","NOT" };
std::string flagtypes[] = { "ZEROFLAG", "CARRYFLAG" };

std::string instructioncodes[] = {
		"fetch( 0=cr,aw & 1=rm,iw,ce & 2=ei", // Fetch
		"ain( 2=aw,ir & 3=wa,rm & 4=ei", // LoadA
		"bin( 2=aw,ir & 3=wb,rm & 4=ei", // LoadB
		"cin( 2=aw,ir & 3=wc,rm & 4=ei", // LoadC
		"ldia( 2=wa,ir & 3=ei", // Load immediate A <val>
		"ldib( 2=wb,ir & 3=ei", // Load immediate B <val>
		//"rdexp( 2=ir,exi & 3=wa,re & 4=ei", // Read from expansion port <index> to register A
		//"wrexp( 2=ir,exi & 3=ra,we & 4=ei", // Write from reg A to expansion port <index>
		"sta( 2=aw,ir & 3=ra,wm & 4=ei", // Store A <addr>
		"add( 2=wa,eo,fl & 3=ei", // Add
		"sub( 2=wa,eo,su,fl & 3=ei", // Subtract
		"mult( 2=wa,eo,mu,fl & 3=ei", // Multiply
		"div( 2=wa,eo,di,fl & 3=ei", // Divide
		"jmp( 2=bnk & 3=cr,aw & 4=rm,j & 5=ei", // Jump to address following instruction
		"jmpz( 2=bnk & 3=cr,aw & 4=ce,rm & 5=j | zeroflag & 6=ei", // Jump if zero to address following instruction
		"jmpc( 2=bnk & 3=cr,aw & 4=ce,rm & 5=j | carryflag & 6=ei", // Jump if carry to address following instruction
		"jreg( 2=ra,j & 3=ei", // Jump to the address stored in Reg A
		"ldain( 2=ra,aw & 3=wa,rm & 4=ei", // Use reg A as memory address, then copy value from memory into A
		"staout( 2=ra,aw & 3=rb,wm & 4=ei", // Use reg A as memory address, then copy value from B into memory
		"ldlge( 2=bnk & 3=cr,aw & 4=bnk,ir & 5=ce,rm,aw & 6=rm,wa & 7=ei", // Use value directly after counter as address, then copy value from memory to reg A and advance counter by 2
		"stlge( 2=bnk & 3=cr,aw & 4=bnk,ir & 5=ce,rm,aw & 6=ra,wm & 7=ei", // Use value directly after counter as address, then copy value from reg A to memory and advance counter by 2
		"ldw( 2=bnk,ir & 3=cr,aw & 4=ce,rm,wa & 5=ei", // Load value directly after counter into A, and advance counter by 2
		"swp( 2=ra,wc & 3=wa,rb & 4=rc,wb & 5=ei", // Swap register A and register B (this will overwrite the contents of register C, using it as a temporary swap area)
		"swpc( 2=ra,wb & 3=wa,rc & 4=rb,wc & 5=ei", // Swap register A and register C (this will overwrite the contents of register B, using it as a temporary swap area)
		"pcr( 2=cr,wa & 3=ei", // Program counter read, get the current program counter value and put it into register A
		"bsl( 2=sl,wa,eo,fl & 3=ei", // Bit shift left A register, the number of bits to shift determined by the value in register B
		"bsr( 2=sr,wa,eo,fl & 3=ei", // Bit shift right A register, the number of bits to shift determined by the value in register B
		"and( 2=and,wa,eo,fl & 3=ei", // Logical AND operation on register A and register B, with result put back into register A
		"or( 2=or,wa,eo,fl & 3=ei", // Logical OR operation on register A and register B, with result put back into register A
		"not( 2=not,wa,eo,fl & 3=ei", // Logical NOT operation on register A, with result put back into register A
		"bnk( 2=bnk,ir & 3=ei", // Change bank, changes the memory bank register to the value specified <val>
		"vbuf( 2=vbf & 3=ei", // Swap the video buffer
		"bnkc( 2=rc,bnk & 3=ei", // Change bank to C register
		"ldwb( 2=bnk,ir & 3=cr,aw & 4=ce,rm,wb & 5=ei", // Load value directly after counter into B, and advance counter by 2
};

std::string helpDialog = R"V0G0N(
Usage: astro8 [options] <path>

Options:
    -h, --help               Display this help menu
    --version                Display the current version
    -c, --compile            Only compile and assemble Armstrong code to .ASM.
                             Will not start emulator.
    -a, --assemble           Only assemble assembly code into AEXE. Will not
                             start emulator.
    -r, --run                Run an already assembled program in AstroEXE format
                             (program.AEXE)
    -nk, --nokeyboard        Disable the keyboard input
    -wb, --webcam            Enable webcam (uses default, only works on Windows)
    -nm, --nomouse           Disable the mouse input
    -v, --verbose            Write extra data to console for better debugging
    -vv, --superverbose      Write a lot of extra data to console for even better
                             debugging
    -cm, --classicmode       Run Emulator in classic mode, using slow but
                             realistic microcode instead of high performance
    -f, --freq <value>       Override the default CPU target frequency with your
                             own.      Default = 16    higher = faster
                             High frequencies may be too hard to reach for some cpus
    --imagemode [frames]     Don't render anything, instead capture [frames] number
                             of frames, which is 10 by default, and save to disk
)V0G0N";


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
	//for (int i = 0; i < sizeof(waveforms) / sizeof(waveforms[0]); i++)
	//{
	//	//Mix_FreeChunk(waveforms[i]);
	//	waveforms[i] = NULL;
	//}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	//Mix_Quit();
}

int clamp(int x, int min, int max) {
	if (x < min)
		return min;
	if (x > max)
		return max;

	return x;
}


// Class for key press input
class KeyPress {
public:
	uint16_t keyCode;
	int uses = 2;
	bool isDown = true; // Differ between sending down/up signal
	friend bool operator== (const KeyPress& k1, const KeyPress& k2);

	KeyPress(int key, bool isPressed) {
		keyCode = key;
		isDown = isPressed;
	}
};
bool operator== (const KeyPress& k1, const KeyPress& k2)
{
	return k1.keyCode == k2.keyCode && k1.isDown == k2.isDown;
}
bool operator!= (const KeyPress& k1, const KeyPress& k2)
{
	return !(k1.keyCode == k2.keyCode && k1.isDown == k2.isDown);
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

int GenerateCharacterROM() {

	// Generate character rom from existing generated file (generate first using C# assembler)
	std::string chline;

	const std::string charsetFilename = executableDirectory + "/char_set_memtape";
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

	return chline.length();
}

int main(int argc, char** argv)
{
#if DEV_MODE
		verbose = true;
#endif

	// Fill the memory
	memoryBytes = vector<vector<uint16_t>>(6, vector<uint16_t>(65535, 0));
	videoBuffer = vector<vector<uint16_t>>(2, vector<uint16_t>(11990, 0));

	//// Fill video buffers with random data to emulate real ram chip
	std::srand(time(0));
	std::generate(videoBuffer[0].begin(), videoBuffer[0].end(), std::rand);
	std::generate(videoBuffer[1].begin(), videoBuffer[1].end(), std::rand);
	videoBuffer[1][148] = 14;
	videoBuffer[1][149] = 27;
	videoBuffer[1][150] = 27;
	videoBuffer[1][151] = 32;
	videoBuffer[1][152] = 21;
	videoBuffer[1][153] = 26;
	videoBuffer[1][154] = 19;
	videoBuffer[1][155] = 54;
	videoBuffer[1][156] = 54;
	videoBuffer[1][157] = 54;


	// Get the executable's installed directory
	executableDirectory = filesystem::weakly_canonical(filesystem::path(argv[0])).parent_path().string();

	std::string code = "";
	std::string filePath = "";
	std::string programName = "program";

	// If no arguments are provided, ask for a path
	if (argc == 1)
	{
		PrintColored("No arguments or path detected. If this is your first time using this program,\nhere is the help menu for assistance:", yellowFGColor, "");
		cout << "\n" << helpDialog << "\n";
		SYS_PAUSE;
	}

	// Search for options in arguments
	for (int i = 1; i < argc; i++)
	{
		string argval = trim(argv[i]);
		if (argval == "-h" || argval == "--help") { // Print help dialog
			PrintColored(VERSION, blackFGColor, whiteBGColor);
			cout << "\n" << helpDialog << "\n";
			exit(1);
		}
		else if (argval == "--version") { // Print version
			PrintColored(VERSION, blackFGColor, whiteBGColor);
			exit(1);
		}
		else if (argval == "-c" || argval == "--compile") // Only compile and assemble code. Will not start emulator.
			compileOnly = true;
		else if (argval == "-a" || argval == "--assemble") // Only assemble code. Will not start emulator.
			assembleOnly = true;
		else if (argval == "-r" || argval == "--run") // Run an already assembled program in AstroEXE format
			runAstroExecutable = true;
		else if (argval == "-nk" || argval == "--nokeyboard") // Disable the keyboard input
			usingKeyboard = false;
		else if (argval == "-nfs" || argval == "--nofilesystem") // Disable the file access
			usingFileSystem = false;
		else if (argval == "-wb" || argval == "--webcam") { // Enable webcam (uses default, only works on Windows)
			if (WINDOWS)
				usingWebcam = true;
			else
				PrintColored("\n! Could not enable Webcam, only works on Windows devices. !\n", yellowFGColor, "");
		}
		else if (argval == "-nm" || argval == "--nomouse") // Disable the mouse input
			usingMouse = false;
		else if (argval == "-v" || argval == "--verbose") // Write extra data to console for better debugging
			verbose = true;
		else if (argval == "-vv" || argval == "--superverbose") // Write extra data to console for better debugging
			superVerbose = true;
		else if (argval == "-cm" || argval == "--classicmode") // Run Emulator in classic mode
			performanceMode = false;
		else if (argval == "--imagemode") { // Don't render anything, instead capture <frames> number of frames and save to disk
			try
			{
				imageOnlyMode = true;
				imageOnlyModeFrames = stoi(argv[i + 1]);
				imageOnlyModeFrameCount = imageOnlyModeFrames;
				i++;
			}
			catch (const std::exception&)
			{
			}
		}
		else if (argval == "-f" || argval == "--freq") { // Override the default CPU frequency with your own.
			try
			{
				target_cpu_freq = stoi(argv[i + 1]) * 1000000;
				i++;
			}
			catch (const std::exception&)
			{
				PrintColored("\nError: specify a valid integer frequency after -f/--freq option ", redFGColor, "");
				cout << "\n\nPress Enter to Exit...";
				cin.ignore();
				exit(1);
			}
		}
		else // If not an option, then it should be a path
			filePath = argval;
	}


	// Open and read the file from the path
	//if (split(filePath, "\n")[0].find('/') != std::string::npos || split(filePath, "\n")[0].find('\\') != std::string::npos) {
		std::string path = trim(split(filePath, "\n")[0]);
		path.erase(std::remove(path.begin(), path.end(), '\''), path.end()); // Remove all single quotes
		path.erase(std::remove(path.begin(), path.end(), '\"'), path.end()); // Remove all double quotes
		programName = path.substr(path.find_last_of("/\\") + 1, path.size());

		// Open and read file
		std::string li;
		ifstream fileStr(path);
		if (fileStr.is_open())
		{
			while (getline(fileStr, li)) {
				code += trim(li) + "\n";
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
		projectDirectory = std::filesystem::canonical(projectDirectory).string() + (WINDOWS ? "\\" : "/");

	/*}
	else if (argc != 1) {
		PrintColored("\nError: could not open file ", redFGColor, "");
		PrintColored("\"" + code + "\"\n", brightBlueFGColor, "");
		cout << "\n\nPress Enter to Exit...";
		cin.ignore();
		exit(1);
	}*/


	// Determine if the file is an AstroExecutable - AEXE
	if (trim(split(code, "\n")[0]) == "ASTRO-8 AEXE Executable file")
		runAstroExecutable = true;


	// Print `building` message if applicable
	if (!runAstroExecutable) {
		PrintColored("Building:", blackFGColor, whiteBGColor);
		cout << "\n\n";
	}


	// Generate required resources if the code is to be executed
	if ((!compileOnly && !assembleOnly) || runAstroExecutable) {
		cout << "* Generating emulation resources:\n";

		// Generate ROM
		cout << "   -  Generating Character ROM...\n";
		int pixnum = GenerateCharacterROM();

		// Generate microcode
		cout << "   -  Generating microcode from instruction set...\n\n";
		GenerateMicrocode();
	}


	// If the code inputted is marked as written in armstrong with #AS,
	// or the user has used the "--compile" option
	if ((split(code, "\n")[0] == "#AS" || compileOnly) && !assembleOnly && !runAstroExecutable)
	{
		cout << "* Preprocessing raw...";
		vector<std::string> codelines = PreProcess(code);
		PrintColored("  Done!\n", brightGreenFGColor, "");

		for (int i = 0; i < codelines.size(); i++)
		{
			// If including another file:    #include "./path/to/file.arm"
			if (split(codelines[i], " ")[0] == "#include") {
				std::string clCpy = codelines[i];

				codelines.erase(codelines.begin() + i); // Remove the #include

				std::string path = trim(split(clCpy, " ")[1]);
				path.erase(std::remove(path.begin(), path.end(), '\''), path.end()); // Remove all single quotes
				path.erase(std::remove(path.begin(), path.end(), '\"'), path.end()); // Remove all double quotes
				path = path.substr(path.find_last_of("/\\") + 1, path.size());

				// If the path is relative, append the known project path to make it absolute.
				if (path[0] == '.')
					if (projectDirectory[projectDirectory.size() - 1] != '.')
						path = projectDirectory + path;

				// Open and read file, appending code onto it after
				std::string codeTmp = "";
				std::string li;
				ifstream fileStr(path);
				if (fileStr.is_open())
				{
					// Add jump to end (this prevents this code from executing accidentally)
					int randNum = rand() % 9999;
					codeTmp += "\ngoto #" + path + to_string(randNum) + "\n";

					// Get all lines of file
					while (getline(fileStr, li)) {
						if (li != "#AS") // We don't need another Armstrong label, so we can remove it
							codeTmp += li + "\n";
					}

					// Label to jump to
					codeTmp += "\n#" + path + to_string(randNum);
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
		cout << "* Begin Compiling Armstrong...\n";
		code = CompileCode(code);

		if (compileOnly) {
			// Store asm into an Astrisc Assembly *.ASM file
			std::ofstream f(projectDirectory + programName + ".asm");
			f << code;
			f.close();
			PrintColored("Assembly file written to " + projectDirectory + programName + ".asm\n", whiteFGColor, "");
		}

		if (code != "") {
			if (verbose) {
				cout << "   -  Output:\n";
				ColorAndPrintAssembly(code, instructions);
			}
		}
		else
			exit(0);
	}
	else if (!runAstroExecutable && verbose)
		ColorAndPrintAssembly(code, instructions);


	// Attempt to parse assembly, will throw error if not proper assembly
	if (!runAstroExecutable) {
		try
		{
			// Generate memory from code and convert from hex to decimal
			vector<vector<std::string>> mbytes = parseCode(code);
			for (int membank = 0; membank < mbytes.size(); membank++)
				for (int memindex = 0; memindex < mbytes[membank].size(); memindex++)
					memoryBytes[membank][memindex] = (HexToDec(mbytes[membank][memindex]));

			// Store memory into an .AEXE file
			std::ofstream f(projectDirectory + programName + ".aexe");
			f << "ASTRO-8 AEXE Executable file" << '\n';
			f << (usingKeyboard == true ? "1" : "0");
			f << (usingWebcam == true ? "1" : "0");
			f << (usingMouse == true ? "1" : "0");
			f << (verbose == true ? "1" : "0");
			f << "," << std::to_string(target_cpu_freq);
			f << '\n';
			for (vector<string>::const_iterator i = mbytes[0].begin(); i != mbytes[0].end(); ++i) {
				f << *i << '\n';
			}
			f.close();
			PrintColored("Binary executable written to " + projectDirectory + programName + ".aexe\n", whiteFGColor, "");
		}
		catch (const std::exception& e)
		{
			cout << e.what() << endl;
			PrintColored("\nError: failed to parse code. if you are trying to run Armstrong, make sure the first line of code contains  \"#AS\" ", redFGColor, "");
			cout << "\n\nPress Enter to Exit...";
			cin.ignore();
			exit(1);
		}
	}

	// User is trying to run an .AEXE file, so parse it into the memory vector
	if (runAstroExecutable) {
		vector<std::string> filelines = split(code, "\n");

		// Make sure it is a valid AEXE file
		if (trim(filelines[0]) == "ASTRO-8 AEXE Executable file") {
			// Use values on the second line as static options
			// This lets people distribute AEXE files without having to describe
			// the exact options to get it working
			if (trim(filelines[1]).size() >= 1) {
				usingKeyboard = trim(filelines[1])[0] == '1';
				usingWebcam = trim(filelines[1])[1] == '1';
				usingMouse = trim(filelines[1])[2] == '1';
				verbose = trim(filelines[1])[3] == '1';
				target_cpu_freq = std::stoi(trim(split(filelines[1], ",")[1]));
			}

			// Skip two lines to begin reading AEXE data
			for (int memindex = 2; memindex < filelines.size(); memindex++)
				memoryBytes[0][memindex - 2] = HexToDec(filelines[memindex]);
		}
		else
		{
			PrintColored("\nInvalid Executable file. Possibly it is out of date or corrupted, or may simply be missing the header:  \"ASTRO-8 AEXE Executable file\"", redFGColor, "");
			cout << "\n\nPress Enter to Exit...";
			cin.ignore();
			exit(1);
		}
	}

	// Stop executing if successfully compiled/assembled
	if (compileOnly || assembleOnly) {
		PrintColored("\nFinished successfully", greenFGColor, "");
		exit(1);
	}


#if WINDOWS
	// Start Webcam if specified and compatable
	struct SimpleCapParams capture;
	while (usingWebcam) {
		int devices = setupESCAPI();


		if (devices == 0)
		{
			PrintColored("\n! Webcam initialization failure or no devices found. !\n", yellowFGColor, "");
			usingWebcam = false;
			break;
		}

		capture.mWidth = 108;
		capture.mHeight = 108;
		capture.mTargetBuf = new int[108 * 108];

		if (initCapture(0, &capture) == 0)
		{
			PrintColored("\n! Webcam capture failure. !\n", yellowFGColor, "");
			usingWebcam = false;
			break;
		}

		//while (isCaptureDone(0) == 0)
		//{
		//	/* Wait until capture is done. */
		//}
		break;
	}
#endif


	// Start Emulation

	cout << "\n\n";
	PrintColored("Starting Emulation...", blackFGColor, whiteBGColor);
	cout << "\n\n";

	if (!imageOnlyMode) // No need to initialize graphics if no rendering is taking place
		InitGraphics("Astro-8 Emulator", 108, 108, 5);
	else { // Otherwise create required directory if outputting images
		std::filesystem::create_directory(projectDirectory + "./frames");
		cout << "Created Directory at: \"" + (projectDirectory + "./frames") + "\"" << endl;
	}

	GenerateAsciiSdciiTables();


	bool keyPress = false;
	bool running = true;
	SDL_Event event;
	SDL_Event lastEvent = SDL_Event();
	SDL_Event pendingEvent;
	int eventUses = 0;

	int updateCount = 0;
	int frameCount = 0;
	auto lastSecond = std::chrono::high_resolution_clock::now();
	auto lastFrame = lastSecond;
	auto lastTick = lastSecond;

	uint16_t lastKey = 0;
	uint16_t samekeyUses = 10;

#if WINDOWS
	uint16_t webcamPixelLoc = 0;
#endif
	deque<KeyPress> keyRollover = {};
	std::map<int, bool> pressedKeys;



	// Channel A
	Tone t0(
		[](double frequency, double time) {
			return Waveforms::square(
				frequency,
				time,
				0); // LFO
		});
	tone[0] = &t0;
	(*tone[0]).setVolume(30);
	// Channel B
	Tone t1(
		[](double frequency, double time) {
			return Waveforms::square(
				frequency,
				time,
				0); // LFO
		});
	tone[1] = &t1;
	(*tone[1]).setVolume(30);
	// Channel C
	Tone t2(
		[](double frequency, double time) {
			return Waveforms::triangle(
				frequency,
				time,
				0); // LFO
		});
	tone[2] = &t2;
	(*tone[2]).setVolume(30);
	// Channel D
	Tone t3(
		[](double frequency, double time) {
			return Waveforms::noise();
		});
	tone[3] = &t3;
	(*tone[3]).setVolume(30);

	//(*tone[0]).playNote(Notes::C4);
	//(*tone[1]).playNote(Notes::C4);
	//(*tone[2]).playNote(Notes::C4);
	//(*tone[3]).playNote(Notes::C4);

	// Create a synthesizer, with default settings
	Synthesizer synth;

	// Open the synth for playback with the sine wave we have created
	synth.open();
	synth.addSoundGenerator(tone[0]);
	synth.addSoundGenerator(tone[1]);
	synth.addSoundGenerator(tone[2]);
	synth.addSoundGenerator(tone[3]);
	synth.unpause();


#if WINDOWS
	// Request initial webcam capture if on
	if (usingWebcam)
		doCapture(0);
#endif

	// Draw the initial random data in the buffer, then clear it which would be done by the BIOS
	Draw();
	for (size_t i = 0; i < 10000000; i++) {videoBuffer[0][0] = 0;}
	videoBuffer = vector<vector<uint16_t>>(2, vector<uint16_t>(11990, 0));

	std::string receivedPath;
	std::string receivedData;
	bool returningFileData = false;
	uint16_t fileData[65535*4];
	int fileIterator = 0;
	uint16_t fileLength = 0;
	ofstream outputProgramFileStream;


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
				<< "Freq: " << SimplifiedHertz(updateCount)
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
		if (tickDiff > (numUpdates * 1000.0 / target_cpu_freq)) {
			lastTick = startTime;
			for (int i = 0; i < numUpdates; ++i)
				Update();
			updateCount += numUpdates;
		}

		// Frame
		if (frameDiff > (1000.0 / TARGET_RENDER_FPS)) {
			lastFrame = startTime;
			//Draw();
			++frameCount;
			pendingEvent = SDL_Event();
			bool keyboardDecided = false;
			int undecidedKey = 168;

			// If the user has activated the webcam feature, write current webcam pixel value to expansion port
#if WINDOWS
			if (usingWebcam) {
				// If the program is asking for more data, reply
				if (memoryBytes[1][53503] >= 0b1000000000000000) {
					// Message request looks like:
					// Format: 0b10 YYYYYYY XXXXXXX    (where (X,Y) is the start pixel)
					uint16_t xReq = memoryBytes[1][53503] & 0b1111111;
					uint16_t yReq = (memoryBytes[1][53503] >> 7) & 0b1111111;

					uint16_t outputWord = 0;

					// Colors come in groups of 7, like 0b00 CC CC CC CC CC CC CC
					// The first 2 bits are blank, since they are used for controls
					// The groups are ordered from right to left (CC at right end is the Leftmost pixel location)

					// Iterate the 7 groups
					for (int gg = 0; gg < 7; gg++) {
						uint32_t pixVal = capture.mTargetBuf[xReq + (yReq * 108) + gg];
						uint16_t compressedColor = ((pixVal & 255) + ((pixVal >> 8) & 255) + ((pixVal >> 16) & 255)) / 255;
						outputWord = outputWord | (compressedColor << (gg * 2));
					}
					memoryBytes[1][53503] = outputWord;

					// Then the next 8 groups (uses +1 more expansion ports)
					for (int i = 0; i < 1; i++)
					{
						outputWord = 0;
						for (int gg = 0; gg < 8; gg++) {
							int targetIndex = xReq + (yReq * 108) + gg + 7 + (i * 8);
							uint32_t pixVal = capture.mTargetBuf[targetIndex >= 108 * 108 ? targetIndex - 108 * 108 : targetIndex];
							uint16_t compressedColor = ((pixVal & 255) + ((pixVal >> 8) & 255) + ((pixVal >> 16) & 255)) / 255;
							outputWord = outputWord | (compressedColor << (gg * 2));
						}
						memoryBytes[1][53504 + i] = outputWord;
					}
				}
				// If command is 0b01, then capture image
				else if (memoryBytes[1][53503] == 0b0100000000000000) {
					doCapture(0);
					memoryBytes[1][53503] = 0;
					cout << endl << "picture" << endl;
				}
			}
#endif



			if (usingFileSystem) {
				if (returningFileData) {
					if (memoryBytes[1][53505] == 0) {
						memoryBytes[1][53505] = fileData[fileIterator] | 0b100000000000000;
						if (fileIterator >= fileLength) {
							fileIterator = 0;
							returningFileData = false;
							memoryBytes[1][53505] = 4095;
						}
						else {
							fileIterator++;
						}
					}
				}
				else
					if (memoryBytes[1][53505] >= 0b1111000000000000) { // Save raw data
						uint8_t ch = memoryBytes[1][53505] & 255; // SDCII format character
						if (ch == 85 && !outputProgramFileStream.is_open()) { // If newline character, attempt to open new file
							outputProgramFileStream.open(receivedPath);
							receivedPath = "";
						}
						else if (ch == 78 && outputProgramFileStream.is_open()) { // If newline character, attempt to open new file
							outputProgramFileStream.close();
						}
						else {
							if (outputProgramFileStream.is_open())
								outputProgramFileStream << (char)(ConvertSdciiToAscii(ch));
							else
								receivedPath += (char)(ConvertSdciiToAscii(ch));
						}
						memoryBytes[1][53505] = 0;
					}
					else if (memoryBytes[1][53505] >= 0b1110000000000000) { // Save data
						uint8_t ch = memoryBytes[1][53505] & 255; // SDCII format character
						if (ch == 85 && !outputProgramFileStream.is_open()) { // If newline character, attempt to open new file
							outputProgramFileStream.open(receivedPath);
							outputProgramFileStream << "Astro-8 Binary Text File Format\n";
							outputProgramFileStream << "<settings>\n";
							receivedPath = "";
						}
						else if (ch == 78 && outputProgramFileStream.is_open()) { // If newline character, attempt to open new file
							outputProgramFileStream.close();
						}
						else {
							if (outputProgramFileStream.is_open())
								outputProgramFileStream << DecToHexFilled(ch, 4) << "\n";
							else
								receivedPath += (char)(ConvertSdciiToAscii(ch));
						}
						memoryBytes[1][53505] = 0;
					}
					else if (memoryBytes[1][53505] >= 0b1101000000000000) { // Load raw text data
						uint8_t ch = memoryBytes[1][53505] & 255; // SDCII format character
						if (ch == 85) { // If newline character, attempt to load from file
							cout << "\n\nLoading from file: " << receivedPath << endl << endl;
							int it = 0;

							fstream fin(receivedPath, fstream::in);
							char cc;
							while (fin >> noskipws >> cc) {
								fileData[it] = ascToSdcii[cc];
								it++;
							}
							fileLength = it;
							cout << it << " characters read\n";
							fin.close();
							receivedPath = "";

							memoryBytes[1][53505] = 0;
							returningFileData = true;
						}
						else {
							receivedPath += (char)(ConvertSdciiToAscii(ch));
							memoryBytes[1][53505] = 0;
						}
					}
					else if (memoryBytes[1][53505] >= 0b1100000000000000) { // Load data
						uint8_t ch = memoryBytes[1][53505] & 255; // SDCII format character
						if (ch == 85) { // If newline character, attempt to load from file
							cout << "\n\nLoading from file: " << receivedPath << endl << endl;
							ifstream hexFile;
							hexFile.open(receivedPath);
							receivedPath = "";
							std::string tmp;
							int it = 0;
							if (hexFile.is_open()) {
								getline(hexFile, tmp);
								getline(hexFile, tmp);
								while (getline(hexFile, tmp))
								{
									fileData[it] = HexToDec(tmp);

									it++;
								}
								cout << it << " lines read\n";
								fileLength = it;
							}
							hexFile.close();
							memoryBytes[1][53505] = 0;
							returningFileData = true;
						}
						else {
							receivedPath += (char)(ConvertSdciiToAscii(ch));
							memoryBytes[1][53505] = 0;
						}
					}
					else if (memoryBytes[1][53505] >= 0b1000000000000000) { // Load data and run
						uint8_t ch = memoryBytes[1][53505] & 255; // SDCII format character
						if (ch == 85) { // If newline character, attempt to load from file
							cout << "\n\nStarting from file: " << receivedPath << endl << endl;
							ifstream hexFile;
							hexFile.open(receivedPath);
							receivedPath = "";
							std::string tmp;
							int it = 0;
							if (hexFile.is_open()) {
								// Clear memory before loading
								for (int i = 0; i < memoryBytes.size(); i++) {
									memoryBytes[0][i] = 0;
									memoryBytes[1][i] = 0;
								}
								for (int i = 0; i < videoBuffer[0].size(); i++) {
									videoBuffer[0][i] = 0;
									videoBuffer[1][i] = 0;
								}
								getline(hexFile, tmp);
								getline(hexFile, tmp);
								while (getline(hexFile, tmp))
								{
									memoryBytes[0][it] = HexToDec(tmp);

									it++;
								}
								programCounter = 0;
								cout << it << " lines read\n";
							}
							hexFile.close();
							memoryBytes[1][53505] = 0;
						}
						else {
							receivedPath += (char)(ConvertSdciiToAscii(ch));
							memoryBytes[1][53505] = 0;
						}
					}
			}


			// Poll all input events
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_KEYDOWN)
					if (event.key.keysym.scancode == SDLK_g)
					{
						SDL_SetRelativeMouseMode(SDL_FALSE);
						SDL_ShowCursor(SDL_ENABLE);
					}
				// Quit if received the quit event
				if (event.type == SDL_QUIT)
					running = false;
				// If using the keyboard in the expansion port
				if (usingKeyboard) {
					/*if (event.type == SDL_KEYDOWN)
						pressedKeys[(int)(event.key.keysym.scancode)] = true;
					else if (event.type == SDL_KEYUP)
						pressedKeys[(int)(event.key.keysym.scancode)] = false;*/

					if (event.type == SDL_KEYDOWN) {
						int keyCode = (int)(event.key.keysym.scancode);
						// If the key is already pressed, don't process this press
						if (pressedKeys[keyCode] == true)
							continue;
						//std::vector<KeyPress>::iterator keyIt = std::find(keyRollover.begin(), keyRollover.end(), KeyPress((int)(event.key.keysym.scancode), true));
						// Ignore if the key is already in the rollover queue.
						// Otherwise, add it to the queue
						deque<KeyPress>::iterator it = find(keyRollover.begin(), keyRollover.end(), KeyPress(keyCode, true));
						if (it == keyRollover.end()) {
							keyRollover.push_back(KeyPress(keyCode, true));
							pressedKeys[keyCode] = true;
						}
					}
					else if (event.type == SDL_KEYUP) {
						int keyCode = (int)(event.key.keysym.scancode);
						// If the key is not pressed, don't process this un-press
						if (pressedKeys[keyCode] == false)
							continue;
						deque<KeyPress>::iterator it = find(keyRollover.begin(), keyRollover.end(), KeyPress(keyCode, false));
						if (it == keyRollover.end()) {
							keyRollover.push_back(KeyPress(keyCode, false));
							pressedKeys[keyCode] = false;
						}
					}
				}
				// If using the mouse in the expansion port
				if (usingMouse)
					if (event.type == SDL_MOUSEMOTION) {
						//// Get mouse relative movement from last position
						//int mXRel;
						//int mYRel;

						//SDL_GetRelativeMouseState(&mXRel, &mYRel);

						//// Automatically convert to twos compliment if the number is less than zero, otherwise pass as-is
						//mXRel = mXRel < 0 ? ((mXRel /4) << 6) & 0b111111000000 : (((~mXRel/4) + 1) << 6) & 0b111111000000;
						//mYRel = -mYRel < 0 ? (-mYRel /4) & 0b111111 : ((~-mYRel /4) + 1) & 0b111111;

						// This system sends the current coordinates of the mouse pointer,
						// which is different than how a mouse actually works I'm quite sure, 
						// And instead they send the relative movement since the last data send, like the above code tried to implement.
						memoryBytes[1][53501] = ((event.motion.x << 7) + event.motion.y) + (memoryBytes[1][53501] & 0b1100000000000000);

					}
					else if (event.type == SDL_MOUSEBUTTONDOWN) {
						////SDL_SetRelativeMouseMode(SDL_TRUE);
						if (event.button.button == 1)      // Left Mouse Button Down
							memoryBytes[1][53501] = 16384 | memoryBytes[1][53501];
						else if (event.button.button == 3) // Right Mouse Button Down
							memoryBytes[1][53501] = 32768 | memoryBytes[1][53501];
					}
					else if (event.type == SDL_MOUSEBUTTONUP) {
						if (event.button.button == 1 && (memoryBytes[1][53501] & 16384) == 16384)      // Left Mouse Button Up
							memoryBytes[1][53501] = 16384 ^ memoryBytes[1][53501];
						else if (event.button.button == 3 && (memoryBytes[1][53501] & 32768) == 32768) // Right Mouse Button Up
							memoryBytes[1][53501] = 32768 ^ memoryBytes[1][53501];
					}
			}

			// If there are keys in the queue, use it and decrease the life
			if (keyRollover.empty() == false) {
				memoryBytes[1][53500] = ConvertAsciiToSdcii(keyRollover.front().keyCode) | (keyRollover.front().isDown << 15);
				keyRollover[0].uses--;
				// If this key has been fully used, remove from the queue
				if (keyRollover[0].uses <= 0)
					keyRollover.pop_front();
				else {
					keyRollover.push_back(keyRollover.front());
					keyRollover.pop_front();
				}
			}
			else
				memoryBytes[1][53500] = 168;
			if (verbose && memoryBytes[1][53500] != 168) {
				PrintColored("\n	-- keypress << ", brightBlackFGColor, "");
				PrintColored(to_string(memoryBytes[1][53500]), greenFGColor, "");
				uint8_t amountPressed = 0;
				for (size_t i = 0; i < pressedKeys.size(); i++)
					if (pressedKeys[i])
						amountPressed++;
				PrintColored("	-- total keys pressed: ", brightBlackFGColor, "");
				PrintColored(to_string(amountPressed), greenFGColor, "");
			}

		}
	}

	//destroy(gRenderer, gWindow);
	SDL_Quit();
#if WINDOWS
	deinitCapture(0);
#endif

	return 0;
}

bool channelsPlaying[] = { false, false, false, false };
void Update()
{
	// If performanceMode is turned off, execute in classic mode
	if (!performanceMode){

		// Execute fetch in single step (normally this process is done in multiple clock cycles,
		// but since it is required for every instruction this emulator speeds it up a little bit.
		// This does not change a program's functionality.)
		
		// CR
		// AW
		// RM
		// IW
		InstructionReg = memoryBytes[0][programCounter];
		// CE
		programCounter += 1;

		// For all steps in the instruction, execute it's corresponding microinstruction
		for (int step = 2; step < 10; step++)
		{

			// Access the microcode of the current instruction from the rom
			int microcodeLocation = ((InstructionReg >> 6) & 0b11111100000) + (step * 4) + (flags[0] * 2) + flags[1];
			MicroInstruction mcode = microinstructionData[microcodeLocation];

			// Check for any reads and execute if applicable
			MicroInstruction readInstr = mcode & READ_MASK;
			switch (readInstr) [[likely]]
				{
			case READ_RA: // Read from A register onto bus
				bus = AReg;
				break;
			case READ_RB: // Read from B register onto bus
				bus = BReg;
				break;
			case READ_RC: // Read from C register onto bus
				bus = CReg;
				break;
			case READ_RM: // Read from memory address onto bus
				bus = (BankReg == 1 && memoryIndex >= 53547) ? videoBuffer[VideoBufReg][memoryIndex - 53547] : memoryBytes[BankReg][memoryIndex];
				break;
			case READ_IR: // Read from the instruction register onto bus
				bus = InstructionReg & 0b11111111111;
				break;
			case READ_CR: // Read from the program counter onto bus
				bus = programCounter;
				break;
				//case READ_RE:
				//	bus = expansionPort[ExpReg];
				//	break;
				}


					// Find ALU modifiers (ie. SUB, MUL DIV, etc. using the ALU Processor)
				MicroInstruction aluInstr = mcode & ALU_MASK;

				// Standalone microinstruction (ungrouped)
				if (mcode & STANDALONE_EO) [[unlikely]]
					{
						flags[0] = 0;
						flags[1] = 0;
						switch (aluInstr)
						{
						case ALU_SU: // Subtract @B from @A and put answer into @A
							flags[1] = 1;
							if (AReg - BReg == 0)
								flags[0] = 1;
							bus = AReg - BReg;
							while (bus < 0)
							{
								bus = 65535 + bus;
								flags[1] = 0;
							}
							break;

						case ALU_MU: // Multiply @A and @B and put answer into @A
							if (AReg * BReg == 0)
								flags[0] = 1;
							bus = AReg * BReg;
							while (bus > 65535)
							{
								bus = bus - 65535;
								flags[1] = 1;
							}
							break;

						case ALU_DI: // Divide @A by @B and put answer into @A
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

							while (bus > 65535)
							{
								bus = bus - 65535;
								flags[1] = 1;
							}
							break;

						case ALU_SL: // Logical bit shift left @A by @B bits and put answer into @A
							bus = (uint16_t)(AReg << (BReg & 0b1111));

							if (bus == 0)
								flags[0] = 1;

							while (bus > 65535)
							{
								bus = bus - 65535;
								flags[1] = 1;
							}
							break;

						case ALU_SR: // Logical bit shift right @A by @B bits and put answer into @A
							bus = (uint16_t)(AReg >> (BReg & 0b1111));

							if (bus == 0)
								flags[0] = 1;

							while (bus > 65535)
							{
								bus = bus - 65535;
								flags[1] = 1;
							}
							break;

						case ALU_AND: // Logical AND @A and @B and put answer into @A
							bus = AReg & BReg;

							if (bus == 0)
								flags[0] = 1;

							while (bus > 65535)
							{
								bus = bus - 65535;
								flags[1] = 1;
							}
							break;

						case ALU_OR: // Logical OR @A and @B and put answer into @A
							bus = AReg | BReg;

							if (bus == 0)
								flags[0] = 1;

							while (bus > 65535)
							{
								bus = bus - 65535;
								flags[1] = 1;
							}
							break;

						case ALU_NOT: // Logical NOT @A and @B and put answer into @A
							bus = ~AReg;

							if (bus == 0)
								flags[0] = 1;

							while (bus > 65535)
							{
								bus = bus - 65535;
								flags[1] = 1;
							}
							break;

						default: // Add @A and @B and put answer into @A
							if (AReg + BReg == 0)
								flags[0] = 1;
							bus = AReg + BReg;
							while (bus > 65535)
							{
								bus = bus - 65535;
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
					case WRITE_WA: // Write from bus into A register
						AReg = bus;
						break;
					case WRITE_WB: // Write from bus into B register
						BReg = bus;
						break;
					case WRITE_WC: // Write from bus into C register
						CReg = bus;
						break;
					case WRITE_IW: // Write from bus into Instruction register
						InstructionReg = bus;
						break;
					case WRITE_WM: // Write from bus into memory location
						// If the region of memory we are writing to is the expansion port mapped memory location for music
						// Only play audio if writing to the dedicated audio expansion port
						if (BankReg == 1 && memoryIndex == 53502) {
							if (verbose) {
								PrintColored("	-- cout >> ", brightBlackFGColor, "");
								PrintColored(to_string(BankReg) + " ", blueFGColor, "");
								PrintColored(to_string(bus) + " ", greenFGColor, "");
								cout << "\n";
							}
							////////////
							// Audio: //
							////////////

							//		Format:     XXXXX FFFFFFFFCCC
							//		You can only toggle one channel at a time per expansion port write.
							//		CCC is converted to the index of the channel that is toggled
							//		Then the frequency for that channel is defined as an int value stored in FFFFFFFF
							//              If FFFFFFFF is all Zeros, then the channel is turned off. Otherwise, the frequency
							//		is changed and the channel is turned ON if it isn't already
							//              CCC indexing starts at 1 instead of 0 to prevent accidental audio output:
							//              This means the first channel is index 1, etc.


							// Calculate target frequency from beginning 7-bits
							float offset = 0.0f;
							float targetSpeed = ConvertNoteIndexToFrequency((bus & 0b1111111000) >> 3);
							int targetChannel = bus & 0b111;

							if (targetChannel > 0 && targetChannel <= 4)
								// If the channel is not playing and the selected frequency is not 0, start playing with frequency
								if (isPlaying[targetChannel - 1] == false && targetSpeed > offset) {
									isPlaying[targetChannel - 1] = true;
									(*tone[targetChannel - 1]).stopNote(lastFreq[targetChannel - 1]);
									(*tone[targetChannel - 1]).playNote(targetSpeed);
									lastFreq[targetChannel - 1] = targetSpeed;
								}
							// If the channel is playing and the selected frequency is not 0, change frequency
								else if (isPlaying[targetChannel - 1] == true && targetSpeed > offset && lastFreq[targetChannel - 1] != targetSpeed) {
									(*tone[targetChannel - 1]).stopNote(lastFreq[targetChannel - 1]);
									(*tone[targetChannel - 1]).playNote(targetSpeed);
									lastFreq[targetChannel - 1] = targetSpeed;
								}
							// If the channel is playing and the selected frequency is 0, stop channel
								else if (isPlaying[targetChannel - 1] == true && targetSpeed == offset) {
									isPlaying[targetChannel - 1] = false;
									(*tone[targetChannel - 1]).stopNote(lastFreq[targetChannel - 1]);
									lastFreq[targetChannel - 1] = targetSpeed;
								}

						}
						// Otherwise just set the memory value to the bus
						else {
							if (BankReg == 1 && memoryIndex >= 53546) // If a video location
								videoBuffer[(int)VideoBufReg][memoryIndex - 53546] = bus;
							else // Else a normal memory location
								memoryBytes[BankReg][memoryIndex] = bus;
						}
						break;
					case WRITE_J: // Write from bus into program counter, ie a `JUMP`
						programCounter = bus;
						break;
					case WRITE_AW: // Write from bus into memory index, which changes where the next read/write from memory will be
						memoryIndex = bus;
						break;
					case WRITE_BNK: // Write from bus into bank register, which changes the current memory bank being accessed
						BankReg = bus & 0b111;
						break;
					case WRITE_VBUF: // Swap the video front and back buffer.
						VideoBufReg = !VideoBufReg;
						videoBuffer[VideoBufReg] = videoBuffer[!VideoBufReg];
						//if (imageOnlyMode) { // Draw an extra time if in imageOnlyMode
						Draw();
						//}
						break;
					}


					bus = 0;

					// Standalone microinstructions (ungrouped)
					if (mcode & STANDALONE_CE) [[unlikely]] // Counter enable microinstruction, increment program counter by 1
						programCounter = programCounter == 65535 ? 0 : programCounter + 1;

						if (mcode & STANDALONE_EI) // End instruction microinstruction, stop executing the current instruction because it is done
							break;
		}
		}
	// If in performance mode, execute instructions quickly
	else {
		// Fetch
		InstructionReg = memoryBytes[0][programCounter];
		FullInstruction inst = ((InstructionReg >> 11) & 0b11111);
		uint16_t arg = InstructionReg & 0b11111111111;
		programCounter = programCounter == 65535 ? 0 : programCounter + 1;
		int tempArithmetic = 0;

		switch (inst)
		{
		case NOP:
			break;
		case AIN:
			AReg = GetMem(BankReg, arg);
			break;
		case BIN:
			BReg = GetMem(BankReg, arg);
			break;
		case CIN:
			CReg = GetMem(BankReg, arg);
			break;
		case LDIA:
			AReg = arg;
			break;
		case LDIB:
			BReg = arg;
			if (superVerbose)
				cout << "ldib  set BReg to " << BReg << endl;
			break;
		case STA:
			SetMem(BankReg, arg, AReg);
			break;
		case ADD:
			flags[0] = 0;
			flags[1] = 0;
			if (AReg + BReg == 0)
				flags[0] = 1;
			tempArithmetic = AReg + BReg;
			while (tempArithmetic > 65535)
			{
				tempArithmetic = tempArithmetic - 65535;
				flags[1] = 1;
			}
			AReg = tempArithmetic;
			break;
		case SUB:
			flags[0] = 0;
			flags[1] = 1;
			if (AReg - BReg == 0)
				flags[0] = 1;
			tempArithmetic = AReg - BReg;
			while (tempArithmetic < 0)
			{
				tempArithmetic = 65535 - tempArithmetic;
				flags[1] = 0;
			}
			AReg = tempArithmetic;
			break;
		case MULT:
			flags[0] = 0;
			flags[1] = 0;
			if (AReg * BReg == 0)
				flags[0] = 1;
			tempArithmetic = AReg * BReg;
			while (tempArithmetic > 65535)
			{
				tempArithmetic = tempArithmetic - 65535;
				flags[1] = 1;
			}
			AReg = tempArithmetic;
			break;
		case DIV:
			flags[0] = 0;
			flags[1] = 0;
			tempArithmetic;
			// Dont divide by zero
			if (BReg != 0) {
				if (AReg / BReg == 0)
					flags[0] = 1;
				tempArithmetic = AReg / BReg;
			}
			else {
				flags[0] = 1;
				tempArithmetic = 0;
			}

			while (tempArithmetic > 65535)
			{
				tempArithmetic = tempArithmetic - 65535;
				flags[1] = 1;
			}
			AReg = tempArithmetic;
			break;
		case JMP:
			programCounter = GetMem(0, programCounter);
			if (superVerbose)
				cout << "jmp  jump to " << programCounter << endl;
			break;
		case JMPZ:
			if (flags[0] == 1)
				programCounter = GetMem(0, programCounter);
			else
				programCounter++;
			if (superVerbose)
				cout << "jmpz  jump to " << programCounter << endl;
			break;
		case JMPC:
			if (flags[1] == 1)
				programCounter = GetMem(0, programCounter);
			else
				programCounter++;
			if (superVerbose)
				cout << "jmpc  jump to " << programCounter << endl;
			break;
		case JREG:
			programCounter = AReg;
			if (superVerbose)
				cout << "jreg  jump to " << AReg << endl;
			break;
		case LDAIN:
			AReg = GetMem(BankReg, AReg);
			if (superVerbose)
				cout << "ldain  change AReg to " << GetMem(BankReg, AReg) << endl;
			break;
		case STAOUT:
			SetMem(BankReg, AReg, BReg);
			if (superVerbose)
				cout << "staout  store BReg to " << AReg << endl;
			break;
		case LDLGE:
			BankReg = arg & 0b111;
			AReg = GetMem(BankReg, GetMem(0, programCounter));
			if (superVerbose)
				cout << "ldlge  change AReg to " << GetMem(0, programCounter) << endl;
			programCounter++;
			break;
		case STLGE:
			BankReg = arg & 0b111;
			SetMem(BankReg, GetMem(0, programCounter), AReg);
			if (superVerbose)
				cout << "stlge  store AReg to " << GetMem(0, programCounter) << endl;
			programCounter++;
			break;
		case LDW:
			//AReg = memoryBytes[0][programCounter];
			BankReg = arg & 0b111;
			AReg = GetMem(BankReg, programCounter);
			programCounter++;
			if (superVerbose)
				cout << "ldw  change AReg to " << AReg << endl;
			break;
		case SWP:
			AReg = AReg ^ BReg;
			BReg = AReg ^ BReg;
			AReg = AReg ^ BReg;
			break;
		case SWPC:
			AReg = AReg ^ CReg;
			CReg = AReg ^ CReg;
			AReg = AReg ^ CReg;
			break;
		case PCR:
			AReg = programCounter - 1;
			break;
		case BSL:
			flags[0] = 0;
			flags[1] = 0;
			tempArithmetic = AReg << (BReg & 0b1111);

			if (tempArithmetic == 0)
				flags[0] = 1;

			while (tempArithmetic > 65535)
			{
				tempArithmetic = tempArithmetic - 65535;
				flags[1] = 1;
			}
			AReg = tempArithmetic;
			break;
		case BSR:
			flags[0] = 0;
			flags[1] = 0;
			tempArithmetic = AReg >> (BReg & 0b1111);

			if (tempArithmetic == 0)
				flags[0] = 1;

			while (tempArithmetic > 65535)
			{
				tempArithmetic = tempArithmetic - 65535;
				flags[1] = 1;
			}
			AReg = tempArithmetic;
			break;
		case AND:
			flags[0] = 0;
			flags[1] = 0;
			tempArithmetic = AReg & BReg;

			if (tempArithmetic == 0)
				flags[0] = 1;

			while (tempArithmetic > 65535)
			{
				tempArithmetic = tempArithmetic - 65535;
				flags[1] = 1;
			}
			AReg = tempArithmetic;
			break;
		case OR:
			flags[0] = 0;
			flags[1] = 0;
			tempArithmetic = AReg | BReg;

			if (tempArithmetic == 0)
				flags[0] = 1;

			while (tempArithmetic > 65535)
			{
				tempArithmetic = tempArithmetic - 65535;
				flags[1] = 1;
			}
			AReg = tempArithmetic;
			break;
		case NOT:
			flags[0] = 0;
			flags[1] = 0;
			tempArithmetic = ~AReg;

			if (tempArithmetic == 0)
				flags[0] = 1;

			while (tempArithmetic > 65535)
			{
				tempArithmetic = tempArithmetic - 65535;
				flags[1] = 1;
			}
			AReg = tempArithmetic;
			break;
		case BNK:
			BankReg = arg & 0b111;
			if (superVerbose)
				cout << "bnk  bank change to " << arg << endl;
			break;
		case VBUF:
			VideoBufReg = !VideoBufReg;
			videoBuffer[VideoBufReg] = videoBuffer[!VideoBufReg];
			Draw();
			if (superVerbose)
				cout << "vbuf" << endl;
			break;
		case BNKC:
			BankReg = CReg & 0b111;
			if (superVerbose)
				cout << "bnkc  bank change to " << CReg << endl;
			break;
		case LDWB:
			BankReg = arg & 0b111;
			BReg = GetMem(BankReg, programCounter);
			programCounter++;
			if (superVerbose)
				cout << "ldwb  change BReg to " << BReg << endl;
			break;
		}
	}

}


void SetMem(uint16_t bank, uint16_t address, uint16_t data) {
	// If the region of memory we are writing to is the expansion port mapped memory location for music
	// Only play audio if writing to the dedicated audio expansion port
	if (bank == 1 && address == 53502) {
		if (verbose) {
			PrintColored("	-- cout >> ", brightBlackFGColor, "");
			PrintColored(to_string(bank) + " ", blueFGColor, "");
			PrintColored(to_string(data) + " ", greenFGColor, "");
			cout << "\n";
		}
		////////////
		// Audio: //
		////////////

		//		Format:     XXXXX FFFFFFFFCCC
		//		You can only toggle one channel at a time per expansion port write.
		//		CCC is converted to the index of the channel that is toggled
		//		Then the frequency for that channel is defined as an int value stored in FFFFFFFF
		//              If FFFFFFFF is all Zeros, then the channel is turned off. Otherwise, the frequency
		//		is changed and the channel is turned ON if it isn't already
		//              CCC indexing starts at 1 instead of 0 to prevent accidental audio output:
		//              This means the first channel is index 1, etc.


		// Calculate target frequency from beginning 7-bits
		float offset = 0.0f;
		float targetSpeed = ConvertNoteIndexToFrequency((data & 0b1111111000) >> 3);
		int targetChannel = data & 0b111;

		if (targetChannel > 0 && targetChannel <= 4)
			// If the channel is not playing and the selected frequency is not 0, start playing with frequency
			if (isPlaying[targetChannel - 1] == false && targetSpeed > offset) {
				isPlaying[targetChannel - 1] = true;
				(*tone[targetChannel - 1]).stopNote(lastFreq[targetChannel - 1]);
				(*tone[targetChannel - 1]).playNote(targetSpeed);
				lastFreq[targetChannel - 1] = targetSpeed;
			}
		// If the channel is playing and the selected frequency is not 0, change frequency
			else if (isPlaying[targetChannel - 1] == true && targetSpeed > offset && lastFreq[targetChannel - 1] != targetSpeed) {
				(*tone[targetChannel - 1]).stopNote(lastFreq[targetChannel - 1]);
				(*tone[targetChannel - 1]).playNote(targetSpeed);
				lastFreq[targetChannel - 1] = targetSpeed;
			}
		// If the channel is playing and the selected frequency is 0, stop channel
			else if (isPlaying[targetChannel - 1] == true && targetSpeed == offset) {
				isPlaying[targetChannel - 1] = false;
				(*tone[targetChannel - 1]).stopNote(lastFreq[targetChannel - 1]);
				lastFreq[targetChannel - 1] = targetSpeed;
			}

	}
	// Otherwise just set the memory value to the bus
	else {
		if (bank == 1 && address >= 53546) // If a video location
			videoBuffer[(int)VideoBufReg][address - 53546] = data;
		else // Else a normal memory location
			memoryBytes[bank][address] = data;
	}
}

uint16_t GetMem(uint16_t bank, uint16_t address) {
	return ((bank == 1 && address >= 53547) ? videoBuffer[VideoBufReg][address - 53547] : memoryBytes[bank][address]);
}

void DrawNextPixel() {
	int charVal = videoBuffer[(int)(!VideoBufReg)][characterRamIndex];
	int characterRamValue = charVal & 0b11111111;
	int colorValue = (charVal >> 8) & 0b11111111;
	bool charPixRomVal = characterRom[(characterRamValue * 64) + (charPixY * 8) + charPixX];

	int pixelVal = videoBuffer[(int)(!VideoBufReg)][pixelRamIndex + 324];
	int r, g, b;

	if (charPixRomVal == true) {
		// If the color is set to 0, then make it white, or vice versa.
		// This is for compatibility with previous program versions, which default the color to 0.
		if (colorValue == 0)
			colorValue = 0b11111111;
		else if (colorValue == 0b11111111)
			colorValue = 0;

		r = BitRange(colorValue, 5, 3) * 36 + 0b11;
		g = BitRange(colorValue, 2, 3) * 36 + 0b11; // 0b00011111
		b = BitRange(colorValue, 0, 2) * 85;
	}
	else {
		if (pixelVal == 65535) {
			r = 255;
			g = 255;
			b = 255;
		}
		else {
			r = (BitRange(pixelVal, 10, 5)) * 8; // Get first 5 bits
			g = (BitRange(pixelVal, 5, 5)) * 8; // get middle bits
			b = (BitRange(pixelVal, 0, 5)) * 8; // Gets last 5 bits
		}
	}

	set_pixel(&pixels, imgX, imgY, 108, r, g, b, 255);


	imgX++;
	charPixX++;
	if (charPixX >= 6) {
		charPixX = 0;
		characterRamIndex++;
	}

	// If x-coord is max, reset and increment y-coord
	if (imgX >= 108)
	{
		imgY++;
		charPixY++;
		charPixX = 0;
		imgX = 0;

		if (charPixY < 6)
			characterRamIndex -= 18;
	}

	if (charPixY >= 6) {
		charPixY = 0;
	}


	if (imgY >= 108) // The final layer is done, reset counter and render image
	{
		imgY = 0;

		characterRamIndex = 0;
		charPixY = 0;
		charPixX = 0;

		apply_pixels(pixels, texture, 108);
		DisplayTexture(gRenderer, texture);
	}

	pixelRamIndex++;
}

// Draw an entire screen's worth of pixels
void Draw() {
	while (true) {
		DrawNextPixel();
		if (pixelRamIndex >= 108 * 108) {
			pixelRamIndex = 0;
			break;
		}
	}
	if (imageOnlyMode) {
		if (imageOnlyModeFrames - imageOnlyModeFrameCount == 0) {
			imageOnlyModeFrameCount -= 1;
			return;
		}

		auto padded = std::to_string(imageOnlyModeFrames - imageOnlyModeFrameCount);
		padded.insert(0, 5U - std::min<int>(std::string::size_type(5), padded.length()), '0');

		//uint8_t padd_amount = std::to_string(imageOnlyModeFrames).length();
		//std::string unpadded = std::to_string(imageOnlyModeFrames - imageOnlyModeFrameCount);
		//std::string paddedFrameNum = std::string(padd_amount - std::min<int>(padd_amount, unpadded.length()), '0') + unpadded;
		Save_Frame(projectDirectory + "./frames/frame_" + padded, pixels);
		imageOnlyModeFrameCount -= 1;
		if (imageOnlyModeFrameCount <= 0)
			exit(1);
	}
}

void DrawPixel(int x, int y, int r, int g, int b)
{
	SDL_SetRenderDrawColor(gRenderer, r, g, b, 255);
	SDL_RenderDrawPoint(gRenderer, x, y);
}

void Save_Frame(const ::std::string& name, vector<unsigned char> img_vals)
{
	constexpr auto dimx = 108u, dimy = 108u;

	using namespace std;
	ofstream ofs(name + ".ppm", ios_base::out | ios_base::binary);
	ofs << "P6" << endl << dimx << ' ' << dimy << endl << "255" << endl;


	for (auto y = 0u; y < dimy; ++y)
		for (auto x = 0u; x < dimx; ++x) {
			const unsigned int offset = (y * 4 * dimx) + x * 4;
			ofs << img_vals[offset + 0] << img_vals[offset + 1] << img_vals[offset + 2];       // red, green, blue
		}

	ofs.close();

	//PrintColored("saved file to \"" + name + ".ppm\"\n", "", "");
}

// Neatly convert a large float number of Hz to a string with label
std::string SimplifiedHertz(float input) {
	if (input == INFINITY)
		input = FLT_MAX;

	if (input >= 1000000000.0) // GHz
		return to_string(floor(input / 100000000.0f) / 10.0f) + " GHz";
	if (input >= 1000000.0) // MHz
		return to_string(floor(input / 100000.0f) / 10.0f) + " MHz";
	if (input >= 1000.0) // KHz
		return to_string(floor(input / 100.0f) / 10.0f) + " KHz";

	return to_string(round(input * 10.0f) / 10.0f) + " Hz";
}

int InitGraphics(const std::string& windowTitle, int width, int height, int pixelScale)
{
	int WINDOW_WIDTH = width;
	int WINDOW_HEIGHT = height;
	int PIXEL_SCALE = pixelScale;

	// Initialize SDL components
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

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

// Split a string <str> by a char <ch> into a vector of strings vector<std::string>
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
vector<vector<std::string>> parseCode(const std::string& input)
{
	vector<vector<std::string>> outputBytes;
	for (int b = 0; b < 4; b++) {
		outputBytes.push_back(vector<std::string>());
		for (int i = 0; i < 65535; i++)
			outputBytes[b].push_back("0000");
	}

	std::string icopy = input;
	transform(icopy.begin(), icopy.end(), icopy.begin(), ::toupper);
	vector<std::string> splitcode = explode(icopy, '\n');

	std::map<std::string, int> variableMap;

#if DEV_MODE
	cout << endl;
#endif

	int memaddr = 0;
	for (int i = 0; i < splitcode.size(); i++)
	{
		splitcode[i] = trim(split(splitcode[i], ",")[0]);
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
		if (splitBySpace[0] == "SET" && splitBySpace.size() == 3)
		{
			int addr;
			int argValue;
			try {
				addr = stoi(splitBySpace[1]);
			}
			catch (exception) { // If the argument is not an integer, it is a variable
				addr = variableMap[splitBySpace[1]];
			}
			try {
				argValue = stoi(splitBySpace[2]);
			}
			catch (exception) { // If the argument is not an integer, it is a variable
				argValue = variableMap[splitBySpace[2]];
			}
			std::string hVal = DecToHexFilled(argValue, 4);
			outputBytes[0][addr] = hVal;
#if DEV_MODE
			cout << ("-\t" + splitcode[i] + "\t  ~   ~\n");
#endif
			continue;
		}

		// Sets the specified memory location to a value with bank:  set <addr> <val> <bank>
		else if (splitBySpace[0] == "SET")
		{
			int addr;
			int argValue;
			try {
				addr = stoi(splitBySpace[1]);
			}
			catch (exception) { // If the argument is not an integer, it is a variable
				addr = variableMap[splitBySpace[1]];
			}
			try {
				argValue = stoi(splitBySpace[2]);
			}
			catch (exception) { // If the argument is not an integer, it is a variable
				argValue = variableMap[splitBySpace[2]];
			}
			std::string hVal = DecToHexFilled(argValue, 4);
			outputBytes[stoi(splitBySpace[3])][addr] = hVal;
#if DEV_MODE
			cout << ("-\t" + splitcode[i] + "\t  ~   ~\n");
#endif
			continue;
		}

		// Set the current location in memory equal to a value: here <value>
		if (splitBySpace[0] == "HERE")
		{
			int addr = memaddr;
			int argValue;
			try {
				argValue = stoi(splitBySpace[1]);
			}
			catch (exception) { // If the argument is not an integer, it is a variable
				argValue = variableMap[splitBySpace[1]];
			}
			std::string hVal = DecToHexFilled(argValue, 4);
			outputBytes[0][addr] = hVal;
#if DEV_MODE
			cout << ("-\t" + splitcode[i] + "\t  ~   ~\n");
#endif
			memaddr += 1;
			continue;
		}

		// Name a constant variable: const @var <value>
		if (splitBySpace[0] == "CONST")
		{
			variableMap[splitBySpace[1]] = stoi(splitBySpace[2]);
#if DEV_MODE
			cout << ("-\t" + splitcode[i] + "\t  ~   ~\n");
#endif
			//memaddr += 1;
			continue;
		}

		// Allocate the current location in memory for a given range: alloc <value>
		// ex:   `alloc 2`
		// is:   here 0
		//       here 0
		if (splitBySpace[0] == "ALLOC")
		{
			int addr = memaddr;
			int argValue;
			try {
				argValue = stoi(splitBySpace[1]);
			}
			catch (exception) { // If the argument is not an integer, it is a variable
				argValue = variableMap[splitBySpace[1]];
			}
			/*std::string hVal = DecToHexFilled(argValue, 4);
			outputBytes[0][addr] = hVal;*/
#if DEV_MODE
			cout << ("-\t" + splitcode[i] + "\t  ~   ~\n");
#endif
			memaddr += argValue;
			continue;
		}

		// Memory address is already used, skip.
		if (outputBytes[0][memaddr] != "0000") {
			memaddr += 1;
		}

#if DEV_MODE
		cout << (to_string(memaddr) + " " + splitcode[i] + "   \t  =>  ");
#endif

		// Find index of instruction
		bool notFound = false;
		for (int f = 0; f < instructions.size(); f++)
		{
			if (instructions[f] == splitBySpace[0])
			{
#if DEV_MODE
				cout << DecToBinFilled(f, 5);
#endif
				outputBytes[0][memaddr] = DecToBinFilled(f, 5);
				break;
			}
			if(f == instructions.size()-1) // if the instruction was not found
			{
				// Create a label: <labelname>:
				variableMap[split(splitBySpace[0], ":")[0]] = memaddr;
				notFound = true;
				//memaddr++;
			}
		}
		if (notFound)
			continue;

		// Check if any args are after the command
		if (splitcode[i] != splitBySpace[0])
		{
			int argValue;
			try{
				argValue = stoi(splitBySpace[1]);
			}
			catch(exception){ // If the argument is not an integer, it is a variable
				argValue = variableMap[splitBySpace[1]];
			}
#if DEV_MODE
			cout << DecToBinFilled(argValue, 11);
#endif
			outputBytes[0][memaddr] += DecToBinFilled(argValue, 11);
		}
		else
		{
#if DEV_MODE
			cout << " 00000000000";
#endif
			outputBytes[0][memaddr] += "00000000000";
		}
#if DEV_MODE
		cout << "  " + BinToHexFilled(outputBytes[0][memaddr], 4) + "\n";
#endif
		outputBytes[0][memaddr] = BinToHexFilled(outputBytes[0][memaddr], 4); // Convert from binary to hex
		memaddr += 1;
	}


	// Save the output
	std::string processedOutput = "";
	processedOutput += "\nv3.0 hex words addressed\n";
	processedOutput += "000: ";
	for (int outindex = 0; outindex < outputBytes[0].size(); outindex++)
	{
		if (outindex % 8 == 0 && outindex != 0)
		{
			std::string locationTmp = DecToHexFilled(outindex, 3);
			transform(locationTmp.begin(), locationTmp.end(), locationTmp.begin(), ::toupper);
			processedOutput += "\n" + DecToHexFilled(outindex, 3) + ": ";
		}
		processedOutput += outputBytes[0][outindex] + " ";

		std::string ttmp = outputBytes[0][outindex];
		transform(ttmp.begin(), ttmp.end(), ttmp.begin(), ::toupper);
	}
#if DEV_MODE
	cout << processedOutput << "\n\n";
#endif

	// Save the data to logisim_pmc.hex
	fstream myStream;
	myStream.open(projectDirectory + "logisim_pmc.hex", ios::out);
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
	vector<std::string> output(2048, "00000");

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
		cout << (newStr) << " .\n";
#endif
		instructioncodes[cl] = newStr;
		instructioncodes[cl] = explode(instructioncodes[cl], '(')[1];
	}

	// Special process fetch instruction
#if DEV_MODE
	cout << "\n\ngenerate fetch... \n";
#endif
	for (int ins = 0; ins < sizeof(instructioncodes) / sizeof(instructioncodes[0]); ins++) // Iterate through all definitions of instructions
	{
		std::string startaddress = DecToBinFilled(ins, 6);

		vector<std::string> instSteps = explode(instructioncodes[0], '&');
		for (int step = 0; step < instSteps.size(); step++) // Iterate through every step
		{
			int actualStep = stoi(explode(instSteps[step], '=')[0]);
			std::string stepContents = explode(explode(instSteps[step], '=')[1], '|')[0];

			std::string midaddress = DecToBinFilled(actualStep, 3);

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
#if DEV_MODE
		cout << (instructioncodes[ins] + "\n");
#endif

		std::string startaddress = DecToBinFilled(ins, 6);

		vector<std::string> instSteps = explode(instructioncodes[ins], '&');
		for (int step = 0; step < instSteps.size(); step++) // Iterate through every step
		{
			int actualStep = stoi(explode(instSteps[step], '=')[0]);
			std::string stepContents = explode(explode(instSteps[step], '=')[1], '|')[0];

			std::string midaddress = DecToBinFilled(actualStep, 3);

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


	// Save the data to logisim_mic.hex
	fstream myStream;
	myStream.open(projectDirectory + "logisim_mic.hex", ios::out);
	myStream << processedOutput;
}

vector<string> vars;
vector<string> labels;
vector<int> labelLineValues;
vector<string> compiledLines;

uint16_t ConvertNoteIndexToFrequency(uint8_t index) {
	uint16_t conversionTable[96];
	conversionTable[0] = 0;
	conversionTable[1] = 16;
	conversionTable[2] = 17;
	conversionTable[3] = 18;
	conversionTable[4] = 19;
	conversionTable[5] = 20;
	conversionTable[6] = 21;
	conversionTable[7] = 23;
	conversionTable[8] = 24;
	conversionTable[9] = 26;
	conversionTable[10] = 27;
	conversionTable[11] = 29;
	conversionTable[12] = 30;
	conversionTable[13] = 32;
	conversionTable[14] = 34;
	conversionTable[15] = 36;
	conversionTable[16] = 38;
	conversionTable[17] = 41;
	conversionTable[18] = 43;
	conversionTable[19] = 46;
	conversionTable[20] = 49;
	conversionTable[21] = 52;
	conversionTable[22] = 55;
	conversionTable[23] = 58;
	conversionTable[24] = 61;
	conversionTable[25] = 65;
	conversionTable[26] = 69;
	conversionTable[27] = 73;
	conversionTable[28] = 77;
	conversionTable[29] = 82;
	conversionTable[30] = 87;
	conversionTable[31] = 92;
	conversionTable[32] = 98;
	conversionTable[33] = 104;
	conversionTable[34] = 110;
	conversionTable[35] = 116;
	conversionTable[36] = 123;
	conversionTable[37] = 130;
	conversionTable[38] = 138;
	conversionTable[39] = 146;
	conversionTable[40] = 155;
	conversionTable[41] = 164;
	conversionTable[42] = 174;
	conversionTable[43] = 185;
	conversionTable[44] = 196;
	conversionTable[45] = 207;
	conversionTable[46] = 220;
	conversionTable[47] = 233;
	conversionTable[48] = 246;
	conversionTable[49] = 261;
	conversionTable[50] = 277;
	conversionTable[51] = 293;
	conversionTable[52] = 311;
	conversionTable[53] = 329;
	conversionTable[54] = 349;
	conversionTable[55] = 370;
	conversionTable[56] = 392;
	conversionTable[57] = 415;
	conversionTable[58] = 440;
	conversionTable[59] = 466;
	conversionTable[60] = 493;
	conversionTable[61] = 523;
	conversionTable[62] = 554;
	conversionTable[63] = 587;
	conversionTable[64] = 622;
	conversionTable[65] = 659;
	conversionTable[66] = 698;
	conversionTable[67] = 740;
	conversionTable[68] = 783;
	conversionTable[69] = 830;
	conversionTable[70] = 880;
	conversionTable[71] = 932;
	conversionTable[72] = 987;
	conversionTable[73] = 1046;
	conversionTable[74] = 1108;
	conversionTable[75] = 1174;
	conversionTable[76] = 1244;
	conversionTable[77] = 1318;
	conversionTable[78] = 1396;
	conversionTable[79] = 1480;
	conversionTable[80] = 1567;
	conversionTable[81] = 1661;
	conversionTable[82] = 1760;
	conversionTable[83] = 1864;
	conversionTable[84] = 1975;

	return conversionTable[index];
}

// This will convert ASCII/SDL2 key codes to their SDCII alternatives
int ConvertAsciiToSdcii(int asciiCode) {

	if (superVerbose)
		cout << "Ascii code for that key is: " << to_string(asciiCode) << endl;

	int actualVal = asciiToSdcii[asciiCode];
	if (actualVal == -1) // -1 Means unspecified value
		actualVal = 168;

	return actualVal;
}

// This will convert SDCII to ASCII alternatives
int ConvertSdciiToAscii(int sdciiCode) {

	if (superVerbose)
		cout << "Sdcii code for that key is: " << to_string(sdciiCode) << endl;

	int actualVal = sdciiToAscii[sdciiCode];
	if (actualVal == -1) // -1 Means unspecified value
		actualVal = 168;

	return actualVal;
}


void GenerateAsciiSdciiTables() {
	for (size_t i = 0; i < sizeof(asciiToSdcii) / sizeof(asciiToSdcii[0]); i++) {
		asciiToSdcii[i] = -1;
		sdciiToAscii[i] = -1;
	}

	// Special characters
	asciiToSdcii[40] = 85;	// enter -> enter
	asciiToSdcii[44] = 0;	// space -> blank
	asciiToSdcii[58] = 1;	// f1 -> smaller solid square
	asciiToSdcii[59] = 2;	// f2 -> full solid square
	asciiToSdcii[87] = 3;	// num+ -> +
	asciiToSdcii[86] = 4;	// num- -> -
	asciiToSdcii[85] = 5;	// num* -> *
	asciiToSdcii[84] = 6;	// num/ -> /
	asciiToSdcii[60] = 7;	// f3 -> full hollow square
	asciiToSdcii[45] = 8;	// _ -> _
	asciiToSdcii[80] = 9;	// l-arr -> <
	asciiToSdcii[79] = 10;	// r-arr -> >
	asciiToSdcii[82] = 71;	// u-arr -> u-arr
	asciiToSdcii[81] = 72;	// d-arr -> d-arr
	asciiToSdcii[49] = 11;	// | -> vertical line |
	asciiToSdcii[66] = 12;	// f9 -> horizontal line --
	asciiToSdcii[55] = 54;	// , -> ,
	asciiToSdcii[54] = 55;	// . -> .

	// Letters
	asciiToSdcii[4] = 13;	// a -> a
	asciiToSdcii[5] = 14;	// b -> b
	asciiToSdcii[6] = 15;	// c -> c
	asciiToSdcii[7] = 16;	// d -> d
	asciiToSdcii[8] = 17;	// e -> e
	asciiToSdcii[9] = 18;	// f -> f
	asciiToSdcii[10] = 19;	// g -> g
	asciiToSdcii[11] = 20;	// h -> h
	asciiToSdcii[12] = 21;	// i -> i
	asciiToSdcii[13] = 22;	// j -> j
	asciiToSdcii[14] = 23;	// k -> k
	asciiToSdcii[15] = 24;	// l -> l
	asciiToSdcii[16] = 25;	// m -> m
	asciiToSdcii[17] = 26;	// n -> n
	asciiToSdcii[18] = 27;	// o -> o
	asciiToSdcii[19] = 28;	// p -> p
	asciiToSdcii[20] = 29;	// q -> q
	asciiToSdcii[21] = 30;	// r -> r
	asciiToSdcii[22] = 31;	// s -> s
	asciiToSdcii[23] = 32;	// t -> t
	asciiToSdcii[24] = 33;	// u -> u
	asciiToSdcii[25] = 34;	// v -> v
	asciiToSdcii[26] = 35;	// w -> w
	asciiToSdcii[27] = 36;	// x -> x
	asciiToSdcii[28] = 37;	// y -> y
	asciiToSdcii[29] = 38;	// z -> z

	// Numbers
	asciiToSdcii[39] = 39;	// 0 -> 0
	asciiToSdcii[30] = 40;	// 1 -> 1
	asciiToSdcii[31] = 41;	// 2 -> 2
	asciiToSdcii[32] = 42;	// 3 -> 3
	asciiToSdcii[33] = 43;	// 4 -> 4
	asciiToSdcii[34] = 44;	// 5 -> 5
	asciiToSdcii[35] = 45;	// 6 -> 6
	asciiToSdcii[36] = 46;	// 7 -> 7
	asciiToSdcii[37] = 47;	// 8 -> 8
	asciiToSdcii[38] = 48;	// 9 -> 9


	asciiToSdcii[42] = 70;	// backspace -> backspace





	// Letters
	sdciiToAscii[13] = 97;	    // a -> a
	sdciiToAscii[14] = 98;   	// b -> b
	sdciiToAscii[15] = 99;  	// c -> c
	sdciiToAscii[16] = 100;	// d -> d
	sdciiToAscii[17] = 101;	// e -> e
	sdciiToAscii[18] = 102;	// f -> f
	sdciiToAscii[19] = 103;	// g -> g
	sdciiToAscii[20] = 104;	// h -> h
	sdciiToAscii[21] = 105;	// i -> i
	sdciiToAscii[22] = 106;	// j -> j
	sdciiToAscii[23] = 107;	// k -> k
	sdciiToAscii[24] = 108;	// l -> l
	sdciiToAscii[25] = 109;	// m -> m
	sdciiToAscii[26] = 110;	// n -> n
	sdciiToAscii[27] = 111;	// o -> o
	sdciiToAscii[28] = 112;	// p -> p
	sdciiToAscii[29] = 113;	// q -> q
	sdciiToAscii[30] = 114;	// r -> r
	sdciiToAscii[31] = 115;	// s -> s
	sdciiToAscii[32] = 116;	// t -> t
	sdciiToAscii[33] = 117;	// u -> u
	sdciiToAscii[34] = 118;	// v -> v
	sdciiToAscii[35] = 119;	// w -> w
	sdciiToAscii[36] = 120;	// x -> x
	sdciiToAscii[37] = 121;	// y -> y
	sdciiToAscii[38] = 122;	// z -> z


	sdciiToAscii[0] = 32;	// space -> space


	sdciiToAscii[85] = 10;	// newline -> newline

	// Numbers
	sdciiToAscii[39] = 48;	// 0 -> 0 
	sdciiToAscii[40] = 49;	// 1 -> 1 
	sdciiToAscii[41] = 50;	// 2 -> 2 
	sdciiToAscii[42] = 51;	// 3 -> 3 
	sdciiToAscii[43] = 52;	// 4 -> 4 
	sdciiToAscii[44] = 53;	// 5 -> 5 
	sdciiToAscii[45] = 54;	// 6 -> 6 
	sdciiToAscii[46] = 55;	// 7 -> 7 
	sdciiToAscii[47] = 56;	// 8 -> 8 
	sdciiToAscii[48] = 57;	// 9 -> 9 

	sdciiToAscii[54] = 46;	// . -> . 

	for (size_t i = 0; i < sizeof(asciiToSdcii) / sizeof(asciiToSdcii[0]); i++)
		if(sdciiToAscii[i] != -1)
			ascToSdcii[sdciiToAscii[i]] = i;
}

// Convert Decimal int to Hexadecimal string, padded with <desiredSize>
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
	if (!IsHex(in) && !IsReg(in) && !IsVar(in) && !IsLabel(in) && !IsPointer(in))
		return true;
	return false;
}

void PutSetOnCurrentLine(const string& value) {
	compiledLines.push_back("here " + value);
}

// Loading of memory value into register, automatically allowing large addressing as needed
void LoadAddress(const string& reg, const string& address) {
	int actualVal = ParseValue(address);
	int actualLineNum = GetLineNumber();


	// Value is small enough to be accessible through normal r/w instructions
	if (actualVal <= 1023) {
		string addrInWord = "ain ";
		if (reg == "@A")
			addrInWord = "ain ";
		else if (reg == "@B")
			addrInWord = "bin ";
		else if (reg == "@C")
			addrInWord = "cin ";
		//else if (split(reg, "[")[0] == "@EX")
		//	addrInWord = "ain ";

		compiledLines.push_back(addrInWord + to_string(actualVal));
		//if (split(reg, "[")[0] == "@EX")
		//	compiledLines.push_back("wrexp " + split(split(reg, "[")[1], "]")[0]);
	}
	// Value is too large to be accessible through normal r/w instructions, use LGE style
	else if (actualVal > 1023) {
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
	if (actualVal <= 1023)
		compiledLines.push_back(MoveFromRegToReg(reg, "@A") + "sta " + to_string(actualVal));
	// Value is too large to be accessible through normal r/w instructions, use LGE style
	else if (actualVal > 1023) {
		compiledLines.push_back(MoveFromRegToReg(reg, "@A"));
		compiledLines.push_back("stlge");
		PutSetOnCurrentLine(to_string(actualVal));
	}
}

// Function to determine the instructions needed to load an immediate value into any register
void RegIdToLDI(const string& in, const string& followingValue) {
	int actualValue = ParseValue(followingValue);

	if (actualValue < 1023) {
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
		//else if (split(in, "[")[0] == "@EX") {
		//	compiledLines.push_back("ldia " + to_string(actualValue));
		//	compiledLines.push_back("wrexp " + split(split(in, "[")[1], "]")[0]);
		//}
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
		//else if (split(in, "[")[0] == "@EX") {
		//	compiledLines.push_back("ldw");
		//	PutSetOnCurrentLine(followingValue);
		//	compiledLines.push_back("wrexp " + split(split(in, "[")[1], "]")[0]);
		//}
	}
}

string MoveFromRegToReg(const string& from, const string& destination) {
	if (from == destination)
		return "";

	if (destination == "@A" && from == "@B")
		return "swp\n";
	if (destination == "@A" && from == "@C")
		return "swpc\n";
	//if (destination == "@A" && split(from, "[")[0] == "@EX")
	//	return "rdexp " + split(split(from, "[")[1], "]")[0] + "\n";

	if (destination == "@B" && from == "@A")
		return "swp\n";
	if (destination == "@B" && from == "@C")
		return "swpc\nswp\n";
	//if (destination == "@B" && split(from, "[")[0] == "@EX")
	//	return "rdexp " + split(split(from, "[")[1], "]")[0] + "\nswp\n";

	if (destination == "@C" && from == "@A")
		return "swpc\n";
	if (destination == "@C" && from == "@B")
		return "swp\nswpc\n";
	//if (destination == "@C" && split(from, "[")[0] == "@EX")
	//	return "rdexp " + split(split(from, "[")[1], "]")[0] + "\nswpc\n";

	//if (split(destination, "[")[0] == "@EX" && from == "@A")
	//	return "wrexp " + split(split(destination, "[")[1], "]")[0] + "\n";
	//if (split(destination, "[")[0] == "@EX" && from == "@B")
	//	return "swp\nwrexp " + split(split(destination, "[")[1], "]")[0] + "\n";
	//if (split(destination, "[")[0] == "@EX" && from == "@C")
	//	return "swpc\nwrexp " + split(split(destination, "[")[1], "]")[0] + "\n";

	return "";
}

// Get the actual line number in compiled assembly, this takes `SET` commands into account,
// where they set a location other than their own and is therefore not a real command when assembled.
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

// Find the memory location of an Armstrong variable, and if it doesn't exist allocate memory
int GetVariableAddress(const string& id) {
	// Search all variable names to get index
	for (int i = 0; i < vars.size(); i++)
		if (id == vars[i])
			return i + 16382;

	// Not found, add to list and return size-1
	vars.push_back(id);
	return 16382 + vars.size() - 1;
}

// Find the line number that a label refers to 
int FindLabelLine(const string& labelName, const vector<string>& labels, const vector<int>& labelLineValues) {
	for (int i = 0; i < labels.size(); i++)
		if (labelName == labels[i])
			return labelLineValues[i];

	// Not found return -1
	return -1;
}

// Parse a value into it's final form, whether it is a constant, memory address, variable, label, or pointer
int ParseValue(const string& input) {
	if (input.size() > 2) {
		if (IsHex(input))      // If preceded by '0x', then it is a hex number
			return HexToDec(split(input, "0x")[1]);
		else if (IsBin(input)) // If preceded by '0b', then it is a binary number
			return BinToDec(split(input, "0b")[1]);
		else if (input[0] == '\'') { // If preceded by ', then it is a char
			if (ConvertAsciiToSdcii((int)toupper(input[1]) - 61) != 168)
				return ConvertAsciiToSdcii((int)toupper(input[1]) - 61);
			else
				return (int)toupper(input[1]) - 61;
		}
	}
	if (IsVar(input)) // If a variable
		return GetVariableAddress(input);
	if (IsDec(input)) // If a decimal number
		return stoi(input);
	if (IsLabel(input)) // If a label
		return FindLabelLine(input, labels, labelLineValues);
	if (IsPointer(input)) { // If a pointer
		std::string pval = input.substr(1);
		return ParseValue(pval.substr(pval.find_last_of("]") + 1, pval.size()));
	}

	return -1;
}

// Reads from mem at the address stored in pointer, into REG A
void LoadPointer(const string& str) {
	LoadAddress("@A", split(str, "]")[1]);
	compiledLines.push_back("bnk " + split(split(str, "[")[1], "]")[0]);
	compiledLines.push_back("ldain");
	compiledLines.push_back("bnk 0");
}

// Writes from REG B to mem at the address stored in pointer
void StoreIntoPointer(const string& str) {
	LoadAddress("@A", split(str, "]")[1]);
	compiledLines.push_back("bnk " + split(split(str, "[")[1], "]")[0]);
	compiledLines.push_back("staout");
	compiledLines.push_back("bnk 0");
}

// Generate assembly for logically comparing any two values, memory locations, or registers
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

	cout << "   -  Preprocessing Armstrong...";

	vector<string> codelines = PreProcess(inputcode);

	// Remove comments from end of lines and trim whitespace
	for (int i = 0; i < codelines.size(); i++) {
		codelines[i] = trim(split(codelines[i], "//")[0]);
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

	cout << "   -  Compiling Armstrong...\n";
	for (int i = 0; i < codelines.size(); i++)
	{
		string command = trim(split(codelines[i], " ")[0]);

		//if (verbose)
		//	PrintColored("Compiling: " + to_string((int)((float)i / (float)codelines.size() * 100)) + "%\n", "", "");


		// "#" label marker ex. #JumpToHere
		if (command[0] == '#')
		{
			int labelLineVal = GetLineNumber();
			labels.push_back(command);
			labelLineValues.push_back(labelLineVal);
			if (verbose) {
				PrintColored("ok.	", greenFGColor, "");
				cout << "label:      ";
				PrintColored("'" + command + "'", brightBlueFGColor, "");
				PrintColored(" line: ", brightBlackFGColor, "");
				PrintColored("'" + to_string(labelLineValues.at(labelLineValues.size() - 1)) + "'\n", brightBlueFGColor, "");
			}

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
			string addrPre = split(trim(split(split(codelines[i], "define ")[1], "=")[0]), " ")[0];
			string valuePre = split(trim(split(split(codelines[i], "define ")[1], "=")[1]), " ")[0];
			if (verbose) {
				PrintColored("ok.	", greenFGColor, "");
				cout << "define:     ";
				PrintColored("'" + addrPre + "'", brightBlueFGColor, "");
				PrintColored(" as ", brightBlackFGColor, "");
				PrintColored("'" + valuePre + "'\n", brightBlueFGColor, "");
			}

			int addr = ParseValue(addrPre);
			int value = ParseValue(valuePre);

			compiledLines.push_back(",\n, " + string("define:  '") + addrPre + "' as '" + valuePre + "'");
			compiledLines.push_back("set " + to_string(addr) + " " + to_string(value));
			continue;
		}

		// "change" command (change <location> = <value or location>)
		else if (command == "change")
		{
			string addrPre = trim(split(split(codelines[i], "change ")[1], "=")[0]);
			string valuePre = trim(split(split(codelines[i], "change ")[1], "=")[1]);
			if (verbose) {
				PrintColored("ok.	", greenFGColor, "");
				cout << "change:     ";
				PrintColored("'" + addrPre + "'", brightBlueFGColor, "");
				PrintColored(" to ", brightBlackFGColor, "");
				PrintColored("'" + valuePre + "'\n", brightBlueFGColor, "");
			}

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
			else if (IsHex(addrPre) || IsDec(addrPre) && IsDec(valuePre)) {
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
			else if (IsHex(addrPre) || IsDec(addrPre) && IsHex(valuePre)) {
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
			else if (IsHex(addrPre) || IsDec(addrPre) && IsVar(valuePre)) {
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
			else if (IsHex(addrPre) || IsDec(addrPre) && IsReg(valuePre)) {
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
			else if (IsHex(addrPre) || IsDec(addrPre) && IsPointer(valuePre)) {
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

		// arithmetic/logic commands add, sub, div, mult  ex. (add <val>,<val> -> <location>)
		else if (command == "add" || command == "sub" || command == "mult" || command == "div" ||
			command == "and" || command == "or" || command == "not" || command == "bsl" || command == "bsr")
		{
			string valAPre = trim(split(split(codelines[i], command + " ")[1], ",")[0]);
			string valBPre = trim(split(split(trim(split(codelines[i], command + " ")[1]), ",")[1], "->")[0]);
			string outLoc = trim(split(split(trim(split(codelines[i], command + " ")[1]), ",")[1], "->")[1]);
			if (verbose) {
				PrintColored("ok.	", greenFGColor, "");
				cout << "arithmetic: ";
				PrintColored("'" + command + "'  ", brightBlueFGColor, "");
				PrintColored("'" + valAPre + "' ", brightBlueFGColor, "");
				PrintColored("with ", brightBlackFGColor, "");
				PrintColored("'" + valBPre + "' ", brightBlueFGColor, "");
				PrintColored("-> ", brightBlackFGColor, "");
				PrintColored("'" + outLoc + "'\n", brightBlueFGColor, "");
			}

			int valAProcessed = ParseValue(valAPre);
			int valBProcessed = ParseValue(valBPre);
			int outLocProcessed = ParseValue(outLoc);

			compiledLines.push_back(",\n, " + command + "'  '" + valAPre + "' with '" + valBPre + "' into '" + outLoc + "'");
			compiledLines.push_back("");


			// "not" command only takes a single argument, so don't attempt to load it
			if (command != "not")
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
			// If second argument is a pointer
				else if (IsPointer(valBPre)) {
					LoadPointer(valBPre);
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
			// If first argument is a pointer
			else if (IsPointer(valAPre)) {
				LoadPointer(valAPre);
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
			if (verbose) {
				PrintColored("ok.	", greenFGColor, "");
				cout << "goto:       ";
				PrintColored("'" + addrPre + "'\n", brightBlueFGColor, "");
			}

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
			if (verbose) {
				PrintColored("ok.	", greenFGColor, "");
				cout << "gotoif:     ";
				PrintColored("'" + valAPre + " " + comparer + " " + valBPre + "'", brightBlueFGColor, "");
				PrintColored(" -> ", brightBlackFGColor, "");
				PrintColored("'" + addrPre + "'\n", brightBlueFGColor, "");
			}

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
			if (verbose) {
				PrintColored("ok.	", greenFGColor, "");
				cout << "if:         ";
				PrintColored("'" + valAPre + " " + comparer + " " + valBPre + "'\n", brightBlueFGColor, "");
			}

			compiledLines.push_back(",\n, " + string("if:   '") + valAPre + " " + comparer + " " + valBPre + "'");
			compiledLines.push_back(codelines[i]);
			continue;
		}

		// 'endif' statement
		else if (command == "endif")
		{
			if (verbose) {
				PrintColored("ok.	", greenFGColor, "");
				cout << "endif:\n";
			}

			compiledLines.push_back(",\n, " + string("endif"));
			compiledLines.push_back(codelines[i]);
			continue;
		}

		// 'asm' inline assembly
		else if (trim(split(codelines[i], "\"")[0]) == "asm")
		{
			if (verbose) {
				PrintColored("ok.	", greenFGColor, "");
				cout << "asm:\n";
			}

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

		// 'presentvbuff' swap the back/front video buffers
		else if (trim(split(codelines[i], "\"")[0]) == "presentvbuff")
		{
			if (verbose) {
				PrintColored("ok.	", greenFGColor, "");
				cout << "video buffer:\n";
			}

			compiledLines.push_back(",\n, " + string("present video buffer"));
			compiledLines.push_back("vbuf");

			continue;
		}

		// Invalid syntax or command
		else {
			PrintColored("?	unknown:    ", redFGColor, "");
			PrintColored("'" + command + "'\n", brightBlueFGColor, "");
			issues++;
		}
	}

	//string formattedstr = "";
	//for (int l = 0; l < compiledLines.size(); l++)
	//{
	//	formattedstr += trim(compiledLines[l]) + "\n";
	//}
	//compiledLines = split(formattedstr, "\n");


	cout << "* Compiling Armstrong  ";
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
