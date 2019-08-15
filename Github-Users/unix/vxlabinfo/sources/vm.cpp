#include "vm.h"
#include "tools.h"

//класс виртуальной машины.

Vm1::Vm1(){ }

Vm1::~Vm1()
{
	//освободим память..
	if(mem.stack.real_addr) VirtualFree((LPVOID)mem.stack.real_addr,0,MEM_RELEASE);
	if(mem.data.real_addr)  VirtualFree((LPVOID)mem.data.real_addr,0,MEM_RELEASE);
	if(mem.code.real_addr)  VirtualFree((LPVOID)mem.code.real_addr,0,MEM_RELEASE);
}

bool Vm1::LoadProgramm(LPVOID CodeAddr,int CodeSize,LPVOID DataAddr,int DataSize)
{
	mem_zero(&mem,sizeof(mem));

	//выделим память под код
	mem.code.real_addr=(DWORD)VirtualAlloc(0,code_block_size,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	if(mem.code.real_addr) mem.code.size=code_block_size;

	//выделим память под данные
	mem.data.real_addr=(DWORD)VirtualAlloc(0,data_block_size,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	if(mem.data.real_addr) mem.data.size=data_block_size;

	//выделим память под стек
	mem.stack.real_addr=(DWORD)VirtualAlloc(0,stack_block_size,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	if(mem.stack.real_addr) mem.stack.size=stack_block_size;
	//проверим что всё ок
	if(mem.code.size==0 || mem.data.size==0 || mem.stack.size==0) return false;

	//загрузим код
	if(CodeSize>0 && CodeSize<code_block_size)
	{
		//тупо скопируем
		mem_copy(CodeAddr,(LPVOID)mem.code.real_addr,CodeSize);
	} else { return false; }

	//загрузим данные
	if(DataSize>0 && DataSize<data_block_size)
	{
		//тупо скопируем
		mem_copy(DataAddr,(LPVOID)mem.data.real_addr,DataSize);
	} else { return false; }

	//настроим виртуальную память
	mem.code.virt_addr=code_base;
	mem.data.virt_addr=data_base;
	mem.stack.virt_addr=stack_base;

	//настроим регистры
	mem_zero(&reg,sizeof(reg));
	reg.eip=(DWORD)mem.code.virt_addr;
	reg.esp=(DWORD)mem.stack.virt_addr;

	return true; 
}

bool Vm1::Run()
{
	//запускаем каждую команду по одной.
	while(exec_command()){}
	return false; //бесполезно
}



///////////////////////////////////////////////////////////
//управление памятью
///////////////////////////////////////////////////////////

DWORD Vm1::convert_addr(DWORD addr,bool virt_to_real)
{
	//конвертер адресов
	//адреса идут один за другим
	section* section=0;

	if(virt_to_real)
	{
		//узнаем из какой секции виртуальный адрес.
		if(addr>=mem.code.virt_addr && addr  <= mem.code.virt_addr  + mem.code.size ) section=&mem.code;
		if(addr>=mem.data.virt_addr && addr  <= mem.data.virt_addr  + mem.data.size ) section=&mem.data;
		if(addr>=mem.stack.virt_addr && addr <= mem.stack.virt_addr + mem.stack.size ) section=&mem.stack;
		if(section==0)return false; //инвалид адрес
		DWORD real=(addr-section->virt_addr) + (DWORD)section->real_addr;
		return real;
	}else{
		//узнаем из какой секции реальный адрес.
		if(addr>=mem.code.real_addr && addr <= mem.code.real_addr + mem.code.size) section=&mem.code;
		if(addr>=mem.data.real_addr && addr <= mem.data.real_addr +  mem.data.size) section=&mem.data;
		if(addr>=mem.stack.real_addr && addr <= mem.stack.real_addr +  mem.stack.size) section=&mem.stack;
		if(section==0)return false; //инвалид адрес
		DWORD real=(addr-section->real_addr) + (DWORD)section->virt_addr;
		return real;
	}
}

//проверка принадлежности виртуальных адресов к секциям
bool Vm1::IsValid_VirtAddr(DWORD addr,DWORD size,section* sect)
{
	if(addr >= sect->virt_addr && (addr+size) <= (sect->virt_addr + sect->size) ) return true;
	return false;
}

//сразу во всех секциях
bool Vm1::IsValid_VirtAddrAll(DWORD addr,DWORD size)
{
	if( IsValid_VirtAddr(addr,size,&mem.code) ) return true;
	if( IsValid_VirtAddr(addr,size,&mem.data) ) return true;
	if( IsValid_VirtAddr(addr,size,&mem.stack)) return true;
	return false;
}


///////////////////////////////////////////////////////////
//парсер опкодов
///////////////////////////////////////////////////////////

//дизассемблер байт-кода
bool Vm1::parse_command()
{
	//проверим выход за пределы секции кода
	if(reg.eip+max_opcode_size>(mem.code.virt_addr+mem.code.size)){return false;}
	//распарсим по параметрам.
	DWORD real_addr=convert_addr(reg.eip,true);
	opcode opc;
	opc.opcodes=*(byte*)real_addr;
	opc.param1.byte=*(byte*)(real_addr+1);
	opc.param2.byte=*(byte*)(real_addr+2);
	//dword не может стоять на первом месте
	opc.param2.dword=*(DWORD*)(real_addr+2);

	switch (opc.opcodes)
	{
	case halt:
		cmd.id=halt;
		cmd.len=1;
		break;
	case push_reg:
		cmd.id=push_reg;
		cmd.len=2;
		cmd.operand=opc.param1.byte;
		break;
	case pop_reg:
		cmd.id=pop_reg;
		cmd.len=2;
		cmd.operand=opc.param1.byte;
		break;
	case cmp_reg_reg:
		cmd.id=cmp_reg_reg;
		cmd.len=3;
		cmd.operand=opc.param1.byte;
		cmd.operand2=opc.param2.byte;
		break;
	case mov_reg_reg:
		cmd.id=mov_reg_reg;
		cmd.len=3;
		cmd.operand=opc.param1.byte;
		cmd.operand2=opc.param2.byte;
		break;
	case mov_reg_mem:
		cmd.id=mov_reg_mem;
		cmd.len=6;
		cmd.operand=opc.param1.byte;
		cmd.operand2=opc.param2.dword;
		break;
	case jmp:
		cmd.id=jmp;
		cmd.len=6;
		cmd.operand=opc.param1.byte; //условие перехода
		cmd.operand2=opc.param2.dword; // дельта
		break;
	case add_reg_reg:
		cmd.id=add_reg_reg;
		cmd.len=3;
		cmd.operand=opc.param1.byte;
		cmd.operand2=opc.param2.byte;
		break;
	case mod_reg_imm:
		cmd.id=mod_reg_imm;
		cmd.len=6;
		cmd.operand=opc.param1.byte;
		cmd.operand2=opc.param2.dword;
		break;
	case shl_reg_reg:
		cmd.id=shl_reg_reg;
		cmd.len=6;
		cmd.operand=opc.param1.byte;
		cmd.operand2=opc.param2.dword;
		break;

	case mov_reg_byte:
		cmd.id=mov_reg_byte;
		cmd.len=6;
		cmd.operand=opc.param1.byte; //регистр
		cmd.operand2=opc.param2.dword;  //адрес
		break;

	case mov_reg_imm:
		cmd.id=mov_reg_imm;
		cmd.len=6;
		cmd.operand=opc.param1.byte; //регистр
		cmd.operand2=opc.param2.dword;  //значение
		break;

	case mov_reg_ptr_reg:
		cmd.id=mov_reg_ptr_reg;
		cmd.len=3;
		cmd.operand=opc.param1.byte; //регистр
		cmd.operand2=opc.param2.byte;  //регистр
		break;

	case add_reg_imm:
		cmd.id=add_reg_imm;
		cmd.len=6;
		cmd.operand=opc.param1.byte; //регистр
		cmd.operand2=opc.param2.dword;  //регистр
		break;

	case xor_byte_ptr_reg:
		cmd.id=xor_byte_ptr_reg;
		cmd.len=3;
		cmd.operand=opc.param1.byte; //регистр
		cmd.operand2=opc.param2.byte;  //параметр
		break;


	case antidebug_1:
		cmd.id=antidebug_1;
		cmd.len=6;
		cmd.operand=opc.param1.byte; 
		cmd.operand2=opc.param2.dword;  
		break;

	default:
		cmd.id=INVALID_OPCODE;
		cmd.len=0;
		return false;
	}
	return true;
}

//переход на обработчик команды
bool Vm1::exec_command()
{
	if(!parse_command()) return false;

	switch (cmd.id)
	{
	case halt:
		cmd_halt();
		return false;
	case push_reg:
		cmd_push(cmd.operand);
		break;
	case pop_reg:
		cmd_pop(cmd.operand);
		break;
	case cmp_reg_reg:
		cmd_cmp_reg_reg(cmd.operand,cmd.operand2);
		break;
	case mov_reg_reg:
		cmd_mov_reg_reg(cmd.operand,cmd.operand2);
		break;
	case mov_reg_mem:
		cmd_mov_reg_mem(cmd.operand,cmd.operand2);
		break;

	case jmp:
		cmd_jmp(cmd.operand2,cmd.operand);
		break;

	case add_reg_reg:
		cmd_add_reg_reg(cmd.operand,cmd.operand2);
		break;

	case mod_reg_imm:
		cmd_mod_reg_imm(cmd.operand,cmd.operand2);
		break;

	case shl_reg_reg:
		cmd_shl_reg_reg(cmd.operand,cmd.operand2);
		break;
		
	case mov_reg_byte:
		cmd_mov_reg_byte(cmd.operand,cmd.operand2);
		break;

	case mov_reg_imm:
		cmd_mov_reg_imm(cmd.operand,cmd.operand2);
		break;
		
	case mov_reg_ptr_reg:
		cmd_mov_reg_ptr_reg(cmd.operand,cmd.operand2);
		break;
		
	case add_reg_imm:
		cmd_add_reg_imm(cmd.operand,cmd.operand2);
		break;	

	case xor_byte_ptr_reg:
		cmd_xor_byte_ptr_reg(cmd.operand,cmd.operand2);
		break;	

	case antidebug_1:
		cmd_antidebug_1(cmd.operand,cmd.operand2);
		break;	

	default:
		return false;
	}
	//след команда..
	reg.eip+=cmd.len;
	return true;
}

DWORD Vm1::GetRegValue(int regs)
{
	switch(regs)
	{
	case eax:
		return reg.eax;
	case ecx:
		return reg.ecx;	
	case edx:
		return reg.edx;	
	case ebx:
		return reg.ebx;	
	case ebp:
		return reg.ebp;	
	case esp:
		return reg.esp;
	case eip:
		return reg.eip;	
	default:
		//бесполезно - надо генерить исключение
		return false;
	}
}


void Vm1::SetRegValue(int regs,DWORD value)
{
	switch(regs)
	{
	case eax:
		reg.eax=value;
		break;
	case ecx:
		reg.ecx=value;	
		break;
	case edx:
		reg.edx=value;
		break;
	case ebx:
		reg.ebx=value;
		break;
	case ebp:
		reg.ebp=value;
		break;
	case esp:
		reg.esp=value;
		break;
	case eip:
		reg.eip=value;
		break;
	default:
		//бесполезно - надо генерить исключение
		break;
	}
}

///////////////////////////////////////////////////////////
//обработчики опкодов
///////////////////////////////////////////////////////////
bool Vm1::cmd_push(DWORD regs)
{
	//printf("push",regs);
	//засунем в стек данные
	//проверим выход за пределы стека
	//возврат false - крушение стека!
	if(IsValid_VirtAddr(reg.esp,4,&mem.stack)) return false;
	
	DWORD real_addr=convert_addr(reg.esp,true);
	if(real_addr)
	{
		*(DWORD*)real_addr=GetRegValue(regs);
		reg.esp+=sizeof(DWORD);
		return true;
	}

	return false;
}

bool Vm1::cmd_pop(DWORD regs)
{
	//printf("pop",regs);
	//получим данные из стека
	if(IsValid_VirtAddr(reg.esp-4,4,&mem.stack)) return false;

	DWORD real_addr=convert_addr(reg.esp,true);
	if(real_addr)
	{
		DWORD value=*(DWORD*)real_addr;
		SetRegValue(regs,value);
		reg.esp-=sizeof(DWORD);
		return true;
	}

	return false;
}


bool Vm1::cmd_halt()
{
	//printf("halt");
	return true;
}


bool Vm1::cmd_cmp_reg_reg(DWORD regs,DWORD regs2)
{
	//printf("cmp reg,reg");
	//сравнение двух регистров
	DWORD reg1=GetRegValue(regs);
	DWORD reg2=GetRegValue(regs2);
	if(reg1==reg2){ reg.zero_flag=true;  reg.carry_flag=false; }
	if(reg1>reg2) { reg.zero_flag=false; reg.carry_flag=false; }
	if(reg1<reg2) { reg.zero_flag=false; reg.carry_flag=true;  }
	return true;
}

bool Vm1::cmd_mov_reg_reg(DWORD regs,DWORD regs2)
{
	//printf("mov reg,reg");
	//записываем одно значение в другое
	SetRegValue(regs,GetRegValue(regs2));
	return true;
}

//нужна функция проверки адреса!!
bool Vm1::cmd_mov_reg_mem(DWORD regs,DWORD addr)
{
	//printf("mov reg,mem");
	//записываем одно значение в другое
	if(!IsValid_VirtAddrAll(addr,4)) return false;

	DWORD real_addr=convert_addr(addr,true);
	DWORD real_src=*(DWORD*)real_addr;
	SetRegValue(regs,real_src);
	return true;
}


bool Vm1::cmd_jmp(DWORD delta,DWORD condition)
{
	//printf("JMP");
	//проверим не выйдем ли за предел..
	DWORD NewEip=reg.eip+delta;
	bool cond_valid=false;
	if(!IsValid_VirtAddr(NewEip,1,&mem.code)) return false;
	
	switch (condition)
	{
	case jmp_above:
		if(reg.carry_flag==0 && reg.zero_flag==0) cond_valid=true;
		break;
	case jmp_below:
		if(reg.carry_flag==1 && reg.zero_flag==0) cond_valid=true;
		break;
	case jmp_equal:
		if(reg.carry_flag==0 && reg.zero_flag==1) cond_valid=true;
		break;
	default:
		return false;
	}
	//сменим EIP
	if(cond_valid)
	{
		reg.eip=NewEip;
		return true;
	}
	return false;
}


bool Vm1::cmd_add_reg_reg(DWORD reg1,DWORD reg2)
{
	//printf("add reg,reg");
	//прибавляем один регистр к другому
	SetRegValue(reg1,GetRegValue(reg1)+GetRegValue(reg2));
	return true;
}

bool Vm1::cmd_mod_reg_imm(DWORD reg1,DWORD imm)
{
	//printf("mod reg,reg");
	//остаток от деления
	DWORD regs1=GetRegValue(reg1);
	regs1 = regs1 % imm;
	SetRegValue(reg1,regs1);
	return true;
}

void trick1()
{
	for(int i=0;i<1000;i++,OutputDebugStringW(GetCommandLineW())){}
}

bool Vm1::cmd_antidebug_1(DWORD reg1,DWORD imm)
{
	//printf("antidebug");
	trick1();
	return true;
}

void trick2()
{
	//!!
}

bool Vm1::cmd_shl_reg_reg(DWORD reg1,DWORD reg2)
{
	//printf("shl reg,reg");
	//побитовое смещение
	DWORD regs1=GetRegValue(reg1);
	DWORD regs2=reg2;
	regs1 = regs1 << regs2;
	SetRegValue(reg1,regs1);
	return true;
}

bool Vm1::cmd_mov_reg_byte(DWORD reg1,DWORD addr)
{
	//printf("mov reg,byte ptr [addr]");

	DWORD real_addr=convert_addr(addr,true);
	BYTE real_src=*(byte*)real_addr;
	SetRegValue(reg1,real_src);
	return true;
}

bool Vm1::cmd_mov_reg_imm(DWORD reg1,DWORD imm)
{
	//printf("mov reg,imm");
	SetRegValue(reg1,imm);
	return true;
}

bool Vm1::cmd_mov_reg_ptr_reg(DWORD reg1,DWORD reg2)
{
	//printf("mov reg,byte ptr [reg]");
	DWORD real_addr=GetRegValue(reg2);
	if(!IsValid_VirtAddrAll(real_addr,4)) return false;
	real_addr=convert_addr(real_addr,true);
	byte real_src=*(byte*)real_addr;
	SetRegValue(reg1,real_src);
	return true;
}

bool Vm1::cmd_add_reg_imm(DWORD reg1,DWORD imm)
{
	//printf("add reg,imm");
	SetRegValue(reg1,GetRegValue(reg1)+imm);
	return true;
}

bool Vm1::cmd_xor_byte_ptr_reg(DWORD reg1,DWORD reg2)
{
	//printf("xor byte ptr [reg],reg2
	DWORD real_addr=GetRegValue(reg1);
	if(!IsValid_VirtAddrAll(real_addr,4)) return false;
	real_addr=convert_addr(real_addr,true);
	byte real_src=*(byte*)real_addr;
	real_src^=(byte)GetRegValue(reg2);
	*(byte*)real_addr=real_src;
	return true;
}



///////////////////////////////////////////////////////////
// тест работы
///////////////////////////////////////////////////////////


char prog[]="\x0B\x00\x01\x10\x00\x00\x0A\x01\x00\x10\x00\x00\x0B\x02\x9A\x02\x00\x00\x0B\x03\x00\x00\x00\x00\x0F\x03\x03\x00\x30\x00\x0E\x00\x02\x07\x02\x01\x0D\x00\x01\x00\x00\x00\x0D\x01\xFF\xFF\xFF\xFF\x03\x01\x03\x06\x00\xDF\xFF\xFF\xFF\x00";
char valid[]="\x12\xec\xc5\xcb\xac\xfc\x86\x96\x23\x7c\x7d\x57\x46\x5c\x43\x4f\x56\x2d\x2a";

bool TestVm(char* sec)
{
	int len=lstrlenA(sec);
	if(len>100)return false;
	char buf[512];
	wsprintfA(buf,"x%s",sec);
	*(byte*)buf=len;

	Vm1 MyVm;
	DWORD tmp;
	tmp=MyVm.LoadProgramm(prog,sizeof(prog),buf,len+1);
	tmp=MyVm.Run();
	
	return CompareString2((byte*)MyVm.mem.data.real_addr,(byte*)valid,sizeof(valid));
}

