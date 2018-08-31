/*
 ============================================================================
 Name        : MiniAssembler.h
 Author      : yanbo
 Version     : MiniAssembler v1.0
 Copyright   : All Copyright Reserved
 Date        : 2018-5-14
 Description : Arm机器语言编码工具,该部分代码主要借鉴于
 V8引擎assembler部分代码, 并对V8过于复杂的过程进行简化
 ============================================================================
 */

#ifndef MiniAssembler_h
#define MiniAssembler_h

#include "MiniOperand.h"

namespace mjs {
class MiniAssembler {
public:
	MiniAssembler();
	// Load/Store instructions
	void ldr(MiniRegister dst, const MemOperand& src, Condition cond = al);
	void str(MiniRegister src, const MemOperand& dst, Condition cond = al);
	void ldrb(MiniRegister dst, const MemOperand& src, Condition cond = al);
	void strb(MiniRegister src, const MemOperand& dst, Condition cond = al);

	void mov(MiniRegister dst, const MiniOperand& src, SBit s = LeaveCC,
			Condition cond = al);
	void mov(MiniRegister dst, MiniRegister src, SBit s = LeaveCC,
			Condition cond = al) {
		mov(dst, MiniOperand(src), s, cond);
	}

	// ARMv7 instructions for loading a 32 bit immediate in two instructions.
	// This may actually emit a different mov instruction, but on an ARMv7 it
	// is guaranteed to only emit one instruction.
	void movw(MiniRegister reg, LUint32 immediate, Condition cond = al);
	// The constant for movt should be in the range 0-0xffff.
	void movt(MiniRegister reg, LUint32 immediate, Condition cond = al);

	void addrmod1(Instr instr, MiniRegister rn, MiniRegister rd,
			const MiniOperand& x);
	void addrmod2(Instr instr, MiniRegister rd, const MemOperand& x);

	void b(int branch_offset, Condition cond);
	// Convenience branch instructions using labels
	void b(Label* L, Condition cond = al) {
		b(branch_offset(L), cond);
	}
	void bl(int branch_offset, Condition cond);
	void blx(int branch_offset);
	void blx(MiniRegister target, Condition cond);
	void bx(MiniRegister target, Condition cond);

	void and_(MiniRegister dst, MiniRegister src1, const MiniOperand& src2,
			SBit s = LeaveCC, Condition cond = al);

	void orr(MiniRegister dst, MiniRegister src1, const MiniOperand& src2,
			SBit s = LeaveCC, Condition cond = al);

	void sub(MiniRegister dst, MiniRegister src1, const MiniOperand& src2,
			SBit s = LeaveCC, Condition cond = al);

	void add(MiniRegister dst, MiniRegister src1, const MiniOperand& src2,
			SBit s = LeaveCC, Condition cond = al);

	// Multiply instructions.
	void mul(MiniRegister dst, MiniRegister src1, MiniRegister src2,
			SBit s = LeaveCC, Condition cond = al);

	void cmp(MiniRegister src1, const MiniOperand& src2, Condition cond = al);

	// 堆栈操作
	void push(MiniRegister src, Condition cond = al) {
		str(src, MemOperand(sp, 4, NegPreIndex), cond);
	}

	void pop(MiniRegister dst, Condition cond = al) {
		ldr(dst, MemOperand(sp, 4, PostIndex), cond);
	}

	void pop() {
		add(sp, sp, MiniOperand(kPointerSize));
	}

	void drop(int count, Condition cond = al);
	void ret(int drop, Condition cond = al);
	void ret(Condition cond = al);
	// 数据定义
	void db(LUint8 data);
	void dd(LUint32 data);

	void bind(Label* L);
	void bind_to(Label* L, int pos);
	void next(Label* L);
	int pc_offset() const;
	int branch_offset(Label* L);
	int target_at(int pos);
	void target_at_put(int pos, int target_pos);
	void instr_at_put(int pos, Instr instr);
	Instr instr_at(int pos);

	void emit(Instr x);
	Instr EncodeMovwImmediate(LUint32 immediate);

	void getCode(CodeDesc* desc);

public:
	CodeDesc m_code;
	LByte* m_pc;
};
}
#endif
