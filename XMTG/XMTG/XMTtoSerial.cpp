#include "SourceManager.cpp"

//XMT to Serial C Code
class XMTtoSerial{
private:
	//Keeps the original source of the program.
	std::string XMTProgram;

	//Keeps the modified version of the program.
	std::string modifiedProgram;

	//Object to use SourceManager.
	SourceManager sourceMgr;
public:
	//Constructor
	XMTtoSerial(std::string InputProgram) {
		XMTProgram = InputProgram;
		modifiedProgram = XMTProgram;
	}

	//To remove XMT headers.
	void manageHeaders() {
		modifiedProgram = sourceMgr.removeHeader(modifiedProgram, "xmtc.h");
		modifiedProgram = (std::string) "#include <stdio.h>" + ENDLINE + modifiedProgram;
	}

	//Replace all spawn with for loops.
	void replaceSpawnWithForLoop() {
		int cur = 0, count = sourceMgr.findCountOfSpecial(modifiedProgram, "spawn");
		std::string startString = "", endString = "", curString = "";
		size_t loc, saveloc;
		while (cur < count) {
			saveloc = loc = sourceMgr.findLocationOfSpecial(modifiedProgram, "spawn", cur + 1);
			curString = sourceMgr.intToString(cur);
			while (modifiedProgram[loc++] != '(');
			while (!isalpha(modifiedProgram[loc])) { loc++; }
			while (isalnum(modifiedProgram[loc])) {
				startString += modifiedProgram[loc];
				loc++;
			}
			while (!isalpha(modifiedProgram[loc])) { loc++; }
			while (isalnum(modifiedProgram[loc])) {
				endString += modifiedProgram[loc];
				loc++;
			}
			while (modifiedProgram[loc++] != '{');
			modifiedProgram.erase(saveloc, loc - saveloc);
			modifiedProgram.insert(saveloc, (std::string) "for( int __Counter" + curString + " = " + startString + "; __Counter" + curString + " < " + endString + "; __Counter" + curString + "++) {");
			modifiedProgram = sourceMgr.replaceEverywhere(modifiedProgram, "$", "__Counter" + curString);
			cur++;
		}
	}

	//Get back generated Serial code.
	std::string getSerialCode() {
		return modifiedProgram;
	}
};
