#include "Includes.h"


//##################################################################
//[ STRUCTURES ]
struct VMFunction
{
public:
	char Name[25];
	DWORD Offset;
	DWORD Size;
	static VMFunction VMFunction::Init(char* n, DWORD o, DWORD s)
	{
		VMFunction func;
		func.Offset = o;
		func.Size = s;
		ZeroMemory(func.Name, 25);
		memcpy((void*)func.Name, (void*)n, strlen(n));
		return func;
	}
	static VMFunction Init()
	{
		VMFunction func;
		func.Offset = 0;
		func.Size = 0;
		ZeroMemory(func.Name, 25);
		return func;
	}
};
struct VMRegs
{
public:
	DWORD reg[6];
	DWORD eip;
};
//##################################################################
//[ ADITIONAL FUNCTIONS ]
vector<std::string> &split(const string &s, char delim, vector<std::string> &elems) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}
vector<string> split(const string &s, char delim) {
	vector<string> elems;
	split(s, delim, elems);
	return elems;
}
//##################################################################
//[ ENUMERATORS ]
enum VMOpcodes
{
	OP_SET,
	OP_CMPSTR,
	OP_WAIT_KEY,
	OP_EXIT_PROGRAM,
	OP_OUTPUT,
	OP_INPUT,
	OP_CALL
};
//##################################################################
// [ CLASSES ]
class VMData
{
public:
	VMData(string lpFile)
	{
		hFile = CreateFile(lpFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			cout << "[ ERROR ] OPENING DATA THE FILE" << endl;
			return;
		}
		dwSize = GetFileSize(hFile, NULL);
		pCode = (BYTE*)VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
		DWORD dwRead;
		ReadFile(hFile, (void*)pCode, dwSize, &dwRead, NULL);
		Data = split(string((char*)pCode), '\n');
	}
	~VMData()
	{
		CloseHandle(hFile);
		VirtualFree((void*)pCode, dwSize, MEM_DECOMMIT);
	}

	char* getData(int id)
	{
		char* lpDecrypt = new char[Data[id].size()];
		ZeroMemory((void*)lpDecrypt, Data[id].size());
		for (int i = 0; i < Data[id].size(); i++)
		{
			if (Data[id][i] == 0x00)
				lpDecrypt[i] = 0xFF;
			else
				lpDecrypt[i] = Data[id][i] - (char)1;
		}
		return lpDecrypt;
	}
private:
	HANDLE hFile;
	BYTE* pCode;
	DWORD dwSize;
	vector<string> Data;
};
class VMFunctions
{
public:
	VMFunctions(string lpFile)
	{
		hFile = CreateFile(lpFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			cout << "[ ERROR ] FAILED LOADING EXPORTS" << endl;
			CloseHandle(hFile);
			return;
		}
		dwSize = GetFileSize(hFile, NULL);
		DWORD dwRead;
		VMFunction* f = new VMFunction[dwSize/sizeof(VMFunction)];
		ZeroMemory((void*)f, dwSize);
		ReadFile(hFile, (void*)f, dwSize, &dwRead, NULL);
		CloseHandle(hFile);
		for (int i = 0; i < dwSize / sizeof(VMFunction); i++)
			Functions.push_back(f[i]);
	}
	DWORD getFunctionOffset(string funcName)
	{
		for (auto it = Functions.begin(); it != Functions.end(); ++it)
		{
			if (funcName.compare(it->Name) == 0)
				return it->Offset;
		}
	}
	VMFunction getFunction(int id)
	{
		return Functions[id];
	}
	VMFunction getFunction(string funcName)
	{
		for (auto it = Functions.begin(); it != Functions.end(); ++it)
		{
			if (funcName.compare(it->Name) == 0)
				return *it;
		}
	}
private:
	HANDLE hFile;
	vector<VMFunction> Functions;
	DWORD dwSize;
};
class VMParser
{
public:
	VMParser(string lpFile, shared_ptr<VMData> d, shared_ptr<VMFunctions> f) :  Data(d), Functions(f)
	{
		hFile = CreateFile(lpFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			cout << "[ ERROR ] OPENING CODE THE FILE" << endl;
			return;
		}
		 dwSize = GetFileSize(hFile, NULL);
		pCode = (BYTE*)VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
		DWORD dwRead;
		ReadFile(hFile, (void*)pCode, dwSize, &dwRead, NULL);
	}
	virtual int parse_command(VMRegs* Registers)
	{
		string in;
		//PARSE OPCODES
		switch (pCode[Registers->eip])
		{
		case VMOpcodes::OP_INPUT:
			char* io;
			io = new char[250];
			ZeroMemory((void*)io, 250);
			cin >> io;
			Registers->reg[pCode[Registers->eip + 1]] = (DWORD)io;
			return 2;
			break;
		case VMOpcodes::OP_CALL:
			callFunction(Functions->getFunction(pCode[Registers->eip + 1]).Name, Registers);
			return 2;
			break;
		case VMOpcodes::OP_EXIT_PROGRAM:
			exit(0);
			return 1;
			break;
		case VMOpcodes::OP_WAIT_KEY:
			system("PAUSE");
			return 1;
			break;
		case VMOpcodes::OP_OUTPUT:
			char* lpStr;
			lpStr = (char*)Registers->reg[pCode[Registers->eip + 1]];
			cout << lpStr << endl;
			return 2;
			break;
		case VMOpcodes::OP_CMPSTR:
			char* fString;
			fString = (char*)Registers->reg[pCode[Registers->eip + 1]];
			char* sString;
			sString = (char*)Registers->reg[pCode[Registers->eip + 2]];
			if (strcmp(fString, sString) == 0)
				callFunction(Functions->getFunction(pCode[Registers->eip + 3]).Name, Registers);
			else
				callFunction(Functions->getFunction(pCode[Registers->eip + 3]).Name, Registers);
			return 5;
			break;
		case VMOpcodes::OP_SET:
			Registers->reg[pCode[Registers->eip + 1]] = (DWORD)Data->getData(pCode[Registers->eip +2]);
			return 3;
			break;
		default:
			cout << "[ ERROR ] INVALID OPCODE @0x" << hex << pCode[Registers->eip] << endl;
			return 0;
			break;
		}
	}
	virtual void callFunction(string funcName, VMRegs* regs)
	{
		int size = 1;
		VMFunction func = Functions->getFunction(funcName);
		regs->eip = func.Offset;
		while (size && regs->eip < func.Offset+func.Size) //CHECK FOR THE END OF THE FUNCTION
		{
			size = parse_command(regs);
			regs->eip += size;
		}
	}
private:
	HANDLE hFile;
	BYTE* pCode;
	DWORD dwSize;
	shared_ptr<VMData> Data;
	shared_ptr<VMFunctions> Functions;
};
//##################################################################
// [ VARIABLES ]
shared_ptr<VMData> Data;
shared_ptr<VMFunctions> Exports;
shared_ptr<VMParser> Parser;
//##################################################################
int main(int argc, char** argv)
{
	Data = shared_ptr<VMData>(new VMData("C:\\Users\\jmbri\\data.stub"));
	Exports = shared_ptr<VMFunctions>(new VMFunctions("C:\\Users\\jmbri\\export.stub"));
	VMRegs Regs = { 0 };
	Parser = shared_ptr<VMParser>(new VMParser("C:\\Users\\jmbri\\final_code.stub", Data, Exports));
	Parser->callFunction("auth", &Regs);
	cout << "FIM!" << endl;
	system("PAUSE");
	exit(0);
}