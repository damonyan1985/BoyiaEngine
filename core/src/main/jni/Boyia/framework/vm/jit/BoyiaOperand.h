/*
 ============================================================================
 Name        : BoyiaOperand.h
 Author      : yanbo
 Version     : BoyiaOperand v1.0
 Copyright   : All Copyright Reserved
 Date        : 2018-5-14
 Description : Arm机器语言编码工具,该部分代码主要借鉴于V8引擎
 assembler部分
 ============================================================================
 */
#ifndef BoyiaOperand_h
#define BoyiaOperand_h

#include "BoyiaOpcode.h"

namespace boyia {
struct BoyiaRegister {
	// 主要寄存器有16个
	static const int kNumBoyiaRegisters = 16;
	// Unfortunately we can't make this private in a struct.
	bool is_valid() const {
		return 0 <= code_ && code_ < kNumBoyiaRegisters;
	}
	bool is(BoyiaRegister reg) const {
		return code_ == reg.code_;
	}
	int code() const {
		return code_;
	}
	int code_;
};

// These constants are used in several locations, including static initializers
const int kBoyiaRegister_no_reg_Code = -1;
const int kBoyiaRegister_r0_Code = 0;
const int kBoyiaRegister_r1_Code = 1;
const int kBoyiaRegister_r2_Code = 2;
const int kBoyiaRegister_r3_Code = 3;
const int kBoyiaRegister_r4_Code = 4;
const int kBoyiaRegister_r5_Code = 5;
const int kBoyiaRegister_r6_Code = 6;
const int kBoyiaRegister_r7_Code = 7;
const int kBoyiaRegister_r8_Code = 8;
const int kBoyiaRegister_r9_Code = 9;
const int kBoyiaRegister_r10_Code = 10;
const int kBoyiaRegister_fp_Code = 11;
const int kBoyiaRegister_ip_Code = 12;
const int kBoyiaRegister_sp_Code = 13;
const int kBoyiaRegister_lr_Code = 14;
const int kBoyiaRegister_pc_Code = 15;

const BoyiaRegister no_reg = { kBoyiaRegister_no_reg_Code };

const BoyiaRegister r0 = { kBoyiaRegister_r0_Code };
const BoyiaRegister r1 = { kBoyiaRegister_r1_Code };
const BoyiaRegister r2 = { kBoyiaRegister_r2_Code };
const BoyiaRegister r3 = { kBoyiaRegister_r3_Code };
const BoyiaRegister r4 = { kBoyiaRegister_r4_Code };
const BoyiaRegister r5 = { kBoyiaRegister_r5_Code };
const BoyiaRegister r6 = { kBoyiaRegister_r6_Code };
const BoyiaRegister r7 = { kBoyiaRegister_r7_Code };
// Used as context BoyiaRegister.
const BoyiaRegister r8 = { kBoyiaRegister_r8_Code };
// Used as lithium codegen scratch BoyiaRegister.
const BoyiaRegister r9 = { kBoyiaRegister_r9_Code };
// Used as roots BoyiaRegister.
const BoyiaRegister r10 = { kBoyiaRegister_r10_Code };
const BoyiaRegister fp  = { kBoyiaRegister_fp_Code };
const BoyiaRegister ip  = { kBoyiaRegister_ip_Code };
const BoyiaRegister sp  = { kBoyiaRegister_sp_Code };
const BoyiaRegister lr  = { kBoyiaRegister_lr_Code };
const BoyiaRegister pc  = { kBoyiaRegister_pc_Code };

class BoyiaOperand {
public:
	BoyiaOperand(LInt32 immediate, int mode);
	BoyiaOperand(int value);
	BoyiaOperand(BoyiaRegister rm);
	// rm <shift_op> shift_imm
	BoyiaOperand(BoyiaRegister rm, ShiftOp shift_op, int shift_imm);

	// rm <shift_op> rs
	BoyiaOperand(BoyiaRegister rm, ShiftOp shift_op, BoyiaRegister rs);

	bool is_reg() const;

	LInt32 immediate() const {
		return imm32_;
	}
	BoyiaRegister rm() const {
		return rm_;
	}
	BoyiaRegister rs() const {
		return rs_;
	}

	ShiftOp shiftOp() const {
		return shift_op_;
	}

	int shiftImm() const {
		return shift_imm_;
	}

private:
	BoyiaRegister rm_;
	BoyiaRegister rs_;
	ShiftOp shift_op_;
	int shift_imm_; // valid if rm_ != no_reg && rs_ == no_reg
	LInt32 imm32_; // valid if rm_ == no_reg
	int rmode_;
};

class MemOperand {
public:
	// [rn +/- offset]      Offset/NegOffset
	// [rn +/- offset]!     PreIndex/NegPreIndex
	// [rn], +/- offset     PostIndex/NegPostIndex
	// offset is any signed 32-bit value; offset is first loaded to BoyiaRegister ip if
	// it does not fit the addressing mode (12-bit unsigned and sign bit)
	MemOperand(BoyiaRegister rn, LInt32 offset = 0, AddrMode am = Offset);

	// [rn +/- rm]          Offset/NegOffset
	// [rn +/- rm]!         PreIndex/NegPreIndex
	// [rn], +/- rm         PostIndex/NegPostIndex
	MemOperand(BoyiaRegister rn, BoyiaRegister rm, AddrMode am = Offset);

	// [rn +/- rm <shift_op> shift_imm]      Offset/NegOffset
	// [rn +/- rm <shift_op> shift_imm]!     PreIndex/NegPreIndex
	// [rn], +/- rm <shift_op> shift_imm     PostIndex/NegPostIndex
	MemOperand(BoyiaRegister rn, BoyiaRegister rm, ShiftOp shift_op,
			int shift_imm, AddrMode am = Offset);

	void set_offset(LInt32 offset) {
		offset_ = offset;
	}

	LInt32 offset() const {
		return offset_;
	}

	BoyiaRegister rn() const {
		return rn_;
	}
	BoyiaRegister rm() const {
		return rm_;
	}
	AddrMode am() const {
		return am_;
	}

	ShiftOp shiftOp() const {
		return shift_op_;
	}

	int shiftImm() const {
		return shift_imm_;
	}

private:
	BoyiaRegister rn_; // base
	BoyiaRegister rm_; // BoyiaRegister offset
	LInt32 offset_; // valid if rm_ == no_reg
	ShiftOp shift_op_;
	int shift_imm_; // valid if rm_ != no_reg && rs_ == no_reg
	AddrMode am_; // bits P, U, and W
};

class Label {
public:
	enum Distance {
		kNear, kFar
	};

	Label() {
	    Unuse();
	    UnuseNear();
	}

	inline void Unuse() { pos_ = 0; }
	inline void UnuseNear() {near_link_pos_ = 0;}

	inline bool is_bound() const {return pos_ < 0;}
	inline bool is_unused() const {return pos_ == 0 && near_link_pos_ == 0;}
	inline bool is_linked() const {return pos_ > 0;}
    inline bool is_near_linked() const {return near_link_pos_ > 0;}

	void bind_to(int pos) {
		pos_ = -pos - 1;
	}

	void link_to(int pos, Distance distance = kFar) {
		if (distance == kNear) {
			near_link_pos_ = pos + 1;
		} else {
			pos_ = pos + 1;
		}
	}

	int pos() const;
	int near_link_pos() const {
		return near_link_pos_ - 1;
	}

private:
	int pos_;
	int near_link_pos_;
};
}

#endif
