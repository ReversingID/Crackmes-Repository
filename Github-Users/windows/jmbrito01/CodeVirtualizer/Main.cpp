#define D_SCL_SECURE_NO_WARNINGS


#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>



using namespace std;

//##############################################################################################
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
//##############################################################################################
//[ DEFINES ]
#define INITIAL_STUB_SIZE						10000
//##############################################################################################
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
//##############################################################################################
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
//##############################################################################################
//[ CLASSES ]
class VMFunctionSystem
{
public:
	VMFunctionSystem() {}
	virtual void AddFunction(char* Name, DWORD Offset, DWORD Size)
	{
		Functions.push_back(VMFunction::Init(Name, Offset, Size));
	}
	virtual void AddFunction(VMFunction func)
	{
		Functions.push_back(func);
	}
	virtual VMFunction* GetFunctionByName(string lpFunction)
	{
		for (auto it = Functions.begin(); it != Functions.end(); ++it)
			if (lpFunction.compare(it->Name) == 0)
				return it._Ptr;
		return NULL;
	}
	virtual VMFunction* GetFunctionById(int ID)
	{
		return &Functions[ID];
	}
	virtual int GetFunctionIdByName(string lpFunction)
	{
		for (int i = 0; i < Functions.size(); i++)
		{
			if (strcmp(lpFunction.c_str(), Functions[i].Name) == 0)
				return i;
		}
		return -1;
	}
	virtual void Export(string lpFile)
	{
		HANDLE hFile = CreateFile(lpFile.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			cout << "[ ERROR ] EXPORTING FUNCTIONS" << endl;
			return;
		}
		VMFunction* f = new VMFunction[Functions.size()];
		for (int i = 0; i < Functions.size(); i++)
		{
			DWORD Written;
			WriteFile(hFile, (void*)&Functions[i], sizeof(VMFunction), &Written, NULL);
		}
		CloseHandle(hFile);
	}
private:
	vector<VMFunction> Functions;
};
class VMData
{
public:
	VMData() {}
	virtual BYTE appendData(char* Data)
	{
		char* newData = (char*)VirtualAlloc(NULL, strlen(Data) + 1, MEM_COMMIT, PAGE_READWRITE);
		ZeroMemory((void*)newData, strlen(Data) + 1);
		for (int i = 0; i < strlen(Data); i++)
		{
			if (Data[i] == 0xFF)
				newData[i] = 0;
			newData[i] = Data[i] + (char)1;
		}
		data.push_back(string(newData));
		return data.size() - 1;
	}
	virtual void buildFile(string filename)
	{
		file.open(filename.c_str(), ios::out);
		for (auto it = data.begin(); it != data.end(); ++it)
		{
			file << it->c_str() << endl;
		}
		file.close();
	}
private:
	vector<string> data;
	ofstream file;
};
class VMOutput
{
public:
	VMOutput(string lpFile)
	{
		vm.open(lpFile.c_str(), ios::out);
	}
	virtual void appendOpcode(VMOpcodes op)
	{
		vm << (BYTE)op;
	}
	virtual void appendPtr(DWORD ptr)
	{
		void* tmp = (char*)malloc(4);
		vm << (char*)tmp;
	}
	virtual void appendByte(BYTE b)
	{
		vm << b;
	}
private:
	ofstream vm;
};
class VMMacroSystem
{
public:
	VMMacroSystem(shared_ptr<VMOutput> out, shared_ptr<VMData> data, shared_ptr<VMFunctionSystem> f) : Out(out), PhysicalData(data), FuncSystem(f) {}
	virtual int parse_function(string funcName, string params)
	{
		if (funcName.compare("set") == 0)
			return set(params);
		else if (funcName.compare("cmp_str") == 0)
			return cmp_str(params);
		else if (funcName.compare("output") == 0)
			return output(params);
		else if (funcName.compare("wait_key") == 0)
			return wait_key();
		else if (funcName.compare("exit_program") == 0)
			return exit_program();
		else if (funcName.compare("call") == 0)
			return call(params);
		else if (funcName.compare("input") == 0)
			return input(params);
		else
			return 0;
	}
	virtual int input(string params)
	{
		//ERASE PARENTHESIS
		params.erase(params.begin());
		params.erase(params.end() - 1);

		Out->appendOpcode(VMOpcodes::OP_INPUT);
		int id = FuncSystem->GetFunctionIdByName(params);
		Out->appendByte((BYTE)id);
		return 2;
	}
	virtual int call(string params)
	{
		//ERASE PARENTHESIS
		params.erase(params.begin());
		params.erase(params.end() - 1);

		Out->appendOpcode(VMOpcodes::OP_CALL);
		int id = FuncSystem->GetFunctionIdByName(params);
		Out->appendByte((BYTE)id);
		return 2;
	}
	virtual int exit_program()
	{
		Out->appendByte((BYTE)VMOpcodes::OP_EXIT_PROGRAM);
		return 1;
	}
	virtual int wait_key()
	{
		Out->appendByte((BYTE)VMOpcodes::OP_WAIT_KEY);
		return 1;
	}
	virtual int output(string params)
	{
		//ERASE PARENTHESIS
		params.erase(params.begin());
		params.erase(params.end() - 1);
		
		Out->appendByte((BYTE)VMOpcodes::OP_OUTPUT);

		if (params.find("reg_") == 0)
		{
			int reg_id = atoi(params.substr(params.find("reg_") + strlen("reg_"), 1).c_str());
			Out->appendByte((BYTE)reg_id);
		}
		return 2;
	}
	virtual int cmp_str(string params)
	{
		params.erase(params.begin());
		params.erase(params.end()-1);
		vector<string> p = split(params, ',');
		Out->appendOpcode(VMOpcodes::OP_CMPSTR);
		string first_param = p[0];
		//CHECK IF FIRST PARAM IS A REGISTER
		if (first_param.find("reg_") == 0)
		{
			int reg_id = atoi(first_param.substr(first_param.find("reg_") + strlen("reg_"), 1).c_str());
			Out->appendByte((BYTE)reg_id);
		}
		//CHECK IF SECOND PARAM IS A REGISTER
		string second_param = p[1];
		if (second_param.find("reg_") == 0)
		{
			int reg_id = atoi(second_param.substr(second_param.find("reg_") + strlen("reg_"), 1).c_str());
			Out->appendByte((BYTE)reg_id);
		}
		int id_1 = FuncSystem->GetFunctionIdByName(p[2]);
		Out->appendByte((BYTE)id_1);
		string last_param = p[3];
		last_param.erase(last_param.end());
		cout << last_param << endl;
		int id_2 = FuncSystem->GetFunctionIdByName(last_param);
		Out->appendByte((BYTE)id_2);
		return 5;
	}
	virtual int set(string params)
	{
		Out->appendOpcode(VMOpcodes::OP_SET);
		string first_param = params.substr(1, params.find_first_of(",")-1);
		//CHECK IF FIRST PARAM IS A REGISTER
		if (first_param.find("reg_") == 0)
		{
			int reg_id = atoi(first_param.substr(first_param.find("reg_")+strlen("reg_"), 1).c_str());
			Out->appendByte((BYTE)reg_id);
		}
		string second_param = params.substr(params.find_first_of(",") + 1, params.find_last_of(")")- params.find_first_of(","));

		//CHECK IF SECOND PARAM IS A STRING
		if (second_param[0] == '\"')
		{
			//IT'S A STRING
			string string_param = second_param.substr(1, second_param.find_last_of("\"") - 1);
			//ADD STRING TO DATA
			BYTE offset = PhysicalData->appendData((char*)string_param.c_str());
			Out->appendByte(offset);
		}
		return 3;
	}
private:
	shared_ptr<VMOutput> Out;
	shared_ptr<VMData> PhysicalData;
	shared_ptr<VMFunctionSystem> FuncSystem;
};
//##############################################################################################
//[ VARIABLES ]
shared_ptr<VMFunctionSystem> FuncSystem;
shared_ptr<VMMacroSystem> MacroSystem;
shared_ptr<VMOutput> Output;
shared_ptr<VMData> Data;
//##############################################################################################
int main(void)
{
	cout << "Please type the file containing the code: " << endl;
	string filename;
	cin >> filename;
	cout << filename << endl;
	cout << "Compiling..." << endl;
	HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		cout << "[ ERROR #1 ] OPENING THE FILE" << endl;
		system("PAUSE");
		exit(0);
	}
	DWORD dwSize = GetFileSize(hFile, NULL);
	void* pAlloc = VirtualAlloc(NULL, dwSize+1, MEM_COMMIT, PAGE_READWRITE);
	ZeroMemory(pAlloc, dwSize+1);
	DWORD Read;
	ReadFile(hFile, pAlloc, dwSize, &Read, NULL);
	//INITIALIZING VM
	FuncSystem			= shared_ptr<VMFunctionSystem>(new VMFunctionSystem());
	//StackSystem			= shared_ptr<VMStackSystem>(new VMStackSystem());
	Output				= shared_ptr<VMOutput>(new VMOutput("C:\\Users\\jmbri\\final_code.stub"));
	Data				= shared_ptr<VMData>(new VMData());
	MacroSystem			= shared_ptr<VMMacroSystem>(new VMMacroSystem(Output, Data, FuncSystem));

	//START PARSING THE FILES
	//PARSING FUNCTIONS
	
	VMFunction new_func = VMFunction();
	string lpCode = string((char*)pAlloc);
	VirtualFree(pAlloc, dwSize + 1, MEM_DECOMMIT);
	DWORD CurOffset = 0;
	void* pStub = malloc(INITIAL_STUB_SIZE);
	ZeroMemory(pStub, INITIAL_STUB_SIZE);
	int location = lpCode.find("#");

	while (location != lpCode.npos)
	{
		ZeroMemory((void*)&new_func, sizeof(VMFunction));
		int end_name = lpCode.find(":", location);
		memcpy(new_func.Name, lpCode.substr(location + 1, end_name - location - 1).c_str(), lpCode.substr(location + 1, end_name - location - 1).length());
		new_func.Offset = CurOffset;

		int start_func = strlen("__begin__") + lpCode.find("__begin__", location + 1)+1;
		int end_func = lpCode.find("__end__", start_func)-start_func;
		string lpFunction = lpCode.substr(start_func, end_func);

		//PASSING EACH COMMAND
		int hash_call = lpFunction.find(">");
		while (hash_call != lpFunction.npos && hash_call < end_func)
		{
			string func_name = lpFunction.substr(hash_call + 1, lpFunction.find("(", hash_call) - hash_call - 1);
			string params = lpFunction.substr(lpFunction.find("(", hash_call), lpFunction.find(")", hash_call) - lpFunction.find("(", hash_call)+1);
			int size = MacroSystem->parse_function(func_name, params);
			if (size > 0)
			{
				hash_call = lpFunction.find(">", hash_call + 1);
				new_func.Size += size;
			}
			else
			{
				cout << "[ SYNTAX ERROR ] INVALID FUNCTION: \"" << func_name << "\"" << endl;
				system("PAUSE");
				hash_call = lpFunction.npos;
			}
		}
		FuncSystem->AddFunction(new_func);
		CurOffset += new_func.Size;
		location = lpCode.find("#", location + 1);
	}
	cout << "Building DATA Stub..." << endl;
	Data->buildFile("C:\\Users\\jmbri\\data.stub");
	cout << "Building exports..." << endl;
	FuncSystem->Export("C:\\Users\\jmbri\\export.stub");
	system("PAUSE");
	exit(0);
}