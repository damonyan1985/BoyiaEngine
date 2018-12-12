#include "BoyiaAssembler.h"
#include "BoyiaAsmUtils.h"
#include <android/log.h>

namespace boyia {

const int kMaxCodeSize = 100 * KB;
const int kInstrSize = sizeof(Instr);

BoyiaAssembler::BoyiaAssembler() {
	m_code.buffer = new LByte[kMaxCodeSize];
	m_code.buffer_size = 0;
	m_code.instr_size = 0;
	m_code.reloc_size = 0;

	m_pc = m_code.buffer;
}

void BoyiaAssembler::ldr(BoyiaRegister dst, const MemOperand& src,
		Condition cond) {
	addrmod2(cond | B26 | L, dst, src);
}
void BoyiaAssembler::str(BoyiaRegister src, const MemOperand& dst,
		Condition cond) {
	addrmod2(cond | B26, src, dst);
}
void BoyiaAssembler::ldrb(BoyiaRegister dst, const MemOperand& src,
		Condition cond) {
	addrmod2(cond | B26 | B | L, dst, src);
}
void BoyiaAssembler::strb(BoyiaRegister src, const MemOperand& dst,
		Condition cond) {
	addrmod2(cond | B26 | B, src, dst);
}

/* MOV指令格式                                                                                                                                          |shift(Imm, Rs)| (imm8, Rm)]
 * [cond(4bits)|00|I(1为立即数，0为寄存器)|opcode(1101)(4bits)|S|Rn(4bits)|Rd(4bits)|shiftOP(4bits)|imm8(8bits)]
 * rd，目标寄存器，src数据源
 * 目前该函数只针对寄存器之间的传值和立即数为8位的传值
 * 大立即数的传递由ldr从内存中获取完成
 */
void BoyiaAssembler::mov(BoyiaRegister rd, const BoyiaOperand& src, SBit s,
		Condition cond) {

	Instr instr = cond | MOV | s | rd.code() * B12;
	// 如果是寄存器寻址，则将SRC中的值装入DST
	if (src.rm().is_valid()) {
		instr |= src.shiftOp() | src.rm().code(); // 只有寄存器寻址才有shift
		if (src.rs().is_valid()) { //  通过寄存器来移位，如 MOV Rd, Rm LSL Rs
			instr |= src.rs().code() * B8 | B4;
		} else { //  通过立即数来移位，如 MOV Rd, Rm LSL #1
			instr |= src.shiftImm() * B7;
		}
	} else {
		// 如果是立即数，其中25位上的bit位要设置为1, Operand是占了12位,
		// 其中bit11-bit8是移位数(rotate),bit7-0是一个8位的立即数(imm8)
		if (src.immediate() <= 0xFF) { // imm8
			instr |= I | src.immediate();
		} else {
			// 转换成多条指令, 据说也可以通过ldr来实现，具体实现方式不知
			LUint32 i = src.immediate();
			LUint32 low8 = i & 0xFF;
			LUint32 low16 = (i & 0xFF00) >> 8;
			LUint32 immHigh = (i & 0xFFFF0000) >> 16;
			//__android_log_print(ANDROID_LOG_INFO, "MiniJS", "MiniJS call low8=%d", low8);

			//__android_log_print(ANDROID_LOG_INFO, "MiniJS", "MiniJS call immHigh=%d", immHigh);
			if (low16) { // 8-15位数，通过(rotate=12)*2,右循环移位24
				low16 = low16 | (12 << 8);
				//__android_log_print(ANDROID_LOG_INFO, "MiniJS", "MiniJS call low16=%d", low16);
				instr |= I | low16;
				emit(instr);
			}

			if (low8) { // 通过OR运算将低8位传入
				orr(rd, rd, BoyiaOperand(low8), s, cond);
				//add(rd, rd, BoyiaOperand(low8), s, cond);
			}

			if (immHigh) { // 通过movt指令传入高16位
				movt(rd, immHigh, cond);
			}

			return;
		}
	}
	emit(instr);
}

void BoyiaAssembler::movw(BoyiaRegister reg, LUint32 immediate, Condition cond) {
}

// 存储到寄存器高16位
void BoyiaAssembler::movt(BoyiaRegister reg, LUint32 immediate, Condition cond) {
	emit(cond | 0x34 * B20 | reg.code() * B12 | EncodeMovwImmediate(immediate));
}

// 硬性规定超过8位的立即数需要通过寄存器来进行运算，先mov，后计算
void BoyiaAssembler::addrmod1(Instr instr, BoyiaRegister rn, BoyiaRegister rd,
		const BoyiaOperand& src) {
	instr |= rn.code() * B16 | rd.code() * B12;
	if (src.rm().is_valid()) {
		instr |= src.rm().code() | src.shiftOp();
		if (src.rs().is_valid()) { //  通过寄存器来移位，如 MOV Rd, Rm LSL Rs
			instr |= src.rs().code() * B8 | B4;
		} else { //  通过立即数来移位，如 MOV Rd, Rm LSL #1
			instr |= src.shiftImm() * B7;
		}
	} else {
		if (src.immediate() <= 0xFF) { // imm8
			instr |= I | src.immediate();
		}
	}

	emit(instr);
}

void BoyiaAssembler::addrmod2(Instr instr, BoyiaRegister rd,
		const MemOperand& x) {
	int am = x.am();
	if (!x.rm().is_valid()) {
		// Immediate offset.
		int offset_12 = x.offset();
		if (offset_12 < 0) {
			offset_12 = -offset_12;
			am ^= U;
		}
		if (!is_uint12(offset_12)) {
			// Immediate offset cannot be encoded, load it first to BoyiaRegister ip
			// rn (and rd in a load) should never be ip, or will be trashed.
			// ASSERT(!x.rn_.is(ip) && ((instr & L) == L || !rd.is(ip)));
			// ldr指令只能编码12位的立即数偏移地址，如果大于12位，则需要将立即数移至
			// 寄存器中寻址
			mov(ip, BoyiaOperand(x.offset()), LeaveCC,
					BoyiaInstruction::ConditionField(instr));
			addrmod2(instr, rd, MemOperand(x.rn(), ip, x.am()));
			return;
		}
		//ASSERT(offset_12 >= 0);  // no masking needed
		instr |= offset_12;
	} else {
		instr |= B25 | x.shiftImm() * B7 | x.shiftOp() | x.rm().code();
	}

	emit(instr | am | x.rn().code() * B16 | rd.code() * B12);
}

void BoyiaAssembler::b(int branch_offset, Condition cond) {
//	ASSERT((branch_offset & 3) == 0);
	int imm24 = branch_offset >> 2;
	emit(cond | B27 | B25 | (imm24 & kImm24Mask));
}

void BoyiaAssembler::bl(int branch_offset, Condition cond) {
	//positions_recorder()->WriteRecordedPositions();
	//ASSERT((branch_offset & 3) == 0);
	int imm24 = branch_offset >> 2;
	//ASSERT(is_int24(imm24));
	emit(cond | B27 | B25 | B24 | (imm24 & kImm24Mask));
}

void BoyiaAssembler::blx(int branch_offset) {
	//positions_recorder()->WriteRecordedPositions();
	//ASSERT((branch_offset & 1) == 0);
	int h = ((branch_offset & 2) >> 1) * B24;
	int imm24 = branch_offset >> 2;
	//ASSERT(is_int24(imm24));
	emit(kSpecialCondition | B27 | B25 | h | (imm24 & kImm24Mask));
}

void BoyiaAssembler::blx(BoyiaRegister target, Condition cond) {
	emit(
			cond | B24 | B21 | 15 * B16 | 15 * B12 | 15 * B8 | BLX
					| target.code());
}

void BoyiaAssembler::bx(BoyiaRegister target, Condition cond) {
	//positions_recorder()->WriteRecordedPositions();
	//ASSERT(!target.is(pc));  // use of pc is actually allowed, but discouraged
	emit(cond | B24 | B21 | 15 * B16 | 15 * B12 | 15 * B8 | BX | target.code());
}

void BoyiaAssembler::and_(BoyiaRegister rd, BoyiaRegister rn,
		const BoyiaOperand& op, SBit s, Condition cond) {
	addrmod1(cond | AND | s, rn, rd, op);
}

void BoyiaAssembler::orr(BoyiaRegister rd, BoyiaRegister rn, const BoyiaOperand& op,
		SBit s, Condition cond) {
//	Instr instr = cond | ORR | s | rn.code() * B16 | rd.code() * B12
//			| op.shiftOp();
	addrmod1(cond | ORR | s, rn, rd, op);
}

void BoyiaAssembler::sub(BoyiaRegister dst, BoyiaRegister src1,
		const BoyiaOperand& src2, SBit s, Condition cond) {
	addrmod1(cond | SUB | s, src1, dst, src2);
}

void BoyiaAssembler::add(BoyiaRegister dst, BoyiaRegister src1,
		const BoyiaOperand& src2, SBit s, Condition cond) {
	addrmod1(cond | ADD | s, src1, dst, src2);
}

void BoyiaAssembler::cmp(BoyiaRegister src1, const BoyiaOperand& src2,
		Condition cond) {
	addrmod1(cond | CMP | S, src1, r0, src2);
}

void BoyiaAssembler::mul(BoyiaRegister dst, BoyiaRegister src1, BoyiaRegister src2,
		SBit s, Condition cond) {
	emit(
			cond | s | dst.code() * B16 | src2.code() * B8 | B7 | B4
					| src1.code());
}

void BoyiaAssembler::db(LUint8 data) {
	*reinterpret_cast<LUint8*>(m_pc) = data;
	m_pc += sizeof(LUint8);
}

void BoyiaAssembler::dd(LUint32 data) {
	*reinterpret_cast<LUint32*>(m_pc) = data;
	m_pc += sizeof(LUint32);
}

void BoyiaAssembler::emit(Instr x) {
	*reinterpret_cast<Instr*>(m_pc) = x;
	m_pc += kInstrSize;
}

Instr BoyiaAssembler::EncodeMovwImmediate(LUint32 immediate) {
	return ((immediate & 0xf000) << 4) | (immediate & 0xfff);
}

void BoyiaAssembler::bind(Label* L) {
	bind_to(L, pc_offset());
}

void BoyiaAssembler::bind_to(Label* L, int pos) {
	//ASSERT(0 <= pos && pos <= pc_offset()); // must have a valid binding position
	// 对之前使用该Label的条件分支指令进行修改
	while (L->is_linked()) {
		int fixup_pos = L->pos();
		next(L); // call next before overwriting link with target at fixup_pos
		// 修改每个已经link label的指令的offset
		target_at_put(fixup_pos, pos);
	}

	// 记录绑定的位置
	L->bind_to(pos);

	// Keep track of the last bound label so we don't eliminate any instructions
	// before a bound label.
	//if (pos > last_bound_pos_)
	//	last_bound_pos_ = pos;
}

int BoyiaAssembler::pc_offset() const {
	return m_pc - m_code.buffer;
}

// Labels refer to positions in the (to be) generated code.
// There are bound, linked, and unused labels.
//
// Bound labels refer to known positions in the already
// generated code. pos() is the position the label refers to.
//
// Linked labels refer to unknown positions in the code
// to be generated; pos() is the position of the last
// instruction using the label.

// The link chain is terminated by a negative code position (must be aligned)
const int kEndOfChain = -4;

// Difference between address of current opcode and value read from pc
// register.
// ARM 3级流水线，取指令，译码，执行，PC寄存器的值位置在
// 为与当前指令的地址相差两个指令长度，即4bytes * 2=8
static const int kPcLoadDelta = 8;
int BoyiaAssembler::branch_offset(Label* L) {
	int target_pos;
	if (L->is_bound()) {
		target_pos = L->pos();
	} else {
		if (L->is_linked()) {
			target_pos = L->pos(); // L's link
		} else {
			target_pos = kEndOfChain;
		}
		// label bind之前记录label离pc的距离
		// label记录下当前pc指针的位置
		L->link_to(pc_offset());
	}

	// Block the emission of the constant pool, since the branch instruction must
	// be emitted at the pc offset recorded by the label.
	//BlockConstPoolFor(1);
	// 应该是如果绑定过Label，则计算当前偏移地址
	return target_pos - (pc_offset() + kPcLoadDelta);
}

int BoyiaAssembler::target_at(int pos) {
	Instr instr = instr_at(pos);
//	if ((instr & ~kImm24Mask) == 0) {
//	    // Emitted label constant, not part of a branch.
//	    return instr - (Code::kHeaderSize - kHeapObjectTag);
//	}
	//ASSERT((instr & 7*B25) == 5*B25);  // b, bl, or blx imm24
	int imm26 = ((instr & kImm24Mask) << 8) >> 6;
	if ((BoyiaInstruction::ConditionField(instr) == kSpecialCondition)
			&& ((instr & B24) != 0)) {
		// blx uses bit 24 to encode bit 2 of imm26
		imm26 += 2;
	}
	return pos + kPcLoadDelta + imm26;
}

void BoyiaAssembler::next(Label* L) {
	//ASSERT(L->is_linked());
	int link = target_at(L->pos());
	if (link == kEndOfChain) {
		L->Unuse();
	} else {
		//ASSERT(link >= 0);
		L->link_to(link);
	}
}

void BoyiaAssembler::target_at_put(int pos, int target_pos) {
	Instr instr = instr_at(pos);
//	  if ((instr & ~kImm24Mask) == 0) {
//	    ASSERT(target_pos == kEndOfChain || target_pos >= 0);
//	    // Emitted label constant, not part of a branch.
//	    // Make label relative to Code* of generated Code object.
//	    instr_at_put(pos, target_pos + (Code::kHeaderSize - kHeapObjectTag));
//	    return;
//	  }
	int imm26 = target_pos - (pos + kPcLoadDelta);
	//ASSERT((instr & 7*B25) == 5*B25);  // b, bl, or blx imm24
	if (BoyiaInstruction::ConditionField(instr) == kSpecialCondition) {
		// blx uses bit 24 to encode bit 2 of imm26
		//ASSERT((imm26 & 1) == 0);
		instr = (instr & ~(B24 | kImm24Mask)) | ((imm26 & 2) >> 1) * B24;
	} else {
		//ASSERT((imm26 & 3) == 0);
		// kImm24Mask=[11...1(24bits)]
		// instr &= ~kImm24Mask;实际上是将24位跳转地址从指令中去除
		instr &= ~kImm24Mask;
	}
	int imm24 = imm26 >> 2;
	//ASSERT(is_int24(imm24));
	instr_at_put(pos, instr | (imm24 & kImm24Mask));
}

void BoyiaAssembler::instr_at_put(int pos, Instr instr) {
	*reinterpret_cast<Instr*>(m_code.buffer + pos) = instr;
}

Instr BoyiaAssembler::instr_at(int pos) {
	return *reinterpret_cast<Instr*>(m_code.buffer + pos);
}

void BoyiaAssembler::getCode(CodeDesc* desc) {
	desc->buffer = m_code.buffer;
	desc->instr_size = pc_offset();
	//LMemcpy(desc->buffer, m_code.buffer, pc_offset());
}

void BoyiaAssembler::drop(int count, Condition cond) {
	if (count > 0) {
		add(sp, sp, BoyiaOperand(count * kPointerSize), LeaveCC, cond);
	}
}

void BoyiaAssembler::ret(int count, Condition cond) {
	drop(count, cond);
	ret(cond);
}

void BoyiaAssembler::ret(Condition cond) {
#if USE_BX
	bx(lr, cond);
#else
	mov(pc, BoyiaOperand(lr), LeaveCC, cond);
#endif
}
}
