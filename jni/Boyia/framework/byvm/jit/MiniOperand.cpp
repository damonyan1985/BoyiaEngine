#include "MiniOperand.h"

namespace mjs {
MiniOperand::MiniOperand(LInt32 immediate, int rmode) {
	rm_ = no_reg;
	imm32_ = immediate;
	rmode_ = rmode;
}

MiniOperand::MiniOperand(int value) {
	rm_ = no_reg;
	imm32_ = value;
	rmode_ = 0;
}

MiniOperand::MiniOperand(MiniRegister rm) {
	rm_ = rm;
	rs_ = no_reg;
	shift_op_ = LSL;
	shift_imm_ = 0;
}

MiniOperand::MiniOperand(MiniRegister rm, ShiftOp shift_op, int shift_imm) {
	//ASSERT(is_uint5(shift_imm));
	//ASSERT(shift_op != ROR || shift_imm != 0);  // use RRX if you mean it
	rm_ = rm;
	rs_ = no_reg;
	shift_op_ = shift_op;
	shift_imm_ = shift_imm & 31;
	if (shift_op == RRX) {
		// encoded as ROR with shift_imm == 0
		//ASSERT(shift_imm == 0);
		shift_op_ = ROR;
		shift_imm_ = 0;
	}
}

MiniOperand::MiniOperand(MiniRegister rm, ShiftOp shift_op, MiniRegister rs) {
	//ASSERT(shift_op != RRX);
	rm_ = rm;
	rs_ = no_reg;
	shift_op_ = shift_op;
	rs_ = rs;
}

bool MiniOperand::is_reg() const {
	return rm_.is_valid() && rs_.is(no_reg) && shift_op_ == LSL
			&& shift_imm_ == 0;
}

MemOperand::MemOperand(MiniRegister rn, LInt32 offset, AddrMode am) {
	rn_ = rn;
	rm_ = no_reg;
	offset_ = offset;
	am_ = am;
}

MemOperand::MemOperand(MiniRegister rn, MiniRegister rm, AddrMode am) {
	rn_ = rn;
	rm_ = rm;
	shift_op_ = LSL;
	shift_imm_ = 0;
	am_ = am;
}

MemOperand::MemOperand(MiniRegister rn, MiniRegister rm, ShiftOp shift_op,
		int shift_imm, AddrMode am) {
	//ASSERT(is_uint5(shift_imm));
	rn_ = rn;
	rm_ = rm;
	shift_op_ = shift_op;
	shift_imm_ = shift_imm & 31;
	am_ = am;
}

int Label::pos() const {
	if (pos_ < 0) return -pos_ - 1;
	if (pos_ > 0) return  pos_ - 1;
	return 0;
}

}
