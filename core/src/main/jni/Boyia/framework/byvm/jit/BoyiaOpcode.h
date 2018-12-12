/*
 ============================================================================
 Name        : BoyiaOpcode.h
 Author      : yanbo
 Version     : MiniOpcode v1.0
 Copyright   : All Copyright Reserved
 Date        : 2018-5-14
 Description : Arm机器语言编码工具,该部分代码主要借鉴于V8引擎
 assembler部分
 ============================================================================
 */

#ifndef BoyiaOpcode_h
#define BoyiaOpcode_h

#include "PlatformLib.h"

namespace boyia {
typedef LInt32 Instr;

const LInt KB = 1024;
const LInt MB = KB * KB;
const LInt GB = KB * KB * KB;
const LInt kMaxInt = 0x7FFFFFFF;
const LInt kMinInt = -kMaxInt - 1;

const LUint32 kMaxUInt32 = 0xFFFFFFFFu;

const int kPointerSize  = sizeof(void*);

struct CodeDesc {
	LByte* buffer;
	LInt buffer_size;
	LInt instr_size;
	LInt reloc_size;
};

// 机器语言opcode编码
enum Condition {
	kNoCondition = -1,

	eq = 0 << 28, // Z set            Equal.
	ne = 1 << 28, // Z clear          Not equal.
	cs = 2 << 28, // C set            Unsigned higher or same.
	cc = 3 << 28, // C clear          Unsigned lower.
	mi = 4 << 28, // N set            Negative.
	pl = 5 << 28, // N clear          Positive or zero.
	vs = 6 << 28, // V set            Overflow.
	vc = 7 << 28, // V clear          No overflow.
	hi = 8 << 28, // C set, Z clear   Unsigned higher.
	ls = 9 << 28, // C clear or Z set Unsigned lower or same.
	ge = 10 << 28, // N == V           Greater or equal.
	lt = 11 << 28, // N != V           Less than.
	gt = 12 << 28, // Z clear, N == V  Greater than.
	le = 13 << 28, // Z set or N != V  Less then or equal
	al = 14 << 28, //                  Always.

	kSpecialCondition = 15 << 28, // Special condition (refer to section A3.2.1).
	kNumberOfConditions = 16,

	// Aliases.
	hs = cs, // C set            Unsigned higher or same.
	lo = cc // C clear          Unsigned lower.
};

// Opcodes for Data-processing instructions (instructions with a type 0 and 1)
// as defined in section A3.4
enum Opcode {
	AND = 0 << 21, // Logical AND.
	EOR = 1 << 21, // Logical Exclusive OR.
	SUB = 2 << 21, // Subtract.
	RSB = 3 << 21, // Reverse Subtract.
	ADD = 4 << 21, // Add.
	ADC = 5 << 21, // Add with Carry.
	SBC = 6 << 21, // Subtract with Carry.
	RSC = 7 << 21, // Reverse Subtract with Carry.
	TST = 8 << 21, // Test.
	TEQ = 9 << 21, // Test Equivalence.
	CMP = 10 << 21, // Compare.
	CMN = 11 << 21, // Compare Negated.
	ORR = 12 << 21, // Logical (inclusive) OR.
	MOV = 13 << 21, // Move.
	BIC = 14 << 21, // Bit Clear.
	MVN = 15 << 21 // Move Not.
};

// The bits for bit 7-4 for some type 0 miscellaneous instructions.
enum MiscInstructionsBits74 {
	// With bits 22-21 01.
	BX = 1 << 4,
	BXJ = 2 << 4,
	BLX = 3 << 4,
	BKPT = 7 << 4,

	// With bits 22-21 11.
	CLZ = 1 << 4
};

// Instruction encoding bits and masks.
enum {
	H = 1 << 5, // Halfword (or byte).
	S6 = 1 << 6, // Signed (or unsigned).
	L = 1 << 20, // Load (or store).
	S = 1 << 20, // Set condition code (or leave unchanged).
	W = 1 << 21, // Writeback base register (or leave unchanged).
	A = 1 << 21, // Accumulate in multiply instruction (or not).
	B = 1 << 22, // Unsigned byte (or word).
	N = 1 << 22, // Long (or short).
	U = 1 << 23, // Positive (or negative) offset/index.
	P = 1 << 24, // Offset/pre-indexed addressing (or post-indexed addressing).
	I = 1 << 25, // Immediate shifter operand (or not).

	B4 = 1 << 4,
	B5 = 1 << 5,
	B6 = 1 << 6,
	B7 = 1 << 7,
	B8 = 1 << 8,
	B9 = 1 << 9,
	B12 = 1 << 12,
	B16 = 1 << 16,
	B18 = 1 << 18,
	B19 = 1 << 19,
	B20 = 1 << 20,
	B21 = 1 << 21,
	B22 = 1 << 22,
	B23 = 1 << 23,
	B24 = 1 << 24,
	B25 = 1 << 25,
	B26 = 1 << 26,
	B27 = 1 << 27,
	B28 = 1 << 28,

	// Instruction bit masks.
	kCondMask = 15 << 28,
	kALUMask = 0x6f << 21,
	kRdMask = 15 << 12, // In str instruction.
	kCoprocessorMask = 15 << 8,
	kOpCodeMask = 15 << 21, // In data-processing instructions.
	kImm24Mask = (1 << 24) - 1,
	kOff12Mask = (1 << 12) - 1
};

// Condition code updating mode.
enum SBit {
	SetCC = 1 << 20, // Set condition code.
	LeaveCC = 0 << 20 // Leave condition code unchanged.
};

// Status register selection.
enum SRegister {
	CPSR = 0 << 22, SPSR = 1 << 22
};

// Shifter types for Data-processing operands as defined in section A5.1.2.
enum ShiftOp {
	LSL = 0 << 5, // Logical shift left.
	LSR = 1 << 5, // Logical shift right.
	ASR = 2 << 5, // Arithmetic shift right.
	ROR = 3 << 5, // Rotate right.

	// RRX is encoded as ROR with shift_imm == 0.
	// Use a special code to make the distinction. The RRX ShiftOp is only used
	// as an argument, and will never actually be encoded. The Assembler will
	// detect it and emit the correct ROR shift operand with shift_imm == 0.
	RRX = -1,
	kNumberOfShifts = 4
};

// Status register fields.
enum SRegisterField {
	CPSR_c = CPSR | 1 << 16,
	CPSR_x = CPSR | 1 << 17,
	CPSR_s = CPSR | 1 << 18,
	CPSR_f = CPSR | 1 << 19,
	SPSR_c = SPSR | 1 << 16,
	SPSR_x = SPSR | 1 << 17,
	SPSR_s = SPSR | 1 << 18,
	SPSR_f = SPSR | 1 << 19
};

// Memory operand addressing mode.
enum AddrMode {
	// Bit encoding P U W.
	Offset = (8 | 4 | 0) << 21, // Offset (without writeback to base).
	PreIndex = (8 | 4 | 1) << 21, // Pre-indexed addressing with writeback.
	PostIndex = (0 | 4 | 0) << 21, // Post-indexed addressing with writeback.
	NegOffset = (8 | 0 | 0) << 21, // Negative offset (without writeback to base).
	NegPreIndex = (8 | 0 | 1) << 21, // Negative pre-indexed with writeback.
	NegPostIndex = (0 | 0 | 0) << 21 // Negative post-indexed with writeback.
};

// Load/store multiple addressing mode.
enum BlockAddrMode {
	// Bit encoding P U W .
	da = (0 | 0 | 0) << 21, // Decrement after.
	ia = (0 | 4 | 0) << 21, // Increment after.
	db = (8 | 0 | 0) << 21, // Decrement before.
	ib = (8 | 4 | 0) << 21, // Increment before.
	da_w = (0 | 0 | 1) << 21, // Decrement after with writeback to base.
	ia_w = (0 | 4 | 1) << 21, // Increment after with writeback to base.
	db_w = (8 | 0 | 1) << 21, // Decrement before with writeback to base.
	ib_w = (8 | 4 | 1) << 21, // Increment before with writeback to base.

	// Alias modes for comparison when writeback does not matter.
	da_x = (0 | 0 | 0) << 21, // Decrement after.
	ia_x = (0 | 4 | 0) << 21, // Increment after.
	db_x = (8 | 0 | 0) << 21, // Decrement before.
	ib_x = (8 | 4 | 0) << 21, // Increment before.

	kBlockAddrModeMask = (8 | 4 | 1) << 21
};

// Coprocessor load/store operand size.
enum LFlag {
	Long = 1 << 22, // Long load/store coprocessor.
	Short = 0 << 22 // Short load/store coprocessor.
};

// -----------------------------------------------------------------------------
// Supervisor Call (svc) specific support.

// Special Software Interrupt codes when used in the presence of the ARM
// simulator.
// svc (formerly swi) provides a 24bit immediate value. Use bits 22:0 for
// standard SoftwareInterrupCode. Bit 23 is reserved for the stop feature.
enum SoftwareInterruptCodes {
	// transition to C code
	kCallRtRedirected = 0x10,
	// break point
	kBreakpoint = 0x20,
	// stop
	kStopCode = 1 << 23
};

inline Condition NegateCondition(Condition cond) {
	return static_cast<Condition>(cond ^ ne);
}

static Condition ReverseCondition(Condition cond);

class BoyiaInstruction {
public:
	enum {
		kInstrSize = 4, kInstrSizeLog2 = 2, kPCReadOffset = 8
	};

#define DECLARE_STATIC_TYPED_ACCESSOR(return_type, Name)                     \
    static inline return_type Name(Instr instr) {                              \
      char* temp = reinterpret_cast<char*>(&instr);                            \
      return reinterpret_cast<BoyiaInstruction*>(temp)->Name();                     \
    }

	// Get the raw instruction bits.
	inline Instr InstructionBits() const {
		return *reinterpret_cast<const Instr*>(this);
	}

	inline int BitField(int hi, int lo) const {
		return InstructionBits() & (((2 << (hi - lo)) - 1) << lo);
	}

	inline Condition ConditionField() const {
		return static_cast<Condition>(BitField(31, 28));
	}

	DECLARE_STATIC_TYPED_ACCESSOR(Condition, ConditionField);

	// Read a bit field's value out of the instruction bits.
	inline int Bits(int hi, int lo) const {
		return (InstructionBits() >> lo) & ((2 << (hi - lo)) - 1);
	}

	// Read one particular bit out of the instruction bits.
	static inline int Bit(Instr instr, int nr) {
		return (instr >> nr) & 1;
	}
};
}
#endif
