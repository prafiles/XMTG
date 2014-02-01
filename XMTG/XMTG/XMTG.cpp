//C++11 Program. Tested on Windows 8.1 VS2013 VC++, Windows 7 VS 2012 VC++ and Ubuntu 13.10 g++-4.8
//Assuming all limitations of XMT C to apply to input program and that it successfully compiles. 
//Minimum Supported Runtime: CUDA 5.5 with some use of deprecated API from CUDA 4.1 (Symbols). Recommended CUDA 6 for full XMT C support.
//Minimum Supported Hardware: CUDA Compute Capability 2.x. Increasing number of threads requires 3.x and more.
//TODO: Further work.
//1. Isolate read only variables in spawn and place them accordingly.
//2. Better error handling on exception exit. Free CUDA mem becomes mandatory. Better verbose
//3. Thread block handing and optimization of cuda execution. Requires prediction or translator flags.
//4. Detect Hardware and optimize accordingly.
//5. Use compiler directives.
//6. Fix pointers -- Comes with CUDA 6 -- Alternatively use Point 8 and 9
//7. Nested Spawns -- Comes with CUDA 6
//8. cudaMallocHost -- Use this on the whole program for advanced data structures. 
//9. cudaDeviceMapHost -- Enable this flag using cudaSetDeviceFlags 
//10. Adding Cilk
//11. Whole application if rethought will give a lot more optimized code.

#include "XMTtoCUDA.cpp"
#include "XMTtoSerial.cpp"
#include "XMTtoCilk.cpp"

//Main Runtime
int main(int argc, char *argv[]) {
	bool flag_CheckErrorHandle = true; //flag added to all operations to look for check cudaerrors.
	bool flag_CheckThread = true; // $ or ThreadID is within Range.
	bool flag_MakeExecutable = false; //To output executable with Source.
	bool flag_InFileSpecified = false; //Infile is specified or not.
	bool flag_OutFileSpecified = false; //Outfile is modified or is assumed default.
	bool flag_Verbose = false; //Enable or disable Verbose Operation.
	OutputType outType = OutputType::CUDA; //Set Default Compile type
	unsigned long maxThreadCap = 1000; //Max number of threads.
	std::string xmtProgram, source, inFile, curArgStr, selfPath, outFile, outProgram;
	IO io;
	SourceManager sourceMgr;
	int curArg = 0;
	size_t temp;
	std::cout << ENDLINE;
	//Argument Processing Begin
	while (curArg < argc) {
		curArgStr = std::string(argv[curArg]);
		curArg++;
		if (curArg == 1) { //Recognizing self path
			selfPath = curArgStr;
			continue;
		}

		if (curArgStr == "-h" || curArgStr == "--help") {
			std::cout << (std::string) "Syntax : XMTtoCUDA [-s] [-n <power>] [-e] [-t] [-x] [-v] infile [-o outfile]" + ENDLINE + ENDLINE
				+ "Options:" + ENDLINE
				+ "infile = Specifiy the input XMT C Program File or Path. (Mandatory)" + ENDLINE
				+ "-s = Translate program to serial C not CUDA. ( -n -e -t get disabled)." + ENDLINE
				+ "-e = Disable error checking. (On by default. Not recommended.)" + ENDLINE
				+ "-t = Disable thread check. (On by default. Program might crash or misbehave.)" + ENDLINE
				+ "-x = Generate Executable. (Optional. Run vsvars32.bat for Win32. Set Environment Vars in others.)" + ENDLINE
				+ "-v = Verbose Operation. (Optional)" + ENDLINE
				+ "-o = Specify Output file optionally. (Optional.)" + ENDLINE
				+ "-n = Maximum number of threads. Expressed as power of 2 (greater than 10)." + ENDLINE
				+ "     Example -n 10 means 2^10 = 1024 Threads." + ENDLINE
				+ "     Default is XMTC 1000 Threads. Change accordingly." + ENDLINE
				+ "     If actual number of threads is less, the extra threads exit safely." + ENDLINE + ENDLINE;
			return 0;
		}
		if (curArgStr == "-s") {
			outType = OutputType::Serial;
			continue;
		}
		if (curArgStr == "-n") {
			maxThreadCap = 2 ^ (atoi(argv[curArg++]));
			continue;
		}
		if (curArgStr == "-e") {
			flag_CheckErrorHandle = false;
			continue;
		}
		if (curArgStr == "-t") {
			flag_CheckThread = false;
			continue;
		}
		if (curArgStr == "-x") {
			flag_MakeExecutable = true;
			continue;
		}
		if (curArgStr == "-v") {
			flag_Verbose = true;
			continue;
		}
		if (curArgStr == "-o") {
			outFile = std::string(argv[curArg++]);
			flag_OutFileSpecified = true;
			continue;
		}
		if (curArgStr[0] == '-') {
			io.Error("Invalid Parameter.");
			return 0;
		}
		temp = curArgStr.length();
		if ((curArgStr[temp - 2] == '.') && (curArgStr[temp - 1] == 'c')) {
			inFile = curArgStr;
			flag_InFileSpecified = true;
		}
	}
	if (!flag_InFileSpecified) {
		io.Error("XMTtoCUDA Fatal: No input file specified.");
	}
	if (!flag_OutFileSpecified) {
		if (outType == OutputType::Serial)
			outFile = inFile.substr(0, inFile.find(".c")) + "-serial.c";
		else
			outFile = inFile + "u";
	}
	if (flag_Verbose){
		io.flagVerbose = true;
	}
	else {
		io.flagVerbose = false;
	}
	//Argument Processing Finish

	//Program Processing Begin
	xmtProgram = io.ReadProgramFile(inFile);
	if (flag_Verbose) std::cout << "Beginning to parse the XMT Program." << ENDLINE;

	//Preprocessing - Recursively adding source in current directory.
	if (flag_Verbose) std::cout << "Starting to parse for local headers." << ENDLINE;
	do {
		source = xmtProgram;
		xmtProgram = sourceMgr.AppendProgramHeader(source);
	} while (source != xmtProgram);
	if (flag_Verbose) std::cout << "Finished parsing for local headers." << ENDLINE;

	if (flag_Verbose) std::cout << "Beginning to translate XMT to CUDA" << ENDLINE;

	//Process Start
	if (outType == OutputType::Serial) {
		XMTtoSerial converter(xmtProgram);
		converter.manageHeaders();
		converter.replaceSpawnWithForLoop();
		outProgram = converter.getSerialCode();
	}
	else {
		XMTtoCUDA converter(xmtProgram);
		if (flag_Verbose)
			converter.enableVerbose();
		else
			converter.disableVerbose();
		converter.setNumOfCUDAThreads(maxThreadCap);
		if (flag_CheckErrorHandle)
			converter.enableCUDAErrorHandling();
		else
			converter.disableCUDAErrorHandling();
		if (flag_CheckThread)
			converter.enableCUDAKernelThreadCheck();
		else
			converter.disableCUDAKernelThreadCheck();
		converter.removeXMTheaders();
		converter.addCUDADetect();
		converter.spawntoKernels();
		outProgram = converter.getCUDAProgram();
	}
	//Process End

	if (flag_Verbose) std::cout << "Finished translation successfully!" << ENDLINE;

	//TODO: Tidying the source.
	outProgram = sourceMgr.normalizeLineEndings(outProgram);
	io.WriteProgram(outFile, outProgram);
	if (flag_MakeExecutable) {
		io.GenerateExecutable(outFile, outType);
	}
	if (flag_Verbose) std::cout << "Program has successfully terminated." << ENDLINE;
	return 0;
}