#include "SourceManager.cpp"

//Class to manage CUDA GPU
class ManageCUDAGPU {
private:
	SourceManager sourceMgr;
public:
	bool flagDeviceMapHost;
	bool flagDeviceScheduleSpin;
	bool flagDeviceScheduleYield;
	bool flagDeviceScheduleBlockingSync;

	ManageCUDAGPU() {
		flagDeviceMapHost = false;
		flagDeviceScheduleSpin = false;
		flagDeviceScheduleYield = false;
		flagDeviceScheduleBlockingSync = false;
	}
	std::string initializeCUDA() {
		std::string code = "";
		return code;
	}

	std::string showCUDAError(std::string message) {
		return (std::string) "if (cudaSuccess != cudaGetLastError()) { fprintf(stderr, \"" + message + "\"); exit(-1); }" + ENDLINE;;
	}

	std::string setCUDAdevice(int i = 0) {
		return (std::string) "cudaSetDevice(" + sourceMgr.intToString(i) + ");" + ENDLINE;
	}

	std::string getDeviceFlags() {
		std::string code = "cudaSetDeviceFlags(0";
		if (flagDeviceMapHost) code += " + cudaDeviceMapHost";
		if (flagDeviceScheduleSpin) code += " + cudaDeviceScheduleSpin";
		if (flagDeviceScheduleYield) code += " + cudaDeviceScheduleYield";
		if (flagDeviceScheduleBlockingSync) code += " + cudaDeviceScheduleBlockingSync";
		code += (std::string) ");" + ENDLINE;
		return code;
	}
};

//Handle all operations for the CUDA kernel generated.
class CUDAKernel {
private:
	//SourceManager object to access functions.
	SourceManager sourceMgr;

	//Flag to add check for errors to all CUDA operations.
	bool flagCheckErrorHandle;

	//$ or ThreadID is within Specified Range, a safe check.
	bool flagCheckThread;

	//Flag for verbose mode.
	bool flagVerbose;

	//Number of threads.
	unsigned long nofThreads;

	//Variables to be changed.
	std::string deferredVarChange;

	//Holds PreKernel Definition operations
	std::string preKernelDefOps;

	//Holds Kernel Memory operations;
	std::string kernelMemOps;

	//Holds kernel block.
	std::string kernelBody;

	//Holds parameters of definition of kernel.
	std::string memoryDefinition;

	//Holds parameters of call of the kernel.
	std::string memoryCall;

	//Holds operations to be performed before the call to kernel.
	std::string preOps;

	//Holds operations to be performed after the kernel call.
	std::string postOps;

	//No to identify each kernel uniquely.
	int countCUDAKernel;

	//No of threads in kernel.
	std::string numberofThreadsSpecifiedStr;

	//No of members in memory definition currently.
	int memDefMembers;

	//No of members in memory call currently.
	int memCallMembers;

public:
	//Constructor
	CUDAKernel() {
		preKernelDefOps = "";
		kernelMemOps = "";
		kernelBody = "";
		memoryDefinition = "";
		memoryCall = "";
		preOps = "";
		postOps = "";
		deferredVarChange = "";
		countCUDAKernel = 0;
		memCallMembers = 0;
		memDefMembers = 0;
		flagCheckErrorHandle = true;
		flagCheckThread = true;
		flagVerbose = false;
		nofThreads = 1000;
	}

	//Set number of threads.
	void setNumOfTCUDAThreads(unsigned long numberOfThreads) {
		nofThreads = numberOfThreads;
	}
	//Enable verbose.
	void enableVerbose() {
		flagVerbose = true;
	}

	//Disable verbose.
	void disableVerbose() {
		flagVerbose = false;
	}

	//Enable Error handling to all cuda functions.
	void enableCUDAErrorHandling() {
		flagCheckErrorHandle = true;
	}

	//Disable Error handling to all cuda functions.
	void disableCUDAErrorHandling() {
		flagCheckErrorHandle = false;
	}

	//Enable thread check in CUDA kernels.
	void enableCUDAKernelThreadCheck() {
		flagCheckThread = true;
	}

	//Disable thread check in CUDA kernels.
	void disableCUDAKernelThreadCheck() {
		flagCheckThread = false;
	}

	//Add operations before kernel definition.
	void appendPreKernelDefOps(std::string Operation) {
		preKernelDefOps += Operation;
		if (flagVerbose) std::cout << (std::string) "Appended Pre Kernel Definition Operation : " + ENDLINE + Operation + ENDLINE;
	}

	//Add memory operations to kernel.
	void appendMemOpstoKernel(std::string Operation) {
		kernelMemOps += Operation;
		if (flagVerbose) std::cout << (std::string) "Appended Kernel Memory Operation : " + ENDLINE + Operation + ENDLINE;
	}

	//Set Kernel Body.
	void setKernelBody(std::string inputKernelBody) {
		kernelBody = inputKernelBody;
		if (flagVerbose) std::cout << (std::string) "Kernel Body Set." + ENDLINE;
	}

	//Add to Memory of Kernel Definition.
	void appendMemoryDefinition(std::string memDef) {
		if (memDefMembers++ > 0) {
			memoryDefinition += ", " + memDef;
		}
		else {
			memoryDefinition = memDef;
		}
		if (flagVerbose) std::cout << (std::string) "Appended Kernel Memory Definition : " + ENDLINE + memDef + ENDLINE;
	}

	//Add to Memory of Kernel Call.
	void appendMemoryCall(std::string memCall) {
		if (memCallMembers++ == 0) {
			memoryCall = memCall;
		}
		else {
			memoryCall += ", " + memCall;
		}
		if (flagVerbose) std::cout << (std::string) "Appended Memory to Kernel Call : " + ENDLINE + memCall + ENDLINE;
	}

	//Add to variable name change, deferred for later.
	void appendVarNameChange(std::string originalVarName, std::string newVarName) {
		deferredVarChange += (std::string) originalVarName + ">" + newVarName + ";";
	}

	//Replace kernel global variables with device ones.
	void performDeferredVarNameChange() {
		std::string originalVarName, newVarName;
		size_t pos = 0, len = deferredVarChange.length();
		while (pos < len) {
			originalVarName = newVarName = "";
			while (deferredVarChange[pos] != '>') {
				originalVarName += deferredVarChange[pos];
				pos++;
			}
			pos++;
			while (deferredVarChange[pos] != ';') {
				newVarName += deferredVarChange[pos];
				pos++;
			}
			kernelBody = sourceMgr.replaceEverywhere(kernelBody, originalVarName, newVarName);
			if (flagVerbose) std::cout << (std::string) "Replaced everywhere in kernel from \"" + originalVarName
				+ "\" with \"" + newVarName + "\"" + ENDLINE;
			pos++;
		}
	}

	//Set number of threads of CUDA Kernel.
	void setNumberOfThreads(std::string NumberofThreads) {
		numberofThreadsSpecifiedStr = NumberofThreads;
		if (flagVerbose) std::cout << (std::string) "Set number of threads to : " + NumberofThreads + ENDLINE;
	}

	//Add Operations before the call to kernel.
	void appendPreCallOperations(std::string ops) {
		preOps += ops;
		if (flagVerbose) std::cout << (std::string) "Appended Pre Kernel Call Operation : " + ENDLINE + ops + ENDLINE;
	}

	//Add Operations after the kernel call.
	void appendPostCallOperations(std::string ops) {
		postOps += ops;
		if (flagVerbose) std::cout << (std::string) "Appended Post Kernel Call Operation : " + ENDLINE + ops + ENDLINE;
	}

	//Set CUDA Kernel no.
	void setCUDAKernelNo(int CUDAKernelNo) {
		countCUDAKernel = CUDAKernelNo;
		if (flagVerbose) std::cout << (std::string) "Set CUDA Kernel No : " + ENDLINE + sourceMgr.intToString(CUDAKernelNo) + ENDLINE;
	}

	//Generate CUDA Kernel String.
	std::string getCUDAKernel() {
		if (flagVerbose) std::cout << (std::string) "Generated CUDA Kernel " + ENDLINE + sourceMgr.intToString(countCUDAKernel) + ENDLINE;
		std::string definition, threadIDdef;
		size_t cur = 0;
		definition = preKernelDefOps;
		definition += "__global__ void spawnCUDAKernel" + sourceMgr.intToString(countCUDAKernel);
		definition += " (" + memoryDefinition + ")" + ENDLINE;
		while (kernelBody[cur] != '{') cur++;
		threadIDdef = (std::string) "int __TID = threadIdx.x + (((gridDim.x * blockIdx.y) + blockIdx.x)*blockDim.x);" + ENDLINE;
		if (flagCheckThread)
			kernelBody.insert(cur + 1, (std::string) ENDLINE + threadIDdef
			+ " if ( __TID > " + numberofThreadsSpecifiedStr + ")  return; " + ENDLINE + kernelMemOps);
		else
			kernelBody.insert(cur + 1, ENDLINE + threadIDdef + kernelMemOps);
		performDeferredVarNameChange();
		return definition + kernelBody;
	}

	//Generate CUDA Kernel Calls.
	std::string getCUDAcall() {
		if (flagVerbose) std::cout << (std::string) "Generated CUDA Kernel Call : " + ENDLINE + sourceMgr.intToString(countCUDAKernel) + ENDLINE;
		std::string call, count;
		count = sourceMgr.intToString(countCUDAKernel);
		call = preOps;
		if (nofThreads < 1024) {
			call += (std::string) "dim3 dimBlock" + count + "( 1000 , 1 ); dim3 dimGrid" + count + "(1, 1);" + ENDLINE;
		}
		else if ((nofThreads / 1024) < 65535){
			call += (std::string) "dim3 dimBlock" + count + "( 1000 , 1 ); dim3 dimGrid" + count + "(" + sourceMgr.intToString(nofThreads / 1024) + "+1, 1);" + ENDLINE;
		}
		else if ((nofThreads / 67107840) < 65535){
			call += (std::string) "dim3 dimBlock" + count + "( 1000 , 1 ); dim3 dimGrid" + count + "(" + sourceMgr.intToString(nofThreads / 67107840) + "+1, 65535);" + ENDLINE;
		}
		else {
			call += (std::string) "dim3 dimBlock" + count + "( 1000 , 1 ); dim3 dimGrid" + count + "(" + sourceMgr.intToString(nofThreads / 67107840) + "+1, 65535);" + ENDLINE;
			std::cout << "Warning use CUDA Compute Capability 3.x or more, or program will crash." << ENDLINE;
		}
		call += (std::string) "spawnCUDAKernel" + count;
		call += (std::string) "<<<dimGrid" + count + ", dimBlock" + count;
		call += ">>>";
		call += " (" + memoryCall + ");" + ENDLINE;
		if (flagCheckErrorHandle) {
			call += (std::string) "if (cudaSuccess != cudaGetLastError()) { fprintf(stderr, \"Kernel launch failed: \"); }" + ENDLINE;
		}
		call += (std::string) "cudaDeviceSynchronize();" + ENDLINE;
		if (flagCheckErrorHandle) {
			call += (std::string) "if (cudaSuccess != cudaGetLastError()) { fprintf(stderr, \"cudaDeviceSynchronize failed!\"); }" + ENDLINE;
		}
		call += postOps;
		return call;
	}
};

//Handles all generic XMT to CUDA handling.
class XMTtoCUDAEngine {
private:
	//Manage CUDA GPU object.
	ManageCUDAGPU manageCudaGpu;

	//SourceManager object.
	SourceManager sourceMgr;

	//Stores location of the last trimmed spawn.
	size_t locationTrimmedSpawn;

	//Internal Object to handle all the operations.
	CUDAKernel kernel;

	//Flag to add check for errors to all CUDA operations.
	bool flagCheckErrorHandle;

	//$ or ThreadID is within Specified Range, a safe check.
	bool flagCheckThread;

	//Flag for verbose
	bool flagVerbose;

	//Has custom malloc been inserted.
	bool flagCustomMallocInserted;

	//Has custom pointer access function be inserted.
	bool flagPointerAccessed;

	//Stores currently modified program.
	std::string modifiedProgram;

	//Stores original XMT program.
	std::string XMTProgram;

	//Stores custom Data types in the program.
	std::string customDataTypes;

	//Count of CUDA kernel generated, to identify them uniquely.
	unsigned int countGeneratedCUDAKernel;

	//Set Number of Threads for CUDA Kernel.
	inline void setNoOfThreads(std::string startThread, std::string endThread) {
		kernel.setNumberOfThreads(endThread + " -  " + startThread + " + 1");
	}

	//Function to generate definition to replace default malloc.
	std::string generateCUDAHostMalloc() {
		std::string function = "";
		size_t i = 0;
		function = (std::string) "void* __CUDAHostMalloc(size_t size) {" + ENDLINE
			+ "void* ptr = NULL;" + ENDLINE //NULL for C99 standards.
			+ "cudaHostAlloc((void**) &ptr, size, cudaHostAllocMapped);" + ENDLINE;
		if (flagCheckErrorHandle) {
			function += (std::string) manageCudaGpu.showCUDAError("CUDA memory allocation on host failed!") + ENDLINE;
		}
		function += (std::string) "return ptr;" + ENDLINE + "}" + ENDLINE;
		return function;
	}

	//Function to generate definition to replace default CUDAmalloc.
	std::string generateCUDADevicemalloc() {
		std::string function = "";
		size_t i = 0;
		function = (std::string) "void* __CUDADeviceMalloc(size_t size) {" + ENDLINE
			+ "void* ptr = NULL;" + ENDLINE //NULL for C99 standards.
			+ "cudaMalloc((void**) &ptr, size);" + ENDLINE;
		if (flagCheckErrorHandle) {
			function += (std::string) manageCudaGpu.showCUDAError("CUDA memory allocation on host failed!") + ENDLINE;
		}
		function += (std::string) "return ptr;" + ENDLINE + "}" + ENDLINE;
		return function;
	}

	//Function to replace default malloc.
	std::string generateEvalMainMemPtr() {
		std::string function = "";
		size_t i = 0;
		function = (std::string) "void* __evalMainMemPtr(void* inputPtr) {" + ENDLINE
			+ "void** ptr = NULL;" + ENDLINE //NULL for C99 standards.
			+ "cudaHostGetDevicePointer(ptr, inputPtr, 0);" + ENDLINE;
		if (flagCheckErrorHandle) {
			function += (std::string) manageCudaGpu.showCUDAError("CUDA access to host memory failed!") + ENDLINE;
		}
		function += (std::string) "return *ptr;" + ENDLINE + "}" + ENDLINE;
		return function;
	}

public:
	//Constructor
	XMTtoCUDAEngine() {
		modifiedProgram = "";
		XMTProgram = "";
		countGeneratedCUDAKernel = -1;
		flagCheckErrorHandle = true;
		flagCheckThread = true;
		flagVerbose = false;
		flagCustomMallocInserted = false;
		flagPointerAccessed = false;
		sourceMgr.findAllDataTypes(XMTProgram);
	}

	//Enable Verbose
	void enableVerbose() {
		flagVerbose = true;
		kernel.enableVerbose();
	}

	//Disable Verbose
	void disableVerbose() {
		flagVerbose = false;
		kernel.disableVerbose();
	}

	//Set Number of CUDA Threads
	void setNumOfCUDAThreads(unsigned long count) {
		kernel.setNumOfTCUDAThreads(count);
	}

	//Enable Error handling to all cuda functions.
	void enableCUDAErrorHandling() {
		flagCheckErrorHandle = true;
		kernel.enableCUDAErrorHandling();
	}

	//Disable Error handling to all cuda functions.
	void disableCUDAErrorHandling() {
		flagCheckErrorHandle = false;
		kernel.disableCUDAErrorHandling();
	}

	//Enable thread check in CUDA kernels.
	void enableCUDAKernelThreadCheck() {
		flagCheckThread = true;
		kernel.enableCUDAKernelThreadCheck();
	}

	//Disable thread check in CUDA kernels.
	void disableCUDAKernelThreadCheck() {
		flagCheckThread = false;
		kernel.disableCUDAKernelThreadCheck();
	}

	//Specify input XMT Program.
	void setXMTProgram(std::string inputProgram) {
		XMTProgram = inputProgram;
		modifiedProgram = inputProgram;
	}

	//Get Modified Program.
	std::string getProgram() {
		return modifiedProgram;
	}

	//Placeholder Function
	void appendToInclude(std::string code) {
		modifiedProgram = sourceMgr.appendToInclude(modifiedProgram, code);
	}

	// Do a custom malloc
	void replaceAllMalloc() {
		if (!flagCustomMallocInserted) {
			flagCustomMallocInserted = true;
			appendToInclude(generateCUDAHostMalloc());
		}
		std::string size;
		size_t pos = 0, len = modifiedProgram.length(), startpos, endpos;
		pos = modifiedProgram.find("malloc", pos);
		while (pos != std::string::npos) {
			size = "";
			startpos = endpos = pos;
			while (modifiedProgram[startpos] != '=') { startpos--; }
			startpos++;
			while (modifiedProgram[endpos] != '(') { endpos++; }
			endpos++;
			while (modifiedProgram[endpos] != ')') {
				size += modifiedProgram[endpos];
				endpos++;
			}
			modifiedProgram.erase(startpos, endpos - startpos + 1);
			modifiedProgram.insert(startpos, (std::string) "__CUDAHostMalloc(" + size + ") ");
			pos = modifiedProgram.find("malloc", pos + 6);
		}
	}

	//TODO: Do a custom calloc
	void replaceAllCalloc() {

	}

	//TODO: Do a custom realloc
	void replaceAllRealloc() {

	}

	//Replace pointer access in spawn.
	std::string replaceMainMemPtrAccess(std::string spawn) {
		size_t cur, savedcur = std::string::npos, start, end;
		std::string tmp = "";
		if (!flagPointerAccessed) {
			flagPointerAccessed = true;
			appendToInclude(generateEvalMainMemPtr());
		}
		cur = spawn.find("->");
		while (cur != std::string::npos) {
			if ((!sourceMgr.isComment(cur, spawn)) && (!sourceMgr.isQuote(cur, spawn))) {
				savedcur = cur;
				tmp = "";
				while (isspace(spawn[cur])) { cur--; }
				while (isalnum(spawn[cur])) {
					tmp = spawn[cur] + tmp;
					cur--;
				}
				spawn.erase(cur, savedcur - cur + 2);
				tmp = (std::string) " __evalMainMemPtr(" + tmp + ")->";
				spawn.insert(cur, tmp);
				cur = spawn.find("->", cur + tmp.size());
			}
			else
				cur = spawn.find("->", cur + 2);
		}
		cur = spawn.find("*");
		while (cur != std::string::npos) {
			if ((!sourceMgr.isComment(cur, spawn)) && (!sourceMgr.isQuote(cur, spawn))) {
				savedcur = start = end = cur;
				tmp = "";
				while (isspace(spawn[start])) { start--; }
				while (isspace(spawn[end])) { end++; }
				if ((sourceMgr.isValidIdentifierChar(spawn[end])) && (ispunct(spawn[start]))) {
					while (sourceMgr.isValidIdentifierChar(spawn[end])) {
						tmp += spawn[end];
						end++;
					}
					spawn.erase(savedcur, end - savedcur + 1);
					spawn.insert(savedcur, (std::string) "*(__evalMainMemPtr(" + tmp + "))");
				}
				cur = spawn.find("*", savedcur + 1);
			}
			else {
				cur = spawn.find("*", cur + 1);
			}
		}
		return spawn;
	}

	//Add string to beginning of Main function block.
	void insertToMainStart(std::string text) {
		size_t locationMainStart = sourceMgr.locateMainStart(modifiedProgram);
		modifiedProgram = modifiedProgram.substr(0, locationMainStart) + text +
			modifiedProgram.substr(locationMainStart, modifiedProgram.length() - locationMainStart);
		if (flagVerbose) std::cout << (std::string) "Appended to Main Block Start." + ENDLINE + text + ENDLINE;
	}

	//Remove the specified header file
	void removeHeaderFile(std::string headerName) {
		modifiedProgram = sourceMgr.removeHeader(modifiedProgram, headerName);
		if (flagVerbose) std::cout << (std::string) "Removed header : " + headerName + ENDLINE;
	}

	//Return total number of Spawns in a XMT program.
	int getSpawnCount() {
		int count = sourceMgr.findCountOfSpecial(modifiedProgram, "spawn");
		return count;
	}

	//Generate memory operations for external variables
	void generateMemoryOperations(std::string variables) {
		bool flagVarArray;
		std::string currentVariable, size, var, type, code, tmp, tmpsize, tmpvar, prevtmpsize;
		size_t cur = 0, pos = 0;
		unsigned int len = variables.length(), ptrcount = 0;
		while (cur < len) {
			flagVarArray = false;
			currentVariable = "";
			while ((variables[cur] != ';') && (cur < len)) { currentVariable += variables[cur++]; }
			cur++;
			pos = sourceMgr.findFirstofVar(modifiedProgram, currentVariable);
			type = sourceMgr.findTypeOf(modifiedProgram, pos);
			while ((!isalpha(modifiedProgram[pos])) && (pos < (modifiedProgram.length()))) { pos++; }
			var = "";
			while ((isalnum(modifiedProgram[pos])) && (pos < (modifiedProgram.length()))) {
				var += modifiedProgram[pos++];
			}
			while (isspace(modifiedProgram[pos])) { pos++; }
			size = "1";
			if (modifiedProgram[pos] == '[') {
				size = "";
				do {
					size += "[";
					pos++;
					if (modifiedProgram[pos] == '[') pos++;
					flagVarArray = true;
					while (!ispunct(modifiedProgram[pos]) && (pos < modifiedProgram.length())) {
						size += modifiedProgram[pos++];
					}
					if (modifiedProgram[pos] == ']') size += ']';
				} while (modifiedProgram[pos] == '[' || modifiedProgram[pos] == ']');
			}
			size.erase(size.length() - 1, 1);
			if (flagVerbose) {
				if (flagVarArray)
					std::cout << (std::string) "Found Array named \"" + var + "\" of Type : " + type + " and of Size : " + size + ENDLINE;
				else
					std::cout << (std::string) "Found Variable named \"" + var + "\" of Type : " + type + ENDLINE;
			}
			if (flagVarArray) {
				//Make pointer and allocate space. Then copy data. Change name of global access variables.
				ptrcount = 0;
				for (size_t i = 0; i<size.length(); i++) {
					if (size[i] == '[') ptrcount++;
				}
				if (ptrcount == 1) {
					code = (std::string)type + "* dev_" + var + "= 0;" + ENDLINE + "cudaMalloc((void**)&dev_" + var + ", " + size.substr(1, size.length() - 2) + " * sizeof(" + type + "));" + ENDLINE;
					if (flagCheckErrorHandle) {
						code += manageCudaGpu.showCUDAError("cudaMalloc for dev_" + var + " failed!");
					}
					code += (std::string)"cudaMemcpy(dev_" + var + ", " + var + ", " + size.substr(1, size.length() - 2) + " * sizeof(" + type + "), cudaMemcpyHostToDevice);" + ENDLINE;
					if (flagCheckErrorHandle) {
						code += manageCudaGpu.showCUDAError("cudaMemcpy from host to device for " + var + " failed!");
					}
				}
				else {
					tmp = "";
					tmpvar = var;
					for (size_t i = 0; i < ptrcount; i++) {
						tmp = "";
						for (size_t j = 0; j < ptrcount - i; j++) tmp += "*";
						for (size_t j = 0, k = 0; j < size.length(); j++) {
							if (size[j] == '[') k++;
							if (k == i + 1) {
								j++;
								tmpsize = "";
								while (size[j] != ']') {
									tmpsize += size[j];
									j++;
								}
								break;
							}
						}
						if (i == 0) {
							code = (std::string) type + tmp + " dev_" + tmpvar + "= 0;" + ENDLINE
								+ "cudaMalloc((void*" + tmp + ")&dev_" + tmpvar + ", " + tmpsize + " * sizeof(" + type + tmp.substr(0, tmp.length() - 1) + "));" + ENDLINE;
							prevtmpsize = tmpsize;
							if (flagCheckErrorHandle) {
								code += manageCudaGpu.showCUDAError("cudaMalloc for dev_" + tmpvar + " failed!");
							}
						}
						else {
							code += (std::string) "for (int __" + (char) ('a' + i - 1) + " = 0; __" + (char) ('a' + i - 1) + " < " + prevtmpsize + "; __" + (char) ('a' + i - 1) + "++) { " + ENDLINE;
							tmpvar += (std::string) "[ __" + (char) ('a' + i - 1) + "]";
							code += (std::string) "cudaMalloc((void*" + tmp + ")&dev_" + tmpvar + ", " + tmpsize + " * sizeof(" + type + tmp.substr(0, tmp.length() - 1) + "));" + ENDLINE;
							prevtmpsize = tmpsize;
							if (flagCheckErrorHandle) {
								code += manageCudaGpu.showCUDAError("cudaMalloc for dev_" + tmpvar + " failed!");
							}
						}
					}
					for (size_t i = 0; i<ptrcount - 1; i++)  {
						code += (std::string) "}" + ENDLINE;
					}
					tmp = "";
					tmpvar = var;
					for (size_t i = 0; i < ptrcount; i++) {
						tmp = "";
						for (size_t j = 0; j < ptrcount - i; j++) tmp += "*";
						for (size_t j = 0, k = 0; j < size.length(); j++) {
							if (size[j] == '[') k++;
							if (k == i) {
								j++;
								tmpsize = "";
								while (size[j] != ']') {
									tmpsize += size[j];
									j++;
								}
								break;
							}
						}
						if (i == 0) {
							code += (std::string)"cudaMemcpy(dev_" + tmpvar + ", " + tmpvar + ", " + tmpsize + " * sizeof(" + type + "), cudaMemcpyHostToDevice);" + ENDLINE;
							prevtmpsize = tmpsize;
							if (flagCheckErrorHandle) {
								code += manageCudaGpu.showCUDAError("cudaMemcpy from host to device for " + var + " failed!");
							}
						}
						else {
							code += (std::string) "for (int __" + (char) ('a' + i - 1) + " = 0; __" + (char) ('a' + i - 1) + " < " + prevtmpsize + "; __" + (char) ('a' + i - 1) + "++) { " + ENDLINE;
							tmpvar += (std::string) "[ __" + (char) ('a' + i - 1) + "]";
							code += (std::string)"cudaMemcpy(dev_" + tmpvar + ", " + tmpvar + ", " + tmpsize + " * sizeof(" + type + "), cudaMemcpyHostToDevice);" + ENDLINE;
							prevtmpsize = tmpsize;
							if (flagCheckErrorHandle) {
								code += manageCudaGpu.showCUDAError("cudaMemcpy from host to device for " + var + " failed!");
							}
						}
					}
					for (size_t i = 0; i<ptrcount - 1; i++)  {
						code += (std::string) "}" + ENDLINE;
					}
				}
				kernel.appendPreCallOperations(code);
				//Copy back results
				if (ptrcount == 1) {
					code = (std::string) "cudaMemcpy(" + var + ", dev_" + var + ", " + size.substr(1, size.length() - 2) + " * sizeof(" + type + "), cudaMemcpyDeviceToHost);" + ENDLINE;
					if (flagCheckErrorHandle) {
						code += manageCudaGpu.showCUDAError("cudaMemcpy from device to host for " + var + " failed!");
					}
				}
				else {
					tmp = "";
					tmpvar = var;
					for (size_t i = 0; i < ptrcount; i++) {
						tmp = "";
						for (size_t j = 0; j < ptrcount - i; j++) tmp += "*";
						for (size_t j = 0, k = 0; j < size.length(); j++) {
							if (size[j] == '[') k++;
							if (k == i) {
								j++;
								tmpsize = "";
								while (size[j] != ']') {
									tmpsize += size[j];
									j++;
								}
								break;
							}
						}
						if (i == 0) {
							code = (std::string)"cudaMemcpy(" + tmpvar + ", dev_" + tmpvar + ", " + tmpsize + " * sizeof(" + type + "), cudaMemcpyDeviceToHost);" + ENDLINE;
							prevtmpsize = tmpsize;
							if (flagCheckErrorHandle) {
								code += manageCudaGpu.showCUDAError("cudaMemcpy from host to device for " + var + " failed!");
							}
						}
						else {
							code += (std::string) "for (int __" + (char) ('a' + i - 1) + " = 0; __" + (char) ('a' + i - 1) + " < " + prevtmpsize + "; __" + (char) ('a' + i - 1) + "++) { " + ENDLINE;
							tmpvar += (std::string) "[ __" + (char) ('a' + i - 1) + "]";
							code += (std::string)"cudaMemcpy(" + tmpvar + ", dev_" + tmpvar + ", " + tmpsize + " * sizeof(" + type + "), cudaMemcpyDeviceToHost);" + ENDLINE;
							prevtmpsize = tmpsize;
							if (flagCheckErrorHandle) {
								code += manageCudaGpu.showCUDAError("cudaMemcpy from host to device for " + var + " failed!");
							}
						}
					}
					for (size_t i = 0; i<ptrcount - 1; i++)  {
						code += (std::string) "}" + ENDLINE;
					}
				}
				//Free Device memory.
				code += (std::string)"cudaFree(dev_" + var + "); " + ENDLINE;
				kernel.appendPostCallOperations(code);
				//Append variable to kernel call.
				kernel.appendMemoryCall("dev_" + var);
				//Append variable to kernel definition.
				code = type;
				for (size_t i = 0; i<ptrcount; i++) code += "*";
				code += (std::string) " " + var;
				kernel.appendMemoryDefinition(code);
			}
			else {
				//TODO: test this
				//Append original variable to kernel call.
				kernel.appendMemoryCall((std::string) "&" + var);
				//Append original variable to kernel definition.
				kernel.appendMemoryDefinition(type + "*dev_" + var);
				//Make pointer and allocate space. Then copy data.
				code = (std::string)type + "*dev_" + var + "= 0;" + ENDLINE + "cudaMalloc((void**)&dev_" + var + ", sizeof(" + type + "));" + ENDLINE;
				if (flagCheckErrorHandle) {
					code += manageCudaGpu.showCUDAError("cudaMalloc for dev_" + var + " failed!");
				}
				code += (std::string)"cudaMemcpy(dev_" + var + ", &" + var + ", sizeof(" + type + "), cudaMemcpyHostToDevice);" + ENDLINE;
				if (flagCheckErrorHandle) {
					code += manageCudaGpu.showCUDAError("cudaMemcpy from host to device for " + var + " failed!");
				}
				kernel.appendPreCallOperations(code);
				//Replace original variable with device variable everywhere in the kernel.
				kernel.appendVarNameChange(var, "*dev_" + var);
				//Copy back results
				code = (std::string)"cudaMemcpy(&" + var + ", dev_" + var + ", sizeof(" + type + "), cudaMemcpyDeviceToHost);" + ENDLINE;
				if (flagCheckErrorHandle) {
					code += manageCudaGpu.showCUDAError("cudaMemcpy from device to host for " + var + " failed!");
				}
				//Free Device memory.
				code += (std::string)"cudaFree(dev_" + var + "); " + ENDLINE;
				kernel.appendPostCallOperations(code);
			}
		}
		if (flagVerbose) std::cout << (std::string) "Memory Operations Performed." + ENDLINE;
	}

	//Replace XMT C style thread ID $ with CUDA style threadIDx
	std::string replaceThreadID(std::string spawn) {
		size_t cur = 0;
		while (spawn[cur] != '(') { cur++; } cur++;
		while (isspace(spawn[cur])) { cur++; }
		std::string startThreadString = "", endThreadString = "";
		while (isalnum(spawn[cur])) { startThreadString += spawn[cur++]; }
		while (!isalnum(spawn[cur])) { cur++; }
		while (isalnum(spawn[cur])) { endThreadString += spawn[cur++]; }
		setNoOfThreads(startThreadString, endThreadString);
		while (cur < spawn.size()) {
			if (spawn[cur] == '$') {
				if (!sourceMgr.isComment(cur, spawn) && !sourceMgr.isQuote(cur, spawn)) {
					spawn[cur] = '(';
					spawn.insert(cur + 1, " " + startThreadString + " + __TID ) ");
				}
			}
			cur++;
		}
		//Append variables for kernel call.
		kernel.appendMemoryCall(startThreadString);
		//Append variables for kernel definition.
		kernel.appendMemoryDefinition("int " + startThreadString);
		//Append before the kernel definition
		kernel.appendPreKernelDefOps("int " + startThreadString + ";" + ENDLINE);
		//Append variables for kernel call.
		kernel.appendMemoryCall(endThreadString);
		//Append variables for kernel definition.
		kernel.appendMemoryDefinition("int " + endThreadString);
		//Append before the kernel definition
		kernel.appendPreKernelDefOps("int " + endThreadString + ";" + ENDLINE);
		if (flagVerbose) std::cout << (std::string) "$ has been replaced with ThreadID in spawn block." + ENDLINE;
		return spawn;
	}

	//Generate CUDA Kernel Definition 
	std::string generateCUDAKernelDefinition(std::string spawn) {
		size_t cur = 0;
		countGeneratedCUDAKernel++;
		std::string globalReads, modSpawn;
		kernel.setCUDAKernelNo(countGeneratedCUDAKernel);
		globalReads = sourceMgr.searchGlobalReads(modifiedProgram, spawn, locationTrimmedSpawn);
		generateMemoryOperations(globalReads);
		modSpawn = replaceThreadID(spawn);
		while (modSpawn[cur] != '{') { cur++; }
		modSpawn.erase(0, cur);
		kernel.setKernelBody(modSpawn);
		if (flagVerbose) std::cout << (std::string) "CUDA Kernel Definition Generated." + ENDLINE;
		return kernel.getCUDAKernel();
	}

	//TODO: Pointer access in kernel block
	void modifyAllPointerAccess() {

	}

	//Generate CUDA Kernel Call.
	std::string generateCUDAKernelCall() {
		if (flagVerbose) std::cout << (std::string) "CUDA Kernel Call Generated." + ENDLINE;
		return kernel.getCUDAcall();
	}

	//Insert CUDA Kernel Definition
	void insertCUDAKernelDefinition(std::string kernelDefinition) {
		kernelDefinition = replaceMainMemPtrAccess(kernelDefinition);
		modifiedProgram.insert(sourceMgr.locateMainBeg(modifiedProgram), kernelDefinition);
		if (flagVerbose) std::cout << (std::string) "CUDA Kernel Definition inserted before main()." + ENDLINE;
	}

	//Remove specified Spawn from the program and return it as a string.
	std::string trimSpawn(int count = 1) {
		std::string spawn = "";
		size_t start = sourceMgr.searchSpawnStart(modifiedProgram, count);
		size_t end = sourceMgr.searchSpawnEnd(modifiedProgram, start);
		spawn = modifiedProgram.substr(start, end - start);
		modifiedProgram.erase(start, end - start);
		locationTrimmedSpawn = start;
		if (flagVerbose) std::cout << (std::string) sourceMgr.intToString(count) + "Spawn block Trimmed." + ENDLINE;
		return spawn;
	}

	//Insert CUDA Kernel Call
	void insertCUDAKernelCall(std::string kernelCall) {
		modifiedProgram.insert(locationTrimmedSpawn, kernelCall);
		if (flagVerbose) std::cout << (std::string) "CUDA Kernel Definition inserted in main()." + ENDLINE;
	}

	//TODO: Phase 2 - Requires CUDA 6
	void searchSspwan(std::string spawn) {
		//search and replace sspawn with i.e. unrolling or a new cuda kernel execution.
	}

	void searchPs(std::string spawn) {
		//replace this with atomic variables. copy them locally and then copy them back after execution.
		//ps(int local_integer, psBaseReg ps_base);
		//Add the value of the local integer to the second parameter (ps base for ps, memory location for psm).
	}

	void searchPsm(std::string spawn) {
		//psm(int local_integer, int variable);
		//Copy the old value of the second parameter to the local integer

		//Append device variable declaration before the kernel definition
		//kernel.appendPreKernelDefOps("__device__ int dev_" + var + ";" + ENDLINE);
		//Copy value of original variable from kernel call to device variable.
		//kernel.appendMemOpstoKernel("dev_" + var + " = " + var + ";" + ENDLINE);
		//Copy back final value of device variable from the kernel post execution back to the original value.
		//kernel.appendPostCallOperations("cudaMemcpyFromSymbol(&" + var + ", \"dev_" + var + "\", sizeof(" + var + "), 0, cudaMemcpyDeviceToHost);" + ENDLINE);
	}
};

//This class handles specific coding.
class XMTtoCUDA {
private:
	//Flag to add check for errors to all CUDA operations.
	bool flagCheckErrorHandle;

	//$ or ThreadID is within Specified Range, a safe check.
	bool flagCheckThread;

	//Flag for verbose.
	bool flagVerbose;

	//Object of the base class.
	XMTtoCUDAEngine engine;

	//Object of class Source Manager.
	SourceManager sourceMgr;

	//Object of class Manage CUDA GPU
	ManageCUDAGPU manageCudaGpu;

public:
	//Constructor
	XMTtoCUDA(std::string XMTProgram) {
		engine.setXMTProgram(XMTProgram);
		flagCheckErrorHandle = true;
		flagCheckThread = true;
		flagVerbose = false;
	}

	//Enable Verbose.
	void enableVerbose() {
		flagVerbose = true;
		engine.enableVerbose();
	}

	//Disable Verbose.
	void disableVerbose() {
		flagVerbose = false;
		engine.disableVerbose();
	}

	//Set number of CUDA Threads 
	void setNumOfCUDAThreads(unsigned long count) {
		engine.setNumOfCUDAThreads(count);
	}

	//Enable Error handling to all cuda functions.
	void enableCUDAErrorHandling() {
		flagCheckErrorHandle = true;
		engine.enableCUDAErrorHandling();
	}

	//Disable Error handling to all cuda functions.
	void disableCUDAErrorHandling() {
		flagCheckErrorHandle = false;
		engine.disableCUDAErrorHandling();
	}

	//Enable thread check in CUDA kernels.
	void enableCUDAKernelThreadCheck() {
		flagCheckThread = true;
		engine.enableCUDAKernelThreadCheck();
	}

	//Disable thread check in CUDA kernels.
	void disableCUDAKernelThreadCheck() {
		flagCheckThread = false;
		engine.disableCUDAKernelThreadCheck();
	}

	//Remove Generic XMT Headers.
	void removeXMTheaders() {
		engine.removeHeaderFile("xmtc.h");
		if (flagVerbose) std::cout << (std::string) "XMT Header removed." + ENDLINE;
	}

	//Add code to detect CUDA card and features.
	void addCUDADetect() {
		engine.appendToInclude((std::string) "#include \"cuda_runtime.h\"" + ENDLINE);
		engine.appendToInclude((std::string) "#include \"device_launch_parameters.h\"" + ENDLINE);
		engine.appendToInclude((std::string) "#include <stdio.h>" + ENDLINE);
		engine.appendToInclude((std::string) "#include <stdlib.h>" + ENDLINE);
		if (flagCheckErrorHandle) {
			engine.insertToMainStart(manageCudaGpu.showCUDAError(" cudaSetDevice failed! Is GPU busy? "));
		}
		engine.insertToMainStart((std::string) ENDLINE + manageCudaGpu.setCUDAdevice()); //Added later because the new statement is inserted at top of the previous one.
		if (flagCheckErrorHandle) {
			engine.insertToMainStart(manageCudaGpu.showCUDAError(" cudaSetDeviceFlag failed! Do you have a CUDA capable GPU installed? "));
		}
		manageCudaGpu.flagDeviceMapHost = manageCudaGpu.flagDeviceScheduleSpin = true;
		engine.insertToMainStart((std::string) ENDLINE + manageCudaGpu.getDeviceFlags());
		if (flagVerbose) std::cout << (std::string) "Added CUDA Detect to the program." + ENDLINE;
	}

	//Change spawn code to CUDA code.
	void spawntoKernels() {
		int count = engine.getSpawnCount(), cur = 1;
		std::string spawn, kernel;
		while (cur <= count) {
			spawn = engine.trimSpawn(cur);
			std::string kernelDefinition = engine.generateCUDAKernelDefinition(spawn);
			std::string kernelCall = engine.generateCUDAKernelCall();
			//Sequence of following inserts matters
			engine.insertCUDAKernelCall(kernelCall);
			engine.insertCUDAKernelDefinition(kernelDefinition);
			cur++;
		}
		engine.replaceAllMalloc();
		engine.replaceAllCalloc();
		engine.replaceAllRealloc();
		if (flagVerbose) std::cout << (std::string) "Spawns have been transformed to Kernels." + ENDLINE;
	}

	//Get CUDA program as string.
	std::string getCUDAProgram() {
		if (flagVerbose) std::cout << (std::string) "CUDA Program Generated." + ENDLINE;
		return engine.getProgram();
	}
};
