//*****************************************************************************
//
//  rv32i.cpp
//  CSCI 463 Assignment 4
//
//  Created by Daniel Widing (z1838064)
//
//*****************************************************************************
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <string>
#include <fstream>
#include <cstring>
#include <sstream>
#include <cassert>

#include "rv32i.h"

using namespace std;

static uint32_t get_opcode(uint32_t insn);
static uint32_t get_rd(uint32_t insn);
static uint32_t get_funct3(uint32_t insn);
static uint32_t get_rs1(uint32_t insn);
static uint32_t get_rs2(uint32_t insn);
static uint32_t get_funct7(uint32_t insn);
static uint32_t get_imm_u(uint32_t insn);
static uint32_t get_imm_j(uint32_t insn);
static uint32_t get_imm_i(uint32_t insn);
static uint32_t get_imm_s(uint32_t insn);
static uint32_t get_imm_b(uint32_t insn);

/**
 * Constructs rv32i object by saving passed memory pointer to decode
 *
 * @param m: pointer to memory to save in new object for decoding
 **/
rv32i::rv32i(memory* m)
{
	//Sets object memory to passed memory
	mem = m;

	//Sets current instruction decoding line to 0 to begin with  
	pc = 0;
}

/**
 * Disassembles all instructions in calling objects memory
 * 
 * Loops through all 32 bit words in memory:
 *   - Print address
 *	 - Print encoded instruction/word
 *   - Decodes instruction
 *   - Prints decoded instruction/error
 **/
void rv32i::disasm()
{
	//Resets instruction address location
	pc = 0;

	//Loops through all bytes
	while (pc < mem->get_size())
	{
		//Print address
		cout << setw(8) << setfill('0') << hex32(pc) << ": ";
		
		//Gets instruction bytes
		uint32_t insn = mem->get32(pc);
		
		//Prints encoded bytes
		cout << setw(8) << setfill('0') << hex << static_cast<int>(insn) << "  " << dec;
		
		//Decodes and prints instruction bytes		
		cout << decode(insn) << endl;

		//Moves to next address
		pc += 4;
	}
}

/**
 * Decodes the passed encoded instruction string and returns it
 *
 * @param insn: encoded instruction to be decoded
 *
 * @return: decoded instruction string
 **/
string rv32i::decode(uint32_t insn) const
{
	//Extracts opcode, funct3, funct7 from instruction
	uint32_t opcode = get_opcode(insn);
	uint32_t funct3 = get_funct3(insn);
	uint32_t funct7 = get_funct7(insn);

	//Returns instuction string based on opcode and instuction
	switch (opcode)
	{
	default:
		return render_illegal_insn();
	case opcode_lui:
		return render_lui(insn);
	case opcode_auipc:
		return render_auipc(insn);
	case opcode_jal:
		return render_jal(insn);
	case opcode_jalr:
		return render_jalr(insn);
	case opcode_btype:
		switch (funct3)
		{
		default:
			return render_illegal_insn();
		case funct3_beq:
			return render_btype(insn, "beq");
		case funct3_bne:
			return render_btype(insn, "bne");
		case funct3_blt:
			return render_btype(insn, "blt");
		case funct3_bge:
			return render_btype(insn, "bge");
		case funct3_bltu:
			return render_btype(insn, "bltu");
		case funct3_bgeu:
			return render_btype(insn, "bgeu");
		}
	case opcode_itype_load:
		switch (funct3)
		{
		default:
			return render_illegal_insn();
		case funct3_lb:
			return render_itype_load(insn, "lb");
		case funct3_lh:
			return render_itype_load(insn, "lh");
		case funct3_lw:
			return render_itype_load(insn, "lw");
		case funct3_lbu:
			return render_itype_load(insn, "lbu");
		case funct3_lhu:
			return render_itype_load(insn, "lhu");
		}
	case opcode_stype:
		switch (funct3)
		{
		default:
			return render_illegal_insn();
		case funct3_sb:
			return render_stype(insn, "sb");
		case funct3_sh:
			return render_stype(insn, "sh");
		case funct3_sw:
			return render_stype(insn, "sw");
		}
	case opcode_itype_alu:
		switch (funct3)
		{
		default:
			return render_illegal_insn();
		case funct3_addi:
			return render_itype_alu(insn, "addi");
		case funct3_slti:
			return render_itype_alu(insn, "slti");
		case funct3_sltiu:
			return render_itype_alu(insn, "sltiu");
		case funct3_xori:
			return render_itype_alu(insn, "xori");
		case funct3_ori:
			return render_itype_alu(insn, "ori");
		case funct3_andi:
			return render_itype_alu(insn, "andi");
		case funct3_slli:
			return render_itype_alu_shamt(insn, "slli");
		case funct3_sr:
			switch (funct7)
			{
			default:
				return render_illegal_insn();
			case funct7_srli:
				return render_itype_alu_shamt(insn, "srli");
			case funct7_srai:
				return render_itype_alu_shamt(insn, "srai");
			}
		}	
	case opcode_rtype:
		switch (funct3)
		{
		default:
			return render_illegal_insn();
		case funct3_addsub:
			switch (funct7)
			{
			default:
				return render_illegal_insn();
			case funct7_add:
				return render_rtype(insn, "add");
			case funct7_sub:
				return render_rtype(insn, "sub");
			}
		case funct3_sll:
			return render_rtype(insn, "sll");
		case funct3_slt:
			return render_rtype(insn, "slt");
		case funct3_sltu:
			return render_rtype(insn, "sltu");
		case funct3_xor:
			return render_rtype(insn, "xor");
		case funct3_sr2:
			switch (funct7)
			{
			default:
				return render_illegal_insn();
			case funct7_srl:
				return render_rtype(insn, "srl");
			case funct7_sra:
				return render_rtype(insn, "sra");
			}
		case funct3_or:
			return render_rtype(insn, "or");
		case funct3_and:
			return render_rtype(insn, "and");
		}
	case opcode_fence:
		return render_fence(insn);
	case opcode_itype_spe:
		switch (funct3)
		{
		default:
			return render_illegal_insn();
		case funct3_ecallbreak:
			switch (insn)
			{
			default:
				return render_illegal_insn();
			case insn_ecall:
				return render_ecall();
			case insn_ebreak:
				return render_ebreak();
			}
		case funct3_csrrw:
			return render_itype_spe("csrrw");
		case funct3_csrrs:
			return render_itype_spe("csrrs");
		case funct3_csrrc:
			return render_itype_spe("csrrc");
		case funct3_csrrwi:
			return render_itype_spe("csrrwi");
		case funct3_csrrsi:
			return render_itype_spe("csrrsi");
		case funct3_csrrci:
			return render_itype_spe("csrrci");
		}
	}
}

/**
 * Extracts the opcode (bits 6-0) from passed instruction
 *
 * @param insn: encoded instruction to get opcode from
 *
 * @return: extracted opcode
 **/
static uint32_t get_opcode(uint32_t insn)
{
	return insn & 0x0000007f;
}

/**
 * Extracts the rd (bits 11-7) from passed instruction
 *
 * @param insn: encoded instruction to get rd from
 *
 * @return: extracted rd
 **/
static uint32_t get_rd(uint32_t insn)
{
	//Shifts instruction bits 7 to the right, to put rd bits at end
	uint32_t rd = insn >> 7;

	//Clears all but the rd bits
	return rd & 0x0000001f;
}

/**
 * Extracts the rs1 (bits 19-15) from passed instruction
 *
 * @param insn: encoded instruction to get rs1 from
 *
 * @return: extracted rs1
 **/
static uint32_t get_rs1(uint32_t insn)
{
	//Shifts instruction bits 15 to the right, to put rs1 bits at end
	uint32_t rs1 = insn >> 15;

	//Clears all but the rs1 bits
	return rs1 & 0x0000001f;
}

/**
 * Extracts the rs2 (bits 24-20) from passed instruction
 *
 * @param insn: encoded instruction to get rs2 from
 *
 * @return: extracted rs2
 **/
static uint32_t get_rs2(uint32_t insn)
{
	//Shifts instruction bits 20 to the right, to put rs2 bits at end
	uint32_t rs2 = insn >> 20;

	//Clears all but the rs2 bits
	return rs2 & 0x0000001f;
}

/**
 * Extracts the funct3 (bits 14-12) from passed instruction
 *
 * @param insn: encoded instruction to get funct3 from
 *
 * @return: extracted funct3
 **/
static uint32_t get_funct3(uint32_t insn)
{
	//Shifts instruction bits 12 to the right, to put funct3 bits at end
	uint32_t funct3 = insn >> 12;

	//Clears all but the funct3 bits
	return funct3 & 0x00000007;
}

/**
 * Extracts the funct7 (bits 31-25) from passed instruction
 *
 * @param insn: encoded instruction to get funct7 from
 *
 * @return: extracted funct7
 **/
static uint32_t get_funct7(uint32_t insn)
{
	//Shifts instruction bits 25 to the right, to put funct7 bits at end
	uint32_t funct3 = insn >> 25;

	//Clears all but the funct7 bits
	return funct3 & 0x0000007f;
}

/**
 * Extracts the imm (bits 31-12) from passed u-type instruction
 *
 * @param insn: encoded instruction to get imm from
 *
 * @return: extracted imm
 **/
static uint32_t get_imm_u(uint32_t insn)
{
	//Shifts instruction bits 12 to the right, to put imm bits at end
	uint32_t imm = insn >> 12;

	//Clears all but the imm bits
	return imm & 0x000fffff;
}

/**
 * Extracts the imm bits from passed j-type instruction and assembles them properly
 *
 * @param insn: encoded instruction to get imm from
 *
 * @return: extracted imm
 **/
static uint32_t get_imm_j(uint32_t insn)
{
	//Gets components to right position
	uint32_t imm20    = insn >> 11;
	uint32_t imm19_12 = insn;
	uint32_t imm11    = insn >> 9;
	uint32_t imm10_1  = insn >> 20;

	//Gets all components of the imm
	imm20    = imm20    & 0x00100000;
	imm19_12 = imm19_12 & 0x000ff000;
	imm11    = imm11    & 0x00000800;
	imm10_1  = imm10_1  & 0x000007fe;

	//Assembles components of imm
	uint32_t imm = imm20 | imm11 | imm19_12 | imm10_1;

	//Sign extends
	if ((insn & 0x80000000) == 0x80000000)
	{
		imm |= 0xFFE00000;
	}

	return imm;
}

/**
 * Extracts the imm (bits 31-20) from passed i-type instruction
 *
 * @param insn: encoded instruction to get imm from
 *
 * @return: extracted imm
 **/
static uint32_t get_imm_i(uint32_t insn)
{
	//Shifts instruction bits 20 to the right, to put imm bits at end
	uint32_t imm = insn >> 20;

	//Clears all but the imm bits
	imm &= 0x00000fff;

	//Sign extends
	if ((insn & 0x80000000) == 0x80000000)
	{
		imm |= 0xFFFFF000;
	}

	return imm;
}

/**
 * Extracts the imm bits from passed s-type instruction and assembles them properly
 *
 * @param insn: encoded instruction to get imm from
 *
 * @return: extracted imm
 **/
static uint32_t get_imm_s(uint32_t insn)
{
	//Gets components to right position
	uint32_t imm11_5 = insn >> 20;
	uint32_t imm4_0 = insn >> 7;

	//Gets all components of the imm
	imm11_5 = imm11_5 & 0x00000fe0;
	imm4_0 = imm4_0   & 0x0000001f;

	//Assembles components of imm
	uint32_t imm = imm11_5 | imm4_0;
	
	//Sign extends
	if ((insn & 0x80000000) == 0x80000000)
	{
		imm |= 0xFFFFF000;
	}
	
	return imm;
}

/**
 * Extracts the imm bits from passed b-type instruction and assembles them properly
 *
 * @param insn: encoded instruction to get imm from
 *
 * @return: extracted imm
 **/
static uint32_t get_imm_b(uint32_t insn)
{
	//Gets components to right position
	uint32_t imm12   = insn >> 19;
	uint32_t imm11   = insn << 4;
	uint32_t imm10_5 = insn >> 20;
	uint32_t imm4_1  = insn >> 7;

	//Gets all components of the imm
	imm12   = imm12   & 0x00001000;
	imm11   = imm11   & 0x00000800;
	imm10_5 = imm10_5 & 0x000007e0;
	imm4_1  = imm4_1  & 0x0000001e;

	//Assembles components of imm
	uint32_t imm = imm12 | imm11 | imm10_5 | imm4_1;

	//Sign extends
	if ((insn & 0x80000000) == 0x80000000)
	{
		imm |= 0xFFFFE000;
	}

	return imm;
}

/**
 * Returns an error string for unreadable instructions
 *
 * @return: error string
 **/
string rv32i::render_illegal_insn() const
{
	return "ERROR: UNIMPLEMENTED INSTRUCTION";
}

/**
 * Decodes the lui instruction as a string
 *
 * @param insn: encoded instruction to decode
 *
 * @return: decoded instruction string
 **/
string rv32i::render_lui(uint32_t insn) const
{
	//Gets encoded components of instruction
	uint32_t rd = get_rd(insn);
	int32_t imm = get_imm_u(insn);

	//Assembles components into decoded string
	ostringstream os;
	os << setw(mnemonic_width) << setfill(' ') << left << "lui" << "x" << dec << rd << ",0x" << hex << imm;

	//Returns decoded string
	return os.str();
}

/**
 * Decodes the auipc instruction as a string
 *
 * @param insn: encoded instruction to decode
 *
 * @return: decoded instruction string
 **/
string rv32i::render_auipc(uint32_t insn) const
{
	//Gets encoded components of instruction
	uint32_t rd = get_rd(insn);
	int32_t imm = get_imm_u(insn);

	//Assembles components into decoded string
	ostringstream os;
	os << setw(mnemonic_width) << setfill(' ') << left << "auipc" << "x" << dec << rd << ",0x" << hex << imm;

	//Returns decoded string
	return os.str();
}

/**
 * Decodes the jal instruction as a string
 *
 * @param insn: encoded instruction to decode
 *
 * @return: decoded instruction string
 **/
string rv32i::render_jal(uint32_t insn) const
{
	//Gets encoded components of instruction
	uint32_t rd = get_rd(insn);
	int32_t imm = get_imm_j(insn);
	
	//Gets relative imm
	int32_t pcrel_21 = imm + pc;

	//Assembles components into decoded string
	ostringstream os;
	os << setw(mnemonic_width) << setfill(' ') << left << "jal" << "x" << dec << rd << ",0x" << hex << pcrel_21;

	//Returns decoded string
	return os.str();
}

/**
 * Decodes the jalr instruction as a string
 *
 * @param insn: encoded instruction to decode
 *
 * @return: decoded instruction string
 **/
string rv32i::render_jalr(uint32_t insn) const
{
	//Gets encoded components of instruction
	uint32_t rd = get_rd(insn);
	int32_t imm = get_imm_i(insn);
	uint32_t rs1 = get_rs1(insn);

	//Assembles components into decoded string
	ostringstream os;
	os << setw(mnemonic_width) << setfill(' ') << left << "jalr" << "x" << dec << rd << "," << dec << imm << "(x" << rs1 << ")";

	//Returns decoded string
	return os.str();
}

/**
 * Decodes b-type instruction as a string
 *
 * @param     insn: encoded instruction to decode
 *        mnemonic: mnemonic for specific instruction
 * 
 * @return: decoded instruction string
 **/
string rv32i::render_btype(uint32_t insn, const char* mnemonic) const
{
	//Gets encoded components of instruction
	uint32_t rs1 = get_rs1(insn);
	uint32_t rs2 = get_rs2(insn);
	int32_t imm = get_imm_b(insn);

	//Gets relative imm
	int32_t pcrel_13 = imm + pc;

	//Assembles components into decoded string
	ostringstream os;
	os << setw(mnemonic_width) << setfill(' ') << left << mnemonic << "x" << dec << rs1 << ",x" << dec << rs2 << ",0x" << hex << pcrel_13;

	//Returns decoded string
	return os.str();
}

/**
 * Decodes i-type loading instruction as a string
 *
 * @param     insn: encoded instruction to decode
 *        mnemonic: mnemonic for specific instruction
 *
 * @return: decoded instruction string
 **/
string rv32i::render_itype_load(uint32_t insn, const char* mnemonic) const
{
	//Gets encoded components of instruction
	uint32_t rd = get_rd(insn);
	int32_t imm = get_imm_i(insn);
	uint32_t rs1 = get_rs1(insn);

	//Assembles components into decoded string
	ostringstream os;
	os << setw(mnemonic_width) << setfill(' ') << left << mnemonic << "x" << dec << rd << "," << dec << imm << "(x" << rs1 << ")";

	//Returns decoded string
	return os.str();
}

/**
 * Decodes s-type instruction as a string
 *
 * @param     insn: encoded instruction to decode
 *        mnemonic: mnemonic for specific instruction
 *
 * @return: decoded instruction string
 **/
string rv32i::render_stype(uint32_t insn, const char* mnemonic) const
{
	//Gets encoded components of instruction
	uint32_t rs2 = get_rs2(insn);
	int32_t imm = get_imm_s(insn);
	uint32_t rs1 = get_rs1(insn);

	//Assembles components into decoded string
	ostringstream os;
	os << setw(mnemonic_width) << setfill(' ') << left << mnemonic << "x" << dec << rs2 << "," << dec << imm << "(x" << rs1 << ")";

	//Returns decoded string
	return os.str();
}

/**
 * Decodes i-type alu instruction as a string
 *
 * @param     insn: encoded instruction to decode
 *        mnemonic: mnemonic for specific instruction
 *
 * @return: decoded instruction string
 **/
string rv32i::render_itype_alu(uint32_t insn, const char* mnemonic) const
{
	//Gets encoded components of instruction
	uint32_t rd = get_rd(insn);
	uint32_t rs1 = get_rs1(insn);
	int32_t imm = get_imm_i(insn);

	//Assembles components into decoded string
	ostringstream os;
	os << setw(mnemonic_width) << setfill(' ') << left << mnemonic << "x" << dec << rd << ",x" << dec << rs1 << "," << dec << imm;

	//Returns decoded string
	return os.str();
}

/**
 * Decodes i-type alu shamt instruction as a string
 *
 * @param     insn: encoded instruction to decode
 *        mnemonic: mnemonic for specific instruction
 *
 * @return: decoded instruction string
 **/
string rv32i::render_itype_alu_shamt(uint32_t insn, const char* mnemonic) const
{
	//Gets encoded components of instruction
	uint32_t rd = get_rd(insn);
	uint32_t rs1 = get_rs1(insn);
	int32_t imm = get_imm_i(insn);

	//Gets shamt from imm value
	int32_t shamt = imm & 0x0000001F;

	//Assembles components into decoded string
	ostringstream os;
	os << setw(mnemonic_width) << setfill(' ') << left << mnemonic << "x" << dec << rd << ",x" << dec << rs1 << "," << dec << shamt;

	//Returns decoded string
	return os.str();
}

/**
 * Decodes r-type instruction as a string
 *
 * @param     insn: encoded instruction to decode
 *        mnemonic: mnemonic for specific instruction
 *
 * @return: decoded instruction string
 **/
string rv32i::render_rtype(uint32_t insn, const char* mnemonic) const
{
	//Gets encoded components of instruction
	uint32_t rd = get_rd(insn);
	uint32_t rs1 = get_rs1(insn);
	uint32_t rs2 = get_rs2(insn);

	//Assembles components into decoded string
	ostringstream os;
	os << setw(mnemonic_width) << setfill(' ') << left << mnemonic << "x" << dec << rd << ",x" << dec << rs1 << ",x" << dec << rs2;

	//Returns decoded string
	return os.str();
}

/**
 * Decodes the fence instruction as a string
 *
 * @param insn: encoded instruction to decode
 *
 * @return: decoded instruction string
 **/
string rv32i::render_fence(uint32_t insn) const
{
	//Gets encoded components of instruction
	uint32_t pred = insn & 0x0f000000;
	uint32_t succ = insn & 0x00f00000;
	string pred2 = "";
	string succ2 = "";

	//Gets pred and succ
	if (pred & 0x08000000)
	{
		pred2 += "i";
	}
	if (pred & 0x04000000)
	{
		pred2 += "o";
	}
	if (pred & 0x02000000)
	{
		pred2 += "r";
	}
	if (pred & 0x01000000)
	{
		pred2 += "w";
	}

	if (succ & 0x00800000)
	{
		succ2 += "i";
	}
	if (succ & 0x00400000)
	{
		succ2 += "o";
	}
	if (succ & 0x00200000)
	{
		succ2 += "r";
	}
	if (succ & 0x00100000)
	{
		succ2 += "w";
	}

	//Assembles components into decoded string
	ostringstream os;
	os << setw(mnemonic_width) << setfill(' ') << left << "fence" << pred2 << "," << succ2;

	//Returns decoded string
	return os.str();
}

/**
 * Decodes the ecall instruction as a string
 *
 * @param insn: encoded instruction to decode
 *
 * @return: decoded instruction string
 **/
string rv32i::render_ecall() const
{
	return "ecall";
}

/**
 * Decodes the ebreak instruction as a string
 *
 * @param insn: encoded instruction to decode
 *
 * @return: decoded instruction string
 **/
string rv32i::render_ebreak() const
{
	return "ebreak";
}

/**
 * Decodes i-type spe instruction as a string
 *
 * @param     insn: encoded instruction to decode
 *        mnemonic: mnemonic for specific instruction
 *
 * @return: decoded instruction string
 **/
string rv32i::render_itype_spe(const char* mnemonic) const
{
	return mnemonic;
}
