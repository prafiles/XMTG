#ifndef SourceManager_CPP_ //Preventing multi-inclusion
#define SourceManager_CPP_

#include "IO.cpp" //To get all headers and IO Operations

//Class to process output source optionally
class SourceManager {
private:
	//Object to access class IO
	IO io;

	//Vector to store all data types (Custom)
	std::vector <std::string> customDataTypes;

	//Vector to store all data types (Defined)
	std::vector <std::string> definedDataTypes;

public:
	bool flagVerbose;
	//Constructor
	SourceManager() {
		flagVerbose = false;
		definedDataTypes.push_back("int");
		definedDataTypes.push_back("long");
		definedDataTypes.push_back("char");
		definedDataTypes.push_back("float");
		definedDataTypes.push_back("double");
		definedDataTypes.push_back("signed");
		definedDataTypes.push_back("unsigned");
	}

	//TODO: Remove all comments from the program source.
	std::string removeComments(std::string source) {
		size_t cur = 0, tmpcur = 0, len = source.length();
		while (cur < len - 1) {
			if ((source[cur] == '/') && (source[cur + 1] == '/')) {
				tmpcur = cur;
				while ((source[tmpcur] != '\n') && (tmpcur < len)) { tmpcur++; }
				source.erase(cur, tmpcur - cur);
				len = source.length();
			}
			cur++;
		}
		cur = 0;
		while (cur < len - 1) {
			if ((source[cur] == '/') && (source[cur + 1] == '*')) {
				tmpcur = cur;
				while ((source[tmpcur] == '*') && (source[tmpcur + 1] == '/') && (tmpcur < len - 1)) { tmpcur++; }
				source.erase(cur, tmpcur - cur);
				len = source.length();
			}
			cur++;
		}
		return source;
	}

	//TODO: Remove of all space in the program.
	std::string removeSpace(std::string source) {
		size_t cur = 0, tmpcur = 0, len = source.length();
		while (cur < len) {
			if (isQuote(cur, source)) { cur++; continue; }
			if (source[cur] == '#') {
				while ((source[cur] != '\n') && (cur < len - 1)) { cur++; continue; }
			}
			if (isspace(source[cur])) {
				source[cur] = ' ';
				tmpcur = cur;
				while ((isspace(source[tmpcur])) && (tmpcur < len - 1)) { tmpcur++; }
				source.erase(cur, tmpcur - cur);
				len = source.length();
			}
			cur++;
		}
		return source;
	}

	//TODO: Reformat given source.
	std::string formatSource(std::string source) {
		return source;
	}

	//Find all the custom data types
	void findAllDataTypes(std::string source) {
		size_t pos = 0, len = source.length();
		std::string type = "";
		pos = source.find("struct", pos);
		while (pos != std::string::npos) {
			while (!isspace(source[pos])) { pos++; }
			while (isspace(source[pos])) { pos++; }
			if (source[pos] == '{') {
				pos = blockEnd(pos - 5, source);
				while (source[pos] != '}') { pos++; }
				while (isspace(source[pos])) { pos++; }
				while (isalnum(source[pos])) {
					type += source[pos];
					pos++;
				}
			}
			else {
				while (!isalnum(source[pos])) {
					type += source[pos];
					pos++;
				}
			}
			customDataTypes.push_back(type);
			type = "";
			pos++;
			pos = source.find("struct", pos);
		}
		pos = 0;
		pos = source.find("union", pos);
		while (pos != std::string::npos) {
			while (!isspace(source[pos])) { pos++; }
			while (isspace(source[pos])) { pos++; }
			if (source[pos] == '{') {
				pos = blockEnd(pos - 5, source);
				while (source[pos] != '}') { pos++; }
				while (isspace(source[pos])) { pos++; }
				while (isalnum(source[pos])) {
					type += source[pos];
					pos++;
				}
			}
			else {
				while (!isalnum(source[pos])) {
					type += source[pos];
					pos++;
				}
			}
			customDataTypes.push_back(type);
			type = "";
			pos++;
			pos = source.find("union", pos);
		}
	}

	std::string normalizeLineEndings(std::string source) {
		size_t pos = 0;
		pos = source.find("\r");
		while (pos != std::string::npos) {
			source.erase(pos, 1);
			pos = source.find("\r");
		}
		pos = 0;
		pos = source.find("\n", pos);
		while (pos != std::string::npos) {
			source.erase(pos, 1);
			source.insert(pos, ENDLINE);
			pos = source.find("\n", pos + 2);
		}
		return source;
	}

	//Check whether current position is a comment.
	bool isComment(size_t position, std::string source) {
		bool flag = false;
		size_t cur = position;
		while ((source[cur] != '\n') && (cur > 0)) {
			if ((source[cur] == '/') && (source[cur + 1] == '/')) { return true; }
			cur--;
		}
		cur = 0;
		while (cur < position) {
			if ((source[cur] == '/') && (source[cur + 1] == '*') && (flag == false)) {
				flag = true;
			}
			if ((source[cur] == '*') && (source[cur + 1] == '/') && (flag == true)) {
				flag = false;
			}
			cur++;
		}
		return flag;
	}

	//Check whether current position is in a qoute.
	bool isQuote(size_t position, std::string source) {
		bool flag = false;
		if (position < 1) return flag;
		size_t cur = position;
		while ((source[--cur] != '\n') && (cur > 0)) {
			if (((source[cur] == '\'') || (source[cur] == '\"')) && (source[cur - 1] != '\\')) { flag = !flag; }
		}
		return flag;
	}

	//Find end of the specified block.
	size_t blockEnd(size_t blockStart, std::string source) {
		int blockCount = 1;
		size_t cur = blockStart;
		while (source[cur] != '{') { cur++; }
		cur++;
		while (blockCount != 0) {
			if (source[cur] == '{') blockCount++;
			if (source[cur] == '}') blockCount--;
			cur++;
		}
		return cur;
	}

	//Sort and Remove Duplicates in a Vector list.
	template <typename T> static void removeDuplicates(std::vector<T>& vec)
	{
		std::sort(vec.begin(), vec.end());
		vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
	}

	//Replace a token everywhere in source.
	std::string replaceEverywhere(std::string source, std::string originalString, std::string newString) {
		size_t start_pos = 0, step = originalString.length();
		while ((start_pos = source.find(originalString, start_pos)) != std::string::npos) {
			if (isComment(start_pos, source)) { start_pos += step; continue; }
			if (isQuote(start_pos, source)) { start_pos += step; continue; }
			if (isalnum(source[start_pos - 1])) { start_pos += step; continue; }
			if (isalnum(source[start_pos + step])) { start_pos += step; continue; }
			source.replace(start_pos, originalString.length(), newString);
			start_pos += newString.length();
		}
		return source;
	}

	//To convert int to std::string
	std::string intToString(int x) {
		std::string count;
		std::stringstream countStream;
		countStream << x;
		count = countStream.str();
		return count;
	}

	//Remove header from the given program
	std::string removeHeader(std::string inputProgram, std::string headerName) {
		int location = inputProgram.find(headerName);
		int start = location, end = location;
		while (inputProgram[--start] != '#');
		while (inputProgram[++end] != '\n');
		inputProgram.erase(start, end - start);
		return inputProgram;
	}

	//Find Variables, Functions or Keywords
	size_t findLocationOfSpecial(std::string code, std::string toFind, int count = 1, size_t offset = 0) {
		size_t loc = offset, saveloc = -1, len = toFind.length();
		int cur = 0;
		loc = code.find(toFind, loc);
		while ((loc != std::string::npos) && (cur < count)) {
			if ((!isComment(loc, code)) && (!isQuote(loc, code))) {
				if (!isalnum(code[loc - 1]) && !isalnum(code[loc + len])) {
					cur++;
					saveloc = loc;
				}
			}
			loc = code.find(toFind, loc + len);
		}
		return saveloc;
	}

	//Find count of occurence of a Function, Variable or Keyword within the given code.
	int findCountOfSpecial(std::string code, std::string toFind) {
		size_t loc = 0, len = code.length();
		int count = 0;
		while (loc < len) {
			loc = code.find(toFind, loc);
			if (loc == std::string::npos) { break; }
			if (isComment(loc, code) || isQuote(loc, code)) { loc++; continue; }
			if (!isalnum(code[loc - 1]) && !isalnum(code[loc + toFind.length()])) { count++; }
			loc++;
		}
		return count;
	}

	//Append custom header files to the program in current directory.
	std::string AppendProgramHeader(std::string program) {
		std::string headerSource = "", headerName = "", originalProgram = program;
		std::ifstream testFileObj;
		bool flagIncludeType, flagFileChanged = false;
		size_t len = program.length(), cur = 0, savedCur = 0;
		while (cur < len) {
			if (program[cur] == '#'){
				savedCur = cur;
				while (!isalpha(program[++cur]));
				if (program.substr(cur, 7) == "include") {
					flagIncludeType = true; cur += 7;
					while (program[cur] != '\"') {
						if (program[cur] == '<') { flagIncludeType = false; break; }
						if (program[cur] == '\n') { flagIncludeType = false; break; }
						cur++;
					}
					if (flagIncludeType) {
						cur++; headerName = "";
						while (program[cur] != '\"') {
							headerName += program[cur++];
						}
						if (flagVerbose) std::cout << (std::string) "Found headerfile : " + headerName + ENDLINE;
						testFileObj.open(headerName.c_str());
						if (testFileObj.is_open()) {
							testFileObj.close();
							headerSource = io.ReadProgramFile(headerName);
							headerSource = replaceEverywhere(headerSource, "extern", "");
							program.erase(savedCur, cur - savedCur + 1);
							program.insert(savedCur, headerSource);
							flagFileChanged = true;
							if (flagVerbose) std::cout << (std::string) "Read headerfile : " + headerName + ENDLINE;
						}
						else {
							if (flagVerbose) std::cout << (std::string) "Not available locally: " + headerName + ENDLINE;
							testFileObj.close();
						}
					}
				}
			}
			cur++;
		}
		if (flagFileChanged)
			return program;
		else
			return originalProgram;
	}

	//Append string to end of Include section of the program.
	std::string appendToInclude(std::string source, std::string codeToInsert) {
		size_t pos = source.find("#"), saveloc = std::string::npos;
		while (pos != std::string::npos) {
			if (isComment(pos, source) || isQuote(pos, source)) {
				pos++;
				continue;
			}
			pos++;
			while (isspace(source[pos])) { pos++; }
			if (source.substr(pos, 7) == std::string("include")) saveloc = pos;
			pos = source.find("#", pos);
		}
		if (saveloc == std::string::npos) saveloc = 0;
		while (source[saveloc] != '\n') { saveloc++; }
		source.insert(saveloc + 1, codeToInsert);
		if (flagVerbose) std::cout << (std::string) "Appended to Include :" + ENDLINE + codeToInsert + ENDLINE;
		return source;
	}

	//Locate Beginning of Main function.
	inline size_t locateMainBeg(std::string code) {
		bool flagFound = false;
		size_t cur = 0, locationMainBeg;
		while (!flagFound) {
			locationMainBeg = cur = code.find("main");
			if (isspace(code[cur - 1])) {
				cur += 4;
				while (isspace(code[cur])) { cur++; }
				if (code[cur] == '('&& !isComment(cur, code) && !isQuote(cur, code)) {
					flagFound = true;
				}
			}
			cur += 4;
		}
		while (locationMainBeg > 0) {
			if (code[locationMainBeg] == '\n') break;
			if (code[locationMainBeg] == '}') break;
			if (code[locationMainBeg] == ';') break;
			locationMainBeg--;
		}
		if (flagVerbose) std::cout << (std::string) "Main Beginning located." + ENDLINE;
		return locationMainBeg;
	}

	//Locate Start of Main block.
	inline size_t locateMainStart(std::string code) {
		bool flagFound = false;
		size_t cur = 0, locationMainStart;
		while (!flagFound) {
			cur = code.find("main", cur);
			if (isspace(code[cur - 1])) {
				cur += 4;
				while (isspace(code[cur])) { cur++; }
				if (code[cur] == '('&& !isComment(cur, code) && !isQuote(cur, code)) {
					flagFound = true;
					locationMainStart = code.find('{', cur);
					locationMainStart++;
				}
			}
			cur += 4;
		}
		if (flagVerbose) std::cout << (std::string) "Main Block start located." + ENDLINE;
		return locationMainStart;
	}

	//Locate End of Main block.
	inline size_t locateMainEnd(std::string code) {
		return blockEnd(locateMainStart(code), code);
		if (flagVerbose) std::cout << (std::string) "Main Block end located." + ENDLINE;
	}

	//Locate Start of Specified Spawn.
	inline size_t searchSpawnStart(std::string code, int count, size_t offset = 0) {
		int hit = 0;
		size_t cur = offset, location = 0;
		while (hit < count)  {
			location = cur = code.find("spawn");
			if (isspace(code[location - 1])) {
				cur = location + 5;
				while (isspace(code[cur])) { cur++; }
				if (((code[cur] == '(') || isspace(code[cur])) && !isComment(cur, code) && !isQuote(cur, code)) { hit++; }
			}
			cur += 5;
		}
		if (flagVerbose) std::cout << (std::string) "Spawn " + intToString(count) + " start located." + ENDLINE;
		return location;
	}

	//Locate End of Spawn block.
	inline size_t searchSpawnEnd(std::string code, size_t SpawnStart) {
		return blockEnd(SpawnStart, code);
		if (flagVerbose) std::cout << (std::string) "Spawn block end located." + ENDLINE;
	}

	//Return true if a Keyword.
	bool isKeyword(std::string word) {
		bool flag = false;
		if (word == "auto") flag = true;
		else if (word == "break") flag = true;
		else if (word == "case") flag = true;
		else if (word == "char") flag = true;
		else if (word == "const") flag = true;
		else if (word == "continue") flag = true;
		else if (word == "default") flag = true;
		else if (word == "do") flag = true;
		else if (word == "double") flag = true;
		else if (word == "else") flag = true;
		else if (word == "enum") flag = true;
		else if (word == "extern") flag = true;
		else if (word == "float") flag = true;
		else if (word == "for") flag = true;
		else if (word == "goto") flag = true;
		else if (word == "if") flag = true;
		else if (word == "inline") flag = true;
		else if (word == "int") flag = true;
		else if (word == "long") flag = true;
		else if (word == "register") flag = true;
		else if (word == "restrict") flag = true;
		else if (word == "return") flag = true;
		else if (word == "short") flag = true;
		else if (word == "signed") flag = true;
		else if (word == "sizeof") flag = true;
		else if (word == "static") flag = true;
		else if (word == "struct") flag = true;
		else if (word == "switch") flag = true;
		else if (word == "typedef") flag = true;
		else if (word == "union") flag = true;
		else if (word == "unsigned") flag = true;
		else if (word == "void") flag = true;
		else if (word == "volatile") flag = true;
		else if (word == "while") flag = true;
		if (flagVerbose) {
			if (flag)
				std::cout << (std::string) "\"" + word + "\" is a Keyword." + ENDLINE;
			else
				std::cout << (std::string) "\"" + word + "\" is not a Keyword." + ENDLINE;
		}
		return flag;
	}

	//Return a string determining variable type.
	std::string findTypeOf(std::string code, size_t position) {
		std::string varType = "", tmp = "";
		size_t len = code.length(), tmplen, pos;
		int cur = 0, length = customDataTypes.size();
		pos = position - 1;
		if (length > 0) {
			while (pos > 0) {
				tmp = "";
				while ((isspace(code[pos]) || ispunct(code[pos])) && code[pos] != ';' && pos > 0) { pos--; }
				if (pos <= 0) break;
				if (code[pos] == ';') break;
				while (isalnum(code[pos]) && pos > 0) {
					tmp = code[pos] + tmp;
					pos--;
				}
				tmplen = tmp.length();
				while (cur < length) {
					if (tmp == customDataTypes.at(cur))
						return tmp;
					else
						cur++;
				}
				pos--;
			}
		}
		pos = position - 1;
		cur = 0;
		length = definedDataTypes.size();
		while (pos > 0) {
			tmp = "";
			while ((isspace(code[pos]) || ispunct(code[pos])) && code[pos] != ';' && pos > 0) { pos--; }
			if (pos <= 0) break;
			if (code[pos] == ';') break;
			while (isalpha(code[pos]) && pos > 0) {
				tmp = code[pos] + tmp;
				pos--;
			}
			tmplen = tmp.length();
			cur = 0;
			while (cur < length) {
				if (tmp == definedDataTypes.at(cur)) {
					varType = (std::string) tmp + " " + varType;
					break;
				}
				else cur++;
			}
			pos--;
		}
		if (flagVerbose) std::cout << (std::string) "Following type found : " + varType + ENDLINE;
		return varType;
	}

	//Return location of first occurence of variable
	size_t findFirstofVar(std::string code, std::string var) {
		size_t loc;
		loc = findLocationOfSpecial(code, var);
		if (flagVerbose) std::cout << (std::string) "First location of \"" + var + "\" found." + ENDLINE;
		return loc;
	}

	//Search for external variables accessed in a spawn and return as a string
	std::string searchGlobalReads(std::string program, std::string spawn, size_t locOfSpawn) {
		std::string variables = "", tmp = ""; //use ; as separator
		std::vector <std::string> vars;
		size_t sizeSpawn = spawn.size(), cur = 0, loc = 0;
		bool flag = false;
		while (spawn[cur] != '{') { cur++; }
		while (cur < sizeSpawn) {
			tmp = "";
			flag = true;
			while ((!isalpha(spawn[cur])) && (cur < sizeSpawn)) { cur++; }
			if (isQuote(cur, spawn) || isComment(cur, spawn)) {
				while ((isalnum(spawn[cur])) && (cur < sizeSpawn)) { cur++; }
				continue;
			}
			while ((isalnum(spawn[cur])) && (cur < sizeSpawn)) {
				tmp += spawn[cur++];
			}
			while (isspace(spawn[cur])) { cur++; }
			if (spawn[cur] == '(') { flag = false; continue; } //neglecting functions
			if (isKeyword(tmp)) { flag = false; continue; }
			loc = program.find_first_of(tmp);
			if ((flag) && (loc < locOfSpawn)) {
				vars.push_back(tmp);
				while ((isalnum(spawn[cur])) && (cur < sizeSpawn)) { cur++; }
			}
		}
		//Sorting and removing duplicates
		SourceManager::removeDuplicates(vars);
		unsigned int count = vars.size();
		cur = 0;
		while (cur < count) {
			variables += vars.at(cur) + ";";
			cur++;
		}
		if (flagVerbose) std::cout << (std::string) "Global Access Variables found in spawn block : " + ENDLINE + variables + ENDLINE;
		return variables;
	}

	//Function to check if characters are for a valid identifier.
	bool isValidIdentifierChar(char ch) {
		return (isalnum(ch) || (ch == '_'));
	}
};

#endif // !SourceManager_CPP_
