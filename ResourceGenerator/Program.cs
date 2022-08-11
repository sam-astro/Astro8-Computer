
//Microsoft (R) Visual C# Compiler version 3.4.0-beta4-19562-05 (ff930dec)
//Copyright (C) Microsoft Corporation. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using System.Text;
using System.IO;
using System.Drawing;

public class Program
{
    static string[] instructions = { "NOP", "AIN", "BIN", "CIN", "LDIA", "LDIB", "RDEXP", "WREXP", "STA", "STC", "ADD", "SUB", "MULT", "DIV", "JMP", "JMPZ", "JMPC", "LDAIN", "HLT", "OUT" };
    static string action = "";
    static string[] microinstructionData = new string[2048];

    public static void Main(string[] args)
    {
        Console.Write("Generating Resources, Please Wait... ");
        MakeCharacterRom();
        Console.Write(" Done!\n\n");
        Console.Write("Anything else? >  ");
        action = Console.ReadLine().ToLower();


        if (action == "imggen")
        {
            Console.Write("Path to image >  ");
            string path = Console.ReadLine().Replace("\"", "");
            Bitmap img = ResizeBitmap(new Bitmap((Bitmap)Image.FromFile(path)), 64, 64);

            string code = "";
            int currentMemIndex = 0;
            code += "#AS\r";
            for (int pos = 0; pos < img.Width * img.Height; pos++)
            {
                Color col = img.GetPixel(pos % img.Width, pos / img.Height);
                int r = col.R / 8;
                int g = col.G / 8;
                int b = col.B / 8;
                string binval = "0" + DecToBinFilled(r, 5) + DecToBinFilled(g, 5) + DecToBinFilled(b, 5);
                int decval = BinToDec(binval);

                code += "define " + (200 + currentMemIndex) + " " + decval + "\r";
                currentMemIndex++;
            }
            code += File.ReadAllText("./draw_image.txt") + "\r";
            File.WriteAllText("./code_text_val.txt", code, Encoding.UTF8);

        }

        Console.WriteLine("\nDone. Press ENTER to EXIT\n");
        Console.ReadLine();
    }

    static string DecToHexFilled(int input, int desiredSize)
    {
        string output = input.ToString("X");

        while (output.Length < desiredSize)
        {
            output = "0" + output;
        }

        return output;
    }
    static string BinToHexFilled(string input, int desiredSize)
    {
        string output = Convert.ToInt32(input, 2).ToString("X");

        while (output.Length < desiredSize)
        {
            output = "0" + output;
        }

        return output;
    }
    static int BinToDec(string input)
    {
        int output = Convert.ToInt32(input, 2);
        return output;
    }
    static string DecToBinFilled(int input, int desiredSize)
    {
        string output = Convert.ToString(input, 2);

        while (output.Length < desiredSize)
        {
            output = "0" + output;
        }

        return output;
    }
    private static readonly Dictionary<char, string> hexCharacterToBinary = new Dictionary<char, string> {
    { '0', "0000" },
    { '1', "0001" },
    { '2', "0010" },
    { '3', "0011" },
    { '4', "0100" },
    { '5', "0101" },
    { '6', "0110" },
    { '7', "0111" },
    { '8', "1000" },
    { '9', "1001" },
    { 'a', "1010" },
    { 'b', "1011" },
    { 'c', "1100" },
    { 'd', "1101" },
    { 'e', "1110" },
    { 'f', "1111" }
};
    static Bitmap ResizeBitmap(Bitmap sourceBMP, int width, int height)
    {
        Bitmap result = new Bitmap(width, height);
        using (Graphics g = Graphics.FromImage(result))
        {
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
            g.DrawImage(sourceBMP, 0, 0, width, height);
        }
        return result;
    }
    static string HexToBin(string hex, int desiredSize)
    {
        StringBuilder result = new StringBuilder();
        foreach (char c in hex)
        {
            // This will crash for non-hex characters. You might want to handle that differently.
            result.Append(hexCharacterToBinary[char.ToLower(c)]);
        }
        string output = result.ToString();
        while (output.Length < desiredSize)
        {
            output = "0" + output;
        }
        if (output.Length > desiredSize)
            output = output.Substring(output.Length - desiredSize);
        return output;
    }

    static int HexToDec(string hex)
    {
        return int.Parse(hex, System.Globalization.NumberStyles.HexNumber);
    }

    static void MakeCharacterRom()
    {
        string path = "../../../../character-set.png";
        Bitmap img = new Bitmap((Bitmap)Image.FromFile(path));
        List<int> l = new List<int>();
        for (int i = 0; i < 65535; i++)
            l.Add(0);

        int charCode = 0;
        int charX = 0;
        int charY = 0;

        for (int y = 0; y < img.Height; y++)
        {
            charX = 0;
            for (int x = 0; x < img.Width; x++)
            {
                l[BinToDec(DecToBinFilled(charCode, 7)+DecToBinFilled(charY, 3)+DecToBinFilled(charX, 3))] = img.GetPixel(x, y).R/255;
                charX++;
                if (charX == 6)
                {
                    charCode++;
                    charX = 0;
                }
            }
            charY++;
            if (charY<6)
                charCode -= 13;
            else
                charY = 0;
        }

        string mt = "";
        string processedOutput = "";

        // Print the output
        processedOutput += "\nv3.0 hex words addressed\n";
        processedOutput += "000: ";
        for (int outindex = 0; outindex < l.Count; outindex++)
        {
            if (outindex % 8 == 0 && outindex != 0)
            {
                processedOutput += "\n" + DecToHexFilled(outindex, 3) + ": ";
            }
            processedOutput += l[outindex] + " ";
            mt+=l[outindex];
        }
        //Console.WriteLine(processedOutput);
        //Console.WriteLine(mt);

        File.WriteAllText("../../../../char_set_processed.hex", processedOutput);
        File.WriteAllText("../../../../char_set_memtape", mt);
    }

    static List<string> parseCode(string input)
    {
        List<string> outputBytes = new List<string>();
        for (int i = 0; i < 4000; i++)
            outputBytes.Add("0000");

        string[] splitcode = input.ToUpper().Split('\n');

        int memaddr = 0;
        for (int i = 0; i < splitcode.Length; i++)
        {
            if (splitcode[i] == null || splitcode[i] == "")
            {
                continue;
            }

            string[] splitBySpace = splitcode[i].Split(' ');

            if (splitBySpace[0][0] == ',')
            {
                Console.Write("-\t" + splitcode[i] + "\n");
                continue;
            }
            if (splitBySpace[0] == "SET")
            {
                string hVal = DecToHexFilled(Int32.Parse(splitBySpace[2]), 4);
                outputBytes[Int32.Parse(splitBySpace[1])] = hVal;
                Console.Write("-\t" + splitcode[i] + "\t  ~   ~\n");
                continue;
            }

            Console.Write(memaddr + " " + splitcode[i] + "   \t  =>  ");

            // Find index of instruction
            for (int f = 0; f < instructions.Length; f++)
            {
                if (instructions[f] == splitBySpace[0])
                {
                    Console.Write(DecToBinFilled(f, 5));
                    outputBytes[memaddr] = DecToBinFilled(f, 5);
                    break;
                }
            }

            // Check if any args are after the command
            if (splitcode[i] != splitBySpace[0])
            {
                Console.Write(" "+DecToBinFilled(Int32.Parse(splitBySpace[1]), 11));
                outputBytes[memaddr] += DecToBinFilled(Int32.Parse(splitBySpace[1]), 11);
            }
            else
            {
                Console.Write(" 00000000000");
                outputBytes[memaddr] += "00000000000";
            }
            Console.Write("  " +BinToHexFilled(outputBytes[memaddr], 4)+"\n");
            outputBytes[memaddr] = BinToHexFilled(outputBytes[memaddr], 4); // Convert from binary to hex
            memaddr++;
        }
        return outputBytes;
    }

    static void GenerateMicrocode()
    {
        // Generate zeros in data
        string[] output = new string[2048];
        for (int osind = 0; osind < output.Length; osind++) { output[osind] = "00000"; }

        string[] microinstructions = { "EO", "CE", "ST", "EI", "FL" };
        string[] writeInstructionSpecialAddress = { "WA", "WB", "WC", "IW", "DW", "WM", "J", "AW", "WE" };
        string[] readInstructionSpecialAddress = { "RA", "RB", "RC", "RM", "IR", "CR", "RE" };
        string[] aluInstructionSpecialAddress = { "SU", "MU", "DI" };
        string[] flags = { "ZEROFLAG", "CARRYFLAG" };
        //   "LDAIN", "HLT", "OUT"

        string[] instructioncodes = {
                "fetch( 0=aw,cr & 1=rm,iw,ce & 2=ei", // Fetch
                "ain( 2=aw,ir & 3=wa,rm & 4=ei", // LoadA
                "bin( 2=aw,ir & 3=wb,rm & 4=ei", // LoadB
                "cin( 2=aw,ir & 3=wc,rm & 4=ei", // LoadC
                "ldia( 2=wa,ir & 3=ei", // Load immediate A <val>
                "ldib( 2=wb,ir & 3=ei", // Load immediate B <val>
                "rdexp( 2=wb,re & 3=ei", // Read from expansion port to register B
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
                "ldain( 2=ra,aw & 3=wa,rm & 4=ei", // Load from reg A as memory address, then copy value from memory into A
                "hlt( 2=st & 3=ei", // Stop the computer clock
                "out( 2=ra,dw & 3=ei", // Output to decimal display and LCD screen
            };

        // Remove spaces from instruction codes and make uppercase
        for (int cl = 0; cl < instructioncodes.Length; cl++)
        {
            string newStr = "";
            for (int clc = 0; clc < instructioncodes[cl].Length; clc++)
            {
                if (instructioncodes[cl][clc] != ' ')
                    newStr += instructioncodes[cl][clc];
            }
            Console.WriteLine(newStr.ToUpper());
            instructioncodes[cl] = newStr.ToUpper();
        }

        // Create indexes for instructions, which allows for duplicates to execute differently for different parameters
        int[] instIndexes = new int[instructioncodes.Length];
        List<string> seenNames = new List<string>();
        for (int cl = 0; cl < instructioncodes.Length; cl++)
        {
            string instName = instructioncodes[cl].Split('(')[0];
            bool foundInList = false;
            for (int clc = 0; clc < seenNames.Count; clc++)
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
                seenNames.Add(instName);
                instIndexes[cl] = seenNames.Count - 1;
            }
            instructioncodes[cl] = instructioncodes[cl].Split('(')[1];
        }

        // Special process fetch instruction
        Console.WriteLine("\n" + instructioncodes[0]);
        for (int ins = 0; ins < instructioncodes.Length; ins++) // Iterate through all definitions of instructions
        {
            int correctedIndex = instIndexes[ins];

            string startaddress = DecToBinFilled(correctedIndex, 5);

            string[] instSteps = instructioncodes[0].Split('&');
            for (int step = 0; step < instSteps.Length; step++) // Iterate through every step
            {
                int actualStep = int.Parse(instSteps[step].Split('=')[0]);
                string stepContents = instSteps[step].Split('=')[1].Split('|')[0];

                string midaddress = DecToBinFilled(actualStep, 4);

                char[] stepComputedInstruction = new char[14] { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };
                for (int mins = 0; mins < microinstructions.Length; mins++)
                {
                    // Check if microinstruction matches at index
                    if (stepContents.Contains(microinstructions[mins]))
                    {
                        stepComputedInstruction[mins] = '1'; // activate
                    }

                    // Check if microinstruction requires special code
                    for (int minsother = 0; minsother < writeInstructionSpecialAddress.Length; minsother++)
                    { // Check all write instruction types
                        if (stepContents.Contains(writeInstructionSpecialAddress[minsother]))
                        {
                            string binaryval = DecToBinFilled(minsother + 1, 4);
                            stepComputedInstruction[5] = binaryval[0];
                            stepComputedInstruction[6] = binaryval[1];
                            stepComputedInstruction[7] = binaryval[2];
                            stepComputedInstruction[8] = binaryval[3];
                        }
                    }

                    // Check if microinstruction requires special code
                    for (int minsother = 0; minsother < readInstructionSpecialAddress.Length; minsother++)
                    { // Check all read instruction types
                        if (stepContents.Contains(readInstructionSpecialAddress[minsother]))
                        {
                            string binaryval = DecToBinFilled(minsother + 1, 3);
                            stepComputedInstruction[9] = binaryval[0];
                            stepComputedInstruction[10] = binaryval[1];
                            stepComputedInstruction[11] = binaryval[2];
                        }
                    }

                    // Check if microinstruction requires special code
                    for (int minsother = 0; minsother < aluInstructionSpecialAddress.Length; minsother++)
                    { // Check all ALU instruction types
                        if (stepContents.Contains(aluInstructionSpecialAddress[minsother]))
                        {
                            string binaryval = DecToBinFilled(minsother + 1, 2);
                            stepComputedInstruction[12] = binaryval[0];
                            stepComputedInstruction[13] = binaryval[1];
                        }
                    }
                }

                // Compute flags combinations
                for (int flagcombinations = 0; flagcombinations < flags.Length * flags.Length; flagcombinations++)
                {
                    char[] endaddress = { '0', '0' };
                    // Look for flags
                    if (instSteps[step].Contains("|"))
                    {
                        string[] inststepFlags = instSteps[step].Split('|')[1].Split(',');
                        for (int flag = 0; flag < inststepFlags.Length; flag++) // Iterate through all flags in step
                        {
                            for (int checkflag = 0; checkflag < flags.Length; checkflag++) // What is the index of the flag
                            {
                                if (inststepFlags[flag] == flags[checkflag])
                                    endaddress[checkflag] = '1';
                            }
                        }
                    }
                    char[] newendaddress = DecToBinFilled(flagcombinations, 2).ToCharArray();

                    bool doesntmatch = false;
                    for (int i = 0; i < endaddress.Length; i++)
                    {
                        if (endaddress[i] == '1')
                        {
                            if (newendaddress[i] != '1')
                                doesntmatch = true;
                        }
                    }
                    if (doesntmatch)
                        continue;

                    Console.WriteLine("\t& " + startaddress + " " + midaddress + " " + new string(newendaddress) + "  =  " + BinToHexFilled(string.Join("", stepComputedInstruction), 4));
                    output[BinToDec(startaddress + midaddress + new string(newendaddress))] = BinToHexFilled(string.Join("", stepComputedInstruction), 5);
                }
            }

            //Console.WriteLine();
        }

        // Do actual processing
        for (int ins = 1; ins < instructioncodes.Length; ins++) // Iterate through all definitions of instructions
        {
            int correctedIndex = instIndexes[ins];

            Console.WriteLine(instructioncodes[correctedIndex]);

            string startaddress = DecToBinFilled(correctedIndex, 5);

            string[] instSteps = instructioncodes[correctedIndex].Split('&');
            for (int step = 0; step < instSteps.Length; step++) // Iterate through every step
            {
                int actualStep = int.Parse(instSteps[step].Split('=')[0]);
                string stepContents = instSteps[step].Split('=')[1].Split('|')[0];

                string midaddress = DecToBinFilled(actualStep, 4);

                char[] stepComputedInstruction = new char[14] { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };
                for (int mins = 0; mins < microinstructions.Length; mins++)
                {
                    // Check if microinstruction matches at index
                    if (stepContents.Contains(microinstructions[mins]))
                    {
                        stepComputedInstruction[mins] = '1'; // activate
                    }

                    // Check if microinstruction requires special code
                    for (int minsother = 0; minsother < writeInstructionSpecialAddress.Length; minsother++)
                    { // Check all write instruction types
                        if (stepContents.Contains(writeInstructionSpecialAddress[minsother]))
                        {
                            string binaryval = DecToBinFilled(minsother + 1, 4);
                            stepComputedInstruction[5] = binaryval[0];
                            stepComputedInstruction[6] = binaryval[1];
                            stepComputedInstruction[7] = binaryval[2];
                            stepComputedInstruction[8] = binaryval[3];
                        }
                    }

                    // Check if microinstruction requires special code
                    for (int minsother = 0; minsother < readInstructionSpecialAddress.Length; minsother++)
                    { // Check all read instruction types
                        if (stepContents.Contains(readInstructionSpecialAddress[minsother]))
                        {
                            string binaryval = DecToBinFilled(minsother + 1, 3);
                            stepComputedInstruction[9] = binaryval[0];
                            stepComputedInstruction[10] = binaryval[1];
                            stepComputedInstruction[11] = binaryval[2];
                        }
                    }

                    // Check if microinstruction requires special code
                    for (int minsother = 0; minsother < aluInstructionSpecialAddress.Length; minsother++)
                    { // Check all ALU instruction types
                        if (stepContents.Contains(aluInstructionSpecialAddress[minsother]))
                        {
                            string binaryval = DecToBinFilled(minsother + 1, 2);
                            stepComputedInstruction[12] = binaryval[0];
                            stepComputedInstruction[13] = binaryval[1];
                        }
                    }
                }

                // Compute flags combinations
                for (int flagcombinations = 0; flagcombinations < flags.Length * flags.Length; flagcombinations++)
                {
                    char[] endaddress = { '0', '0' };
                    int[] stepLocked = { 0, 0 };
                    // If flags are specified in current step layer, set them to what is specified and lock that bit
                    if (instSteps[step].Contains("|"))
                    {
                        string[] inststepFlags = instSteps[step].Split('|')[1].Split(',');
                        for (int flag = 0; flag < inststepFlags.Length; flag++) // Iterate through all flags in step
                        {
                            for (int checkflag = 0; checkflag < flags.Length; checkflag++) // What is the index of the flag
                            {
                                if (inststepFlags[flag].Contains(flags[checkflag]))
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
                    char[] newendaddress = DecToBinFilled(flagcombinations, 2).ToCharArray();

                    // Make sure the current combination doesn't change the locked bits, otherwise go to next step
                    bool doesntmatch = false;
                    for (int i = 0; i < endaddress.Length; i++)
                    {
                        if (stepLocked[i] == 1)
                        {
                            if (newendaddress[i] != endaddress[i])
                                doesntmatch = true;
                        }
                    }
                    if (doesntmatch)
                        continue;

                    Console.WriteLine("\t& " + startaddress + " " + midaddress + " " + new string(newendaddress) + "  =  " + BinToHexFilled(string.Join("", stepComputedInstruction), 4));
                    output[BinToDec(startaddress + midaddress + new string(newendaddress))] = BinToHexFilled(string.Join("", stepComputedInstruction), 5);
                }
            }

            //Console.WriteLine();
        }


        string processedOutput = "";

        // Print the output
        Console.Write("\nv3.0 hex words addressed\n");
        processedOutput += "\nv3.0 hex words addressed\n";
        Console.Write("000: ");
        processedOutput += "000: ";
        for (int outindex = 0; outindex < output.Length; outindex++)
        {
            if (outindex % 8 == 0 && outindex != 0)
            {
                Console.Write("\n" + DecToHexFilled(outindex, 3) + ": ");
                processedOutput += "\n" + DecToHexFilled(outindex, 3) + ": ";
            }
            Console.Write(output[outindex] + " ");
            processedOutput += output[outindex] + " ";
            microinstructionData[outindex] = HexToBin(output[outindex], 17);
        }

        File.WriteAllText("../../../../microinstructions_cpu_v1", processedOutput);
    }
}
