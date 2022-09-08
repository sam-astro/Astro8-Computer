
#include <vector>
#include <algorithm> 
#include <string> 
#include <chrono>
#include <limits.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <sstream>
#include <fstream>
#include <codecvt>
#include "processing.h"
#include <filesystem>

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

std::string VERSION = "Astro-8 VERSION: v1.0.1-alpha";


#if UNIX
#include <unistd.h>
#elif WINDOWS
#include <windows.h>
#endif


using namespace std;

bool compileOnly, assembleOnly, runAstroExecutable, verbose;

bool usingKeyboard = true;


int AReg = 0;
int BReg = 0;
int CReg = 0;
int BankReg = 0;
int ExpReg = 0;
uint16_t expansionPort[4];
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
int pixelRamIndex = 53871;


// 10000000 = 10.0MHz
int target_cpu_freq = 10000000;
#define TARGET_RENDER_FPS 60.0

vector<vector<int>> memoryBytes;

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
	WRITE_BNK = 0b0000010100000000,
	WRITE_EXI = 0b0000010110000000,
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
std::vector< unsigned char > pixels(108 * 108 * 4, 0);


Mix_Chunk* waveforms[4];

float speed_chunks[4] = { 1, 1, 1, 1 };


vector<std::string> instructions = { "NOP", "AIN", "BIN", "CIN", "LDIA", "LDIB", "RDEXP", "WREXP", "STA", "ADD", "SUB", "MULT", "DIV", "JMP", "JMPZ","JMPC", "JREG", "LDAIN", "STAOUT", "LDLGE", "STLGE", "LDW", "SWP", "SWPC", "PCR", "BSL", "BSR", "AND", "OR", "NOT", "BNK" };

std::string microinstructions[] = { "EO", "CE", "ST", "EI", "FL" };
std::string writeInstructionSpecialAddress[] = { "WA", "WB", "WC", "IW", "DW", "WM", "J", "AW", "WE", "BNK", "EXI"};
std::string readInstructionSpecialAddress[] = { "RA", "RB", "RC", "RM", "IR", "CR", "RE" };
std::string aluInstructionSpecialAddress[] = { "SU", "MU", "DI", "SL", "SR", "AND","OR","NOT" };
std::string flagtypes[] = { "ZEROFLAG", "CARRYFLAG" };

std::string instructioncodes[] = {
		"fetch( 0=cr,aw & 1=rm,iw,ce & 2=ei", // Fetch
		"ain( 2=aw,ir & 3=wa,rm & 4=ei", // LoadA
		"bin( 2=aw,ir & 3=wb,rm & 4=ei", // LoadB
		"cin( 2=aw,ir & 3=wc,rm & 4=ei", // LoadC
		"ldia( 2=wa,ir & 3=ei", // Load immediate A <val>
		"ldib( 2=wb,ir & 3=ei", // Load immediate B <val>
		"rdexp( 2=ir,exi & 3=wa,re & 4=ei", // Read from expansion port <index> to register A
		"wrexp( 2=ir,exi & 3=ra,we & 4=ei", // Write from reg A to expansion port <index>
		"sta( 2=aw,ir & 3=ra,wm & 4=ei", // Store A <addr>
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
		"bsr( 2=sr,wa,eo,fl & 3=ei", // Bit shift right A register, the number of bits to shift determined by the value in register B
		"and( 2=and,wa,eo,fl & 3=ei", // Logical AND operation on register A and register B, with result put back into register A
		"or( 2=or,wa,eo,fl & 3=ei", // Logical OR operation on register A and register B, with result put back into register A
		"not( 2=not,wa,eo,fl & 3=ei", // Logical NOT operation on register A, with result put back into register A
		"bnk( 2=bnk,ir & 3=ei", // Change bank, changes the memory bank register to the value specified <val>
};

std::string helpDialog = R"V0G0N(
Usage: astro8 [options] <path>

Options:
  -h, --help               Display this help menu
  -c, --compile            Only compile and assemble Armstrong code. Will not
                           start emulator.
  -a, --assemble           Only assemble assembly code into AEXE. Will not
                           start emulator.
  -r, --run                Run an already assembled program in AstroEXE fBankRegormat
                           (program.AEXE)
  -nk, --nokeyboard        Use the mouse mode for the emulator (disables
                           keyboard input)
  -v, --verbose            Write extra data to console for better debugging
  -f, --freq <value>       Override the default CPU target frequency with your
                           own.      Default = 10    higher = faster
                           High frequencies may be too hard to reach for some cpus
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
	for (int i = 0; i < sizeof(waveforms) / sizeof(waveforms[0]); i++)
	{
		Mix_FreeChunk(waveforms[i]);
		waveforms[i] = NULL;
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	Mix_Quit();
}

int clamp(int x, int min, int max) {
	if (x < min)
		return min;
	if (x > max)
		return max;

	return x;
}




/* global vars */
Uint16 audioFormat = MIX_DEFAULT_FORMAT;  // current audio format constant
int audioFrequency,  // frequency rate of the current audio format
audioChannelCount,  // number of channels of the current audio format
audioAllocatedMixChannelsCount;  // number of mix channels allocated

static inline Uint16 formatSampleSize(Uint16 format) { return (format & 0xFF) / 8; }

// Get chunk time length (in ms) given its size and current audio format
static int computeChunkLengthMillisec(int chunkSize)
{
	const Uint32 points = chunkSize / formatSampleSize(audioFormat);  // bytes / samplesize == sample points
	const Uint32 frames = (points / audioChannelCount);  // sample points / channels == sample frames
	return ((frames * 1000) / audioFrequency);  // (sample frames * 1000) / frequency == play length, in ms
}

// Custom handler object to control which part of the Mix_Chunk's audio data will be played, with which pitch-related modifications.
// This needed to be a template because the actual Mix_Chunk's data format may vary (AUDIO_U8, AUDIO_S16, etc) and the data type varies with it (Uint8, Sint16, etc)
// The AudioFormatType should be the data type that is compatible with the current SDL_mixer-initialized audio format.
template<typename AudioFormatType>
struct PlaybackSpeedEffectHandler
{
	const AudioFormatType* const chunkData;  // pointer to the chunk sample data (as array)
	const float& speedFactor;  // the playback speed factor
	float position;  // current position of the sound, in ms
	const int duration;  // the duration of the sound, in ms
	const int chunkSize;  // the size of the sound, as a number of indexes (or sample points). thinks of this as a array size when using the proper array type (instead of just Uint8*).
	const bool loop;  // flags whether playback should stay looping
	const bool attemptSelfHalting;  // flags whether playback should be halted by this callback when playback is finished
	bool altered;  // true if this playback has been pitched by this handler

	PlaybackSpeedEffectHandler(const Mix_Chunk& chunk, const float& speed, bool loop, bool trySelfHalt)
		: chunkData(reinterpret_cast<AudioFormatType*>(chunk.abuf)), speedFactor(speed),
		position(0), duration(computeChunkLengthMillisec(chunk.alen)),
		chunkSize(chunk.alen / formatSampleSize(audioFormat)),
		loop(loop), attemptSelfHalting(trySelfHalt), altered(false)
	{}

	// processing function to be able to change chunk speed/pitch.
	void modifyStreamPlaybackSpeed(int mixChannel, void* stream, int length)
	{
		AudioFormatType* buffer = static_cast<AudioFormatType*>(stream);
		const int bufferSize = length / sizeof(AudioFormatType);  // buffer size (as array)
		const float speedFactor = this->speedFactor;  // take a "snapshot" of speed factor

		// if there is still sound to be played
		if (position < duration || loop)
		{
			const float delta = 1000.0 / audioFrequency,  // normal duration of each sample
				vdelta = delta * speedFactor;  // virtual stretched duration, scaled by 'speedFactor'

		// if playback is unaltered and pitch is required (for the first time)
			if (!altered && speedFactor != 1.0f)
				altered = true;  // flags playback modification and proceed to the pitch routine.

			if (altered)  // if unaltered, this pitch routine is skipped
			{
				for (int i = 0; i < bufferSize; i += audioChannelCount)
				{
					const int j = i / audioChannelCount;  // j goes from 0 to size/channelCount, incremented 1 by 1
					const float x = position + j * vdelta;  // get "virtual" index. its corresponding value will be interpolated.
					const int k = floor(x / delta);  // get left index to interpolate from original chunk data (right index will be this plus 1)
					const float prop = (x / delta) - k;  // get the proportion of the right value (left will be 1.0 minus this)

					// usually just 2 channels: 0 (left) and 1 (right), but who knows...
					for (int c = 0; c < audioChannelCount; c++)
					{
						// check if k will be within bounds
						if (k * audioChannelCount + audioChannelCount - 1 < chunkSize || loop)
						{
							AudioFormatType v0 = chunkData[(k * audioChannelCount + c) % chunkSize],
								// v_ = chunkData[((k-1) * channelCount + c) % chunkSize],
								// v2 = chunkData[((k+2) * channelCount + c) % chunkSize],
								v1 = chunkData[((k + 1) * audioChannelCount + c) % chunkSize];

							// put interpolated value on 'data'
							// buffer[i + c] = (1 - prop) * v0 + prop * v1;  // linear interpolation
							buffer[i + c] = v0 + prop * (v1 - v0);  // linear interpolation (single multiplication)
							// buffer[i + c] = v0 + 0.5f * prop * ((prop - 3) * v0 - (prop - 2) * 2 * v1 + (prop - 1) * v2);  // quadratic interpolation
							// buffer[i + c] = v0 + (prop / 6) * ((3 * prop - prop2 - 2) * v_ + (prop2 - 2 * prop - 1) * 3 * v0 + (prop - prop2 + 2) * 3 * v1 + (prop2 - 1) * v2);  // cubic interpolation
							// buffer[i + c] = v0 + 0.5f * prop * ((2 * prop2 - 3 * prop - 1) * (v0 - v1) + (prop2 - 2 * prop + 1) * (v0 - v_) + (prop2 - prop) * (v2 - v2));  // cubic spline interpolation
						}
						else  // if k will be out of bounds (chunk bounds), it means we already finished; thus, we'll pass silence
						{
							buffer[i + c] = 0;
						}
					}
				}
			}

			// update position
			position += (bufferSize / audioChannelCount) * vdelta;

			// reset position if looping
			if (loop) while (position > duration)
				position -= duration;
		}
		else  // if we already played the whole sound but finished earlier than expected by SDL_mixer (due to faster playback speed)
		{
			// set silence on the buffer since Mix_HaltChannel() poops out some of it for a few ms.
			for (int i = 0; i < bufferSize; i++)
				buffer[i] = 0;

			if (attemptSelfHalting)
				Mix_HaltChannel(mixChannel);  // XXX unsafe call, since it locks audio; but no safer solution was found yet...
		}
	}

	// Mix_EffectFunc_t callback that redirects to handler method (handler passed via userData)
	static void mixEffectFuncCallback(int channel, void* stream, int length, void* userData)
	{
		static_cast<PlaybackSpeedEffectHandler*>(userData)->modifyStreamPlaybackSpeed(channel, stream, length);
	}

	// Mix_EffectDone_t callback that deletes the handler at the end of the effect usage  (handler passed via userData)
	static void mixEffectDoneCallback(int, void* userData)
	{
		delete static_cast<PlaybackSpeedEffectHandler*>(userData);
	}

	// function to register a handler to this channel for the next playback.
	static void registerEffect(int channel, const Mix_Chunk& chunk, const float& speed, bool loop, bool trySelfHalt)
	{
		Mix_RegisterEffect(channel, mixEffectFuncCallback, mixEffectDoneCallback, new PlaybackSpeedEffectHandler(chunk, speed, loop, trySelfHalt));
	}
};

// Register playback speed effect handler according to the current audio format; effect valid for a single playback; if playback is looped, lasts until it's halted
void setupPlaybackSpeedEffect(const Mix_Chunk* const chunk, const float& speed, int channel, bool loop = false, bool trySelfHalt = false)
{
	// select the register function for the current audio format and register the effect using the compatible handlers
	// XXX is it correct to behave the same way to all S16 and U16 formats? Should we create case statements for AUDIO_S16SYS, AUDIO_S16LSB, AUDIO_S16MSB, etc, individually?
	switch (audioFormat)
	{
	case AUDIO_U8:  PlaybackSpeedEffectHandler<Uint8 >::registerEffect(channel, *chunk, speed, loop, trySelfHalt); break;
	case AUDIO_S8:  PlaybackSpeedEffectHandler<Sint8 >::registerEffect(channel, *chunk, speed, loop, trySelfHalt); break;
	case AUDIO_U16: PlaybackSpeedEffectHandler<Uint16>::registerEffect(channel, *chunk, speed, loop, trySelfHalt); break;
	default:
	case AUDIO_S16: PlaybackSpeedEffectHandler<Sint16>::registerEffect(channel, *chunk, speed, loop, trySelfHalt); break;
	case AUDIO_S32: PlaybackSpeedEffectHandler<Sint32>::registerEffect(channel, *chunk, speed, loop, trySelfHalt); break;
	case AUDIO_F32: PlaybackSpeedEffectHandler<float >::registerEffect(channel, *chunk, speed, loop, trySelfHalt); break;
	}
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
	memoryBytes.push_back(vector<int>());
	memoryBytes.push_back(vector<int>());

	// Fill the memory
	for (int memindex = 0; memindex < 65535; memindex++)
		memoryBytes[1].push_back(0);


	// Get the executable's installed directory
	executableDirectory = filesystem::weakly_canonical(filesystem::path(argv[0])).parent_path().string();

	std::string code = "";
	std::string filePath = "";
	std::string programName = "program";

	// If no arguments are provided, ask for a path
	if (argc == 1)
	{
		PrintColored("No arguments detected. If this is your first time using this program,\nhere is the help menu for assistance:", yellowFGColor, "");
		cout << "\n" << helpDialog << "\n";
		cout << ("OR, enter path to file >  ");
		std::string line;
		while (true) {
			getline(cin, line);
			if (line.empty()) {
				continue;
			}
			else {
				filePath = line;
				break;
			}
		}
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
		else if (argval == "-c" || argval == "--compile") // Only compile and assemble code. Will not start emulator.
			compileOnly = true;
		else if (argval == "-a" || argval == "--assemble") // Only assemble code. Will not start emulator.
			assembleOnly = true;
		else if (argval == "-r" || argval == "--run") // Run an already assembled program in AstroEXE format
			runAstroExecutable = true;
		else if (argval == "-nk" || argval == "--nokeyboard") // Use the mouse mode for the emulator (disables keyboard input)
			usingKeyboard = false;
		else if (argval == "-v" || argval == "--verbose") // Write extra data to console for better debugging
			verbose = true;
		else if (argval == "-f" || argval == "--freq") { // Override the default CPU frequency with your own.
			try
			{
				target_cpu_freq = stoi(argv[i + 1]) * 1000000;
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
	if (split(filePath, "\n")[0].find('/') != std::string::npos || split(filePath, "\n")[0].find('\\') != std::string::npos) {
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

	}
	else if (argc != 1) {
		PrintColored("\nError: could not open file ", redFGColor, "");
		PrintColored("\"" + code + "\"\n", brightBlueFGColor, "");
		cout << "\n\nPress Enter to Exit...";
		cin.ignore();
		exit(1);
	}


	// Determine if the file is an AstroExecutable AEXE
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
			vector<std::string> mbytes = parseCode(code);
			for (int memindex = 0; memindex < mbytes.size(); memindex++)
				memoryBytes[0].push_back(HexToDec(mbytes[memindex]));

			// Store memory into an .AEXE file
			std::ofstream f(projectDirectory + programName + ".aexe");
			f << "ASTRO-8 AEXE Executable file" << '\n';
			f << (usingKeyboard == true ? "1" : "0") << '\n';
			for (vector<string>::const_iterator i = mbytes.begin(); i != mbytes.end(); ++i) {
				f << *i << '\n';
			}
			f.close();
			PrintColored("Binary executable written to " + projectDirectory + programName + ".aexe\n", whiteFGColor, "");
		}
		catch (const std::exception&)
		{
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
			// This lets people distribute AEXE files wihtout having to describe
			// the exact options to get it working
			if (trim(filelines[1]).size() >= 1) {
				usingKeyboard = trim(filelines[1])[0] == '1';
			}

			// Skip two lines to begin reading AEXE data
			for (int memindex = 2; memindex < filelines.size(); memindex++)
				memoryBytes[0].push_back(HexToDec(filelines[memindex]));
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


	// Start Emulation

	cout << "\n\n";
	PrintColored("Starting Emulation...", blackFGColor, whiteBGColor);
	cout << "\n\n";

	InitGraphics("Astro-8 Emulator", 108, 108, 5);


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
		if (tickDiff > (numUpdates * 1000.0 / target_cpu_freq)) {
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
				// If using the keyboard in the expansion port
				if (usingKeyboard) {
					if (event.type == SDL_KEYDOWN) {
						// Keyboard only uses lowest 8 bits, so the upper ones stay
						expansionPort[0] = ConvertAsciiToSdcii((int)(event.key.keysym.scancode)) + (expansionPort[0] & 0b1111111100000000);

						PrintColored("\n	-- keypress << ", brightBlackFGColor, "");
						PrintColored(to_string(expansionPort[0]), greenFGColor, "");

					}
					else if (event.type == SDL_KEYUP) {

						expansionPort[0] = 168; // Keyboard idle state is 168 (max value), since 0 is reserved for space
					}
				}
				// If using the mouse in the expansion port
				else if (!usingKeyboard)
					if (event.type == SDL_MOUSEMOTION) {
						// Get mouse location
						//cout << event.motion.x << endl;
						expansionPort[1] = ((event.motion.x << 6) + event.motion.y) + (expansionPort[1] & 0b1111000000000000);
					}
					else if (event.type == SDL_MOUSEBUTTONDOWN) {
						if (event.button.button == 1)      // Left Mouse Button Down
							expansionPort[1] = 4096 | expansionPort[1];
						else if (event.button.button == 3) // Right Mouse Button Down
							expansionPort[1] = 8192 | expansionPort[1];
					}
					else if (event.type == SDL_MOUSEBUTTONUP) {
						if (event.button.button == 1 && (expansionPort[1] & 4096) == 4096)      // Left Mouse Button Up
							expansionPort[1] = 4096 ^ expansionPort[1];
						else if (event.button.button == 3 && (expansionPort[1] & 8192) == 8192) // Right Mouse Button Up
							expansionPort[1] = 8192 ^ expansionPort[1];
					}
			}
		}
	}

	destroy(gRenderer, gWindow);
	SDL_Quit();

	return 0;
}

bool channelsPlaying[] = { false, false, false, false };
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
			InstructionReg = memoryBytes[0][programCounter];
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
			bus = memoryBytes[BankReg][memoryIndex];
			break;
		case READ_IR:
			bus = InstructionReg & 0b11111111111;
			break;
		case READ_CR:
			bus = programCounter;
			break;
		case READ_RE:
			bus = expansionPort[ExpReg];
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
					bus = 65535 + bus;
					flags[1] = 0;
				}
				break;

			case ALU_MU: // Multiply
				if (AReg * BReg == 0)
					flags[0] = 1;
				bus = AReg * BReg;
				if (bus >= 65535)
				{
					bus = bus - 65535;
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

				if (bus >= 65535)
				{
					bus = bus - 65535;
					flags[1] = 1;
				}
				break;

			case ALU_SL: // Logical bit shift left
				bus = AReg << (BReg & 0b1111);

				if (bus == 0)
					flags[0] = 1;

				if (bus >= 65535)
				{
					bus = bus - 65535;
					flags[1] = 1;
				}
				break;

			case ALU_SR: // Logical bit shift right
				bus = AReg >> (BReg & 0b1111);

				if (bus == 0)
					flags[0] = 1;

				if (bus >= 65535)
				{
					bus = bus - 65535;
					flags[1] = 1;
				}
				break;

			case ALU_AND: // Logical AND
				bus = AReg & BReg;

				if (bus == 0)
					flags[0] = 1;

				if (bus >= 65535)
				{
					bus = bus - 65535;
					flags[1] = 1;
				}
				break;

			case ALU_OR: // Logical OR
				bus = AReg | BReg;

				if (bus == 0)
					flags[0] = 1;

				if (bus >= 65535)
				{
					bus = bus - 65535;
					flags[1] = 1;
				}
				break;

			case ALU_NOT: // Logical NOT
				bus = ~AReg;

				if (bus == 0)
					flags[0] = 1;

				if (bus >= 65535)
				{
					bus = bus - 65535;
					flags[1] = 1;
				}
				break;

			default: // Add
				if (AReg + BReg == 0)
					flags[0] = 1;
				bus = AReg + BReg;
				if (bus >= 65535)
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
			memoryBytes[BankReg][memoryIndex] = bus;
			//cout <<endl<< to_string(BankReg )<< " , " << to_string(memoryIndex )<< " = " << bus << endl;
			break;
		case WRITE_J:
			programCounter = bus;
			break;
		case WRITE_AW:
			memoryIndex = bus;
			break;
		case WRITE_BNK:
			//PrintColored("\nChange from: " + to_string(BankReg) + " to " + to_string(bus) + "\n", whiteFGColor, "");
			BankReg = bus & 1;
			break;
		case WRITE_WE:
			expansionPort[ExpReg] = bus;
			if (verbose) {
				PrintColored("\n	-- cout >> ", brightBlackFGColor, "");
				PrintColored(to_string(expansionPort[ExpReg]), greenFGColor, "");
				cout << "\n";
				//PrintColored(DecToBinFilled(expansionPort[ExpReg], 16), greenFGColor, "");
				//cout << "\n";
			}

			////////////
			// Audio: //
			////////////

			//		Format:     FFFFFCCC XXXXXXXX
			//		You can only toggle one channel at a time per expansion port write.
			//		CCC is converted to the index of the channel that is toggled
			//		Then the frequency for that channel is defined as an int value stored in FFFFF
			//      If FFFFF is all Zeros, then the channel is turned off. Otherwise, the frequency
			//		is changed and the channel is turned ON if it isn't already
			//      CCC indexing starts at 1 instead of 0 to prevent accidental audio output

			//testFreqInt += 1000;

			// Calculate target frequency from beginning 5-bits
			float offset = 0.0f;
			float targetSpeed = (((expansionPort[ExpReg] & 0b1111100000000000) >> 11) / 15.0f) + offset;
			int targetChannel = (expansionPort[ExpReg] & 0b11100000000) >> 8;
			//cout << targetChannel << " : " << targetSpeed << endl;

			// Use upper 8 bits to play audio
			if (targetChannel > 0 && targetChannel <= 4)
				if (Mix_Playing(targetChannel - 1) == false && targetSpeed > offset) {
					speed_chunks[targetChannel - 1] = targetSpeed;
					Mix_PlayChannel(targetChannel - 1, waveforms[targetChannel - 1], -1);
					setupPlaybackSpeedEffect(waveforms[targetChannel - 1], speed_chunks[targetChannel - 1], targetChannel - 1, true, true);
					//cout << "Play & Frequency change" << endl;
					//channelsPlaying[targetChannel - 1] = true;
				}
				else if (Mix_Playing(targetChannel - 1) == true && targetSpeed > offset && speed_chunks[targetChannel - 1] != targetSpeed) {
					//Mix_HaltChannel(targetChannel - 1);
					speed_chunks[targetChannel - 1] = targetSpeed;
					//Mix_PlayChannel(targetChannel - 1, waveforms[targetChannel - 1], -1);
					//setupPlaybackSpeedEffect(waveforms[targetChannel - 1], speed_chunks[targetChannel - 1], targetChannel - 1, true, true);
					//cout << "Frequency change" << endl;
				}
				else if (Mix_Playing(targetChannel - 1) == true && targetSpeed == offset) {
					//Mix_FadeOutChannel(targetChannel - 1, 10);
					Mix_HaltChannel(targetChannel - 1);
					//channelsPlaying[targetChannel - 1] = false;
				}




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
	int characterRamValue = memoryBytes[1][characterRamIndex + 53546];
	bool charPixRomVal = characterRom[(characterRamValue * 64) + (charPixY * 8) + charPixX];

	int pixelVal = memoryBytes[1][pixelRamIndex];
	int r, g, b;

	if (charPixRomVal == true) {
		r = 255;
		g = 255;
		b = 255;
	}
	else {
		r = BitRange(pixelVal, 10, 5) * 8; // Get first 5 bits
		g = BitRange(pixelVal, 5, 5) * 8; // get middle bits
		b = BitRange(pixelVal, 0, 5) * 8; // Gets last 5 bits
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

void Draw() {
	while (true) {
		DrawNextPixel();
		if (pixelRamIndex >= 65535) {
			pixelRamIndex = 53871;
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

	//Initialize SDL_mixer
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096) < 0)
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());

	Mix_QuerySpec(&audioFrequency, &audioFormat, &audioChannelCount);  // query specs
	audioAllocatedMixChannelsCount = Mix_AllocateChannels(MIX_CHANNELS);

	// Load waves
	waveforms[0] = Mix_LoadWAV((executableDirectory + "/square.wav").c_str());
	if (waveforms[0] == NULL)
		cout << ("Failed to load sound:" + executableDirectory + "/square.wav" + " SDL_mixer Error: " + Mix_GetError() + "\n");
	waveforms[1] = Mix_LoadWAV((executableDirectory + "/square.wav").c_str());
	if (waveforms[1] == NULL)
		cout << ("Failed to load sound:" + executableDirectory + "/square.wav" + " SDL_mixer Error: " + Mix_GetError() + "\n");
	waveforms[2] = Mix_LoadWAV((executableDirectory + "/triangle.wav").c_str());
	if (waveforms[2] == NULL)
		cout << ("Failed to load sound:" + executableDirectory + "/triangle.wav" + " SDL_mixer Error: " + Mix_GetError() + "\n");
	waveforms[3] = Mix_LoadWAV((executableDirectory + "/noise.wav").c_str());
	if (waveforms[3] == NULL)
		cout << ("Failed to load sound:" + executableDirectory + "/noise.wav" + " SDL_mixer Error: " + Mix_GetError() + "\n");

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
		if (splitBySpace[0] == "SET")
		{
			int addr = stoi(splitBySpace[1]);
			std::string hVal = DecToHexFilled(stoi(splitBySpace[2]), 4);
			outputBytes[addr] = hVal;
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
			outputBytes[addr] = hVal;
#if DEV_MODE
			cout << ("-\t" + splitcode[i] + "\t  ~   ~\n");
#endif
			memaddr += 1;
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
		for (int f = 0; f < instructions.size(); f++)
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
		std::string startaddress = DecToBinFilled(ins, 5);

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
#if DEV_MODE
		cout << (instructioncodes[ins] + "\n");
#endif

		std::string startaddress = DecToBinFilled(ins, 5);

		vector<std::string> instSteps = explode(instructioncodes[ins], '&');
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


	// Save the data to logisim_mic.hex
	fstream myStream;
	myStream.open(projectDirectory + "logisim_mic.hex", ios::out);
	myStream << processedOutput;
}

vector<string> vars;
vector<string> labels;
vector<int> labelLineValues;
vector<string> compiledLines;



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
	string addrInWord = "ain ";
	int actualLineNum = GetLineNumber();

	if (reg == "@A")
		addrInWord = "ain ";
	else if (reg == "@B")
		addrInWord = "bin ";
	else if (reg == "@C")
		addrInWord = "cin ";
	else if (split(reg, "[")[0] == "@EX")
		addrInWord = "ain ";

	// Value is small enough to be accessible through normal r/w instructions
	if (actualVal <= 2047) {
		compiledLines.push_back(addrInWord + to_string(actualVal));
		if (split(reg, "[")[0] == "@EX")
			compiledLines.push_back("wrexp " + split(split(reg, "[")[1], "]")[0]);
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
		else if (split(in, "[")[0] == "@EX") {
			compiledLines.push_back("ldia " + to_string(actualValue));
			compiledLines.push_back("wrexp "+split(split(in, "[")[1], "]")[0]);
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
		else if (split(in, "[")[0] == "@EX") {
			compiledLines.push_back("ldw");
			PutSetOnCurrentLine(followingValue);
			compiledLines.push_back("wrexp " + split(split(in, "[")[1], "]")[0]);
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
	if (destination == "@A" && split(from, "[")[0] == "@EX")
		return "rdexp " + split(split(from, "[")[1], "]")[0]+"\n";

	if (destination == "@B" && from == "@A")
		return "swp\n";
	if (destination == "@B" && from == "@C")
		return "swpc\nswp\n";
	if (destination == "@B" && split(from, "[")[0] == "@EX")
		return "rdexp " + split(split(from, "[")[1], "]")[0]+"\nswp\n";

	if (destination == "@C" && from == "@A")
		return "swpc\n";
	if (destination == "@C" && from == "@B")
		return "swp\nswpc\n";
	if (destination == "@C" && split(from, "[")[0] == "@EX")
		return "rdexp " + split(split(from, "[")[1], "]")[0] + "\nswpc\n";

	if (split(destination, "[")[0] == "@EX" && from == "@A")
		return "wrexp " + split(split(destination, "[")[1], "]")[0] + "\n";
	if (split(destination, "[")[0] == "@EX" && from == "@B")
		return "swp\nwrexp " + split(split(destination, "[")[1], "]")[0] + "\n";
	if (split(destination, "[")[0] == "@EX" && from == "@C")
		return "swpc\nwrexp " + split(split(destination, "[")[1], "]")[0] + "\n";

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
			return i + 16382;

	// Not found, add to list and return size-1
	vars.push_back(id);
	return 16382 + vars.size() - 1;
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
		std::string pval = split(input, "*")[1];
		return ParseValue(pval.substr(pval.find_last_of("]") + 1, pval.size()));
	}

	return -1;
}

// Reads from mem at the address stored in pointer, into REG A
void LoadPointer(const string& str) {
	compiledLines.push_back("bnk " + str.substr(str.find_last_of("[") + 1, path.size())[0]);
	LoadAddress("@A", str.substr(str.find_last_of("]") + 1, path.size()));
	compiledLines.push_back("ldain");
	compiledLines.push_back("bnk 0");
}

// Writes from REG B to mem at the address stored in pointer
void StoreIntoPointer(const string& str) {
	compiledLines.push_back("bnk " + str.substr(str.find_last_of("[") + 1, path.size())[0]);
	LoadAddress("@A", split(str, "*")[1]);
	compiledLines.push_back("staout");
	compiledLines.push_back("bnk 0");
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
