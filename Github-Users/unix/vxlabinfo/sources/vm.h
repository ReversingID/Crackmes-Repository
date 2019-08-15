#include "stdafx.h"

struct virt_proc
{
	DWORD eax;
	DWORD ecx;
	DWORD edx;
	DWORD ebx;
	DWORD ebp;
	DWORD esp;
	DWORD eip;
	bool zero_flag;
	bool carry_flag;
};

struct section
{
	DWORD virt_addr;
	DWORD real_addr;
	DWORD size;
};

struct virt_mem
{
	section code;
	section data;
	section stack;
};

#define code_block_size 0x1000
#define data_block_size  0x1000
#define stack_block_size  0x10000

#define code_base 0
#define data_base code_base+code_block_size
#define stack_base data_base+data_block_size


struct command
{
	DWORD id;
	DWORD operand;
	DWORD operand2;
	int len;
};

enum Command{halt=0x00,push_reg,pop_reg,cmp_reg_reg,mov_reg_reg,mov_reg_mem,jmp,add_reg_reg,mod_reg_imm,shl_reg_reg,mov_reg_byte,mov_reg_imm,mov_reg_ptr_reg,add_reg_imm,xor_byte_ptr_reg,antidebug_1};
enum Regs{eax=0x00,ecx,edx,ebx,ebp,esp,eip};
#define max_opcode_size 10 //2-4-4
#define INVALID_OPCODE -1
enum Jmps{jmp_above,jmp_below,jmp_equal};


struct param
{
	byte byte;
	DWORD dword;
};

struct opcode
{
	byte opcodes;
	param param1;
	param param2;
};







class Vm1
{
public:
	Vm1();
	~Vm1();
	bool Vm1::LoadProgramm(LPVOID CodeAddr,int CodeSize,LPVOID DataAddr,int DataSize);
	bool Vm1::Run();
	virt_mem mem;
private:
	virt_proc reg;
	command cmd;

	bool Vm1::cmd_push(DWORD reg);
	bool Vm1::cmd_pop(DWORD reg);
	bool Vm1::cmd_halt();
	DWORD Vm1::GetRegValue(int regs);
	void Vm1::SetRegValue(int regs,DWORD value);
	bool Vm1::parse_command();
	DWORD Vm1::convert_addr(DWORD addr,bool virt_to_real);
	bool Vm1::exec_command();
	bool Vm1::cmd_cmp_reg_reg(DWORD regs,DWORD regs2);
	bool Vm1::cmd_mov_reg_reg(DWORD regs,DWORD regs2);
	bool Vm1::cmd_jmp(DWORD delta,DWORD condition);
	bool Vm1::cmd_mov_reg_mem(DWORD regs,DWORD addr);
	bool Vm1::IsValid_VirtAddr(DWORD addr,DWORD size,section* sect);
	bool Vm1::IsValid_VirtAddrAll(DWORD addr,DWORD size);
	bool Vm1::cmd_add_reg_reg(DWORD reg1,DWORD reg2);
	bool Vm1::cmd_mod_reg_imm(DWORD reg1,DWORD reg2);
	bool Vm1::cmd_shl_reg_reg(DWORD reg1,DWORD reg2);
	bool Vm1::cmd_mov_reg_byte(DWORD reg1,DWORD addr);
	bool Vm1::cmd_mov_reg_imm(DWORD reg1,DWORD imm);
	bool Vm1::cmd_mov_reg_ptr_reg(DWORD reg1,DWORD reg2);
	bool Vm1::cmd_add_reg_imm(DWORD reg1,DWORD imm);
	bool Vm1::cmd_xor_byte_ptr_reg(DWORD reg1,DWORD imm);
	bool Vm1::cmd_antidebug_1(DWORD reg1,DWORD imm);
	
};





bool TestVm(char* sec);
