/*
 ============================================================================
 Name        : MiniOperand.h
 Author      : yanbo
 Version     : MiniOperand v1.0
 Copyright   : All Copyright Reserved
 Date        : 2018-5-14
 Description : Arm机器语言编码工具,该部分代码主要借鉴于V8引擎
 assembler部分
 ============================================================================
 */
#ifndef MiniOperand_h
#define MiniOperand_h

#include "MiniOpcode.h"

namespace mjs {
struct MiniRegister {
	// 主要寄存器有16个
	static const int kNumMiniRegisters = 16;
	// Unfortunately we can't make this private in a struct.
	bool is_valid() const {
		return 0 <= code_ && code_ < kNumMiniRegisters;
	}
	bool is(MiniRegister reg) const {
		return code_ == reg.code_;
	}
	int code() const {
		return code_;
	}
	int code_;
};

// These constants are used in several locations, including static initializers
const int kMiniRegister_no_reg_Code = -1;
const int kMiniRegister_r0_Code = 0;
const int kMiniRegister_r1_Code = 1;
const int kMiniRegister_r2_Code = 2;
const int kMiniRegister_r3_Code = 3;
const int kMiniRegister_r4_Code = 4;
const int kMiniRegister_r5_Code = 5;
const int kMiniRegister_r6_Code = 6;
const int kMiniRegister_r7_Code = 7;
const int kMiniRegister_r8_Code = 8;
const int kMiniRegister_r9_Code = 9;
const int kMiniRegister_r10_Code = 10;
const int kMiniRegister_fp_Code = 11;
const int kMiniRegister_ip_Code = 12;
const int kMiniRegister_sp_Code = 13;
const int kMiniRegister_lr_Code = 14;
const int kMiniRegister_pc_Code = 15;

const MiniRegister no_reg = { kMiniRegister_no_reg_Code };

const MiniRegister r0 = { kMiniRegister_r0_Code };
const MiniRegister r1 = { kMiniRegister_r1_Code };
const MiniRegister r2 = { kMiniRegister_r2_Code };
const MiniRegister r3 = { kMiniRegister_r3_Code };
const MiniRegister r4 = { kMiniRegister_r4_Code };
const MiniRegister r5 = { kMiniRegister_r5_Code };
const MiniRegister r6 = { kMiniRegister_r6_Code };
const MiniRegister r7 = { kMiniRegister_r7_Code };
// Used as context MiniRegister.
const MiniRegister r8 = { kMiniRegister_r8_Code };
// Used as lithium codegen scratch MiniRegister.
const MiniRegister r9 = { kMiniRegister_r9_Code };
// Used as roots MiniRegister.
const MiniRegister r10 = { kMiniRegister_r10_Code };
const MiniRegister fp = { kMiniRegister_fp_Code };
const MiniRegister ip = { kMiniRegister_ip_Code };
const MiniRegister sp = { kMiniRegister_sp_Code };
const MiniRegister lr = { kMiniRegister_lr_Code };
const MiniRegister pc = { kMiniRegister_pc_Code };

class MiniOperand {
public:
	MiniOperand(LInt32 immediate, int mode);
	MiniOperand(int value);
	MiniOperand(MiniRegister rm);
	// rm <shift_op> shift_imm
	MiniOperand(MiniRegister rm, ShiftOp shift_op, int shift_imm);

	// rm <shift_op> rs
	MiniOperand(MiniRegister rm, ShiftOp shift_op, MiniRegister rs);

	bool is_reg() const;

	LInt32 immediate() const {
		return imm32_;
	}
	MiniRegister rm() const {
		return rm_;
	}
	MiniRegister rs() const {
		return rs_;
	}

	ShiftOp shiftOp() const {
		return shift_op_;
	}

	int shiftImm() const {
		return shift_imm_;
	}

private:
	MiniRegister rm_;
	MiniRegister rs_;
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
	// offset is any signed 32-bit value; offset is first loaded to MiniRegister ip if
	// it does not fit the addressing mode (12-bit unsigned and sign bit)
	MemOperand(MiniRegister rn, LInt32 offset = 0, AddrMode am = Offset);

	// [rn +/- rm]          Offset/NegOffset
	// [rn +/- rm]!         PreIndex/NegPreIndex
	// [rn], +/- rm         PostIndex/NegPostIndex
	MemOperand(MiniRegister rn, MiniRegister rm, AddrMode am = Offset);

	// [rn +/- rm <shift_op> shift_imm]      Offset/NegOffset
	// [rn +/- rm <shift_op> shift_imm]!     PreIndex/NegPreIndex
	// [rn], +/- rm <shift_op> shift_imm     PostIndex/NegPostIndex
	MemOperand(MiniRegister rn, MiniRegister rm, ShiftOp shift_op,
			int shift_imm, AddrMode am = Offset);

	void set_offset(LInt32 offset) {
		offset_ = offset;
	}

	LInt32 offset() const {
		return offset_;
	}

	MiniRegister rn() const {
		return rn_;
	}
	MiniRegister rm() const {
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
	MiniRegister rn_; // base
	MiniRegister rm_; // MiniRegister offset
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
