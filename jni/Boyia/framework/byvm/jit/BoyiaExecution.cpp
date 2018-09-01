#include "BoyiaExecution.h"
#include "BoyiaAssembler.h"
#include "PlatformLib.h"
#include <android/log.h>
#include <string.h>
#include <sys/mman.h>

// void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
// int munmap(void *start, size_t length);

namespace boyia {
#define ACCESS_MASM(masm) masm.
#define __ ACCESS_MASM(masm)

template<typename F>
F FUNCTION_CAST(LByte* addr) {
	return reinterpret_cast<F>(reinterpret_cast<LInt>(addr));
}

// When running without a simulator we call the entry directly.
#define CALL_GENERATED_CODE(entry, p0, p1, p2, p3, p4) \
  (entry(p0, p1, p2, p3, p4))

//typedef void (*F0)();
typedef void* (*F2)(LInt x, LInt y, LInt p2, LInt p3, LInt p4);
//"ldr r2,%2 \t\n"
//"cmp r2, #0 \t\n"
//"ble end\t\n"
//"ldr r0,%0 \t\n"
//"ldr r1,%1 \t\n"
//"armmemcpy: ldrb r3, [r1], #1\t\n"
//"strb r3, [r0], #1 \t\n"
//"subs r2, r2, #1 \t\n"
//"bne armmemcpy \t\n"
//"end: \t\n"

NativeCode::NativeCode()
    : m_codeSegment(NULL)
    , m_codeSize(0) {
}

NativeCode::~NativeCode() {
	munmap(m_codeSegment, m_codeSize);
}

LVoid NativeCode::copyCode(LByte* buffer, LInt len) {
	// mmap可以在代码段申请一段内存
	if (m_codeSegment != NULL) {
		munmap(m_codeSegment, m_codeSize);
	}

	m_codeSegment = (LByte*) mmap(NULL, len, PROT_WRITE | PROT_EXEC,
		                   MAP_ANON | MAP_PRIVATE, -1, 0);
	m_codeSize = len;

	memmove(m_codeSegment, buffer, len);
}

LByte* NativeCode::codeStart() {
	return m_codeSegment;
}

void BoyiaExecution::callCode() {
	char testBuffer[100];
	LMemset(testBuffer, 0, 100);
	const char* testStr = "Hello World";

	int testInt[] = {71111,71112,71113,71114};
	//LInt addr = (LInt)callCode;

	// 测试实现一个memcpy
	BoyiaAssembler masm;
	Label armmemcpy;
	Label armend;
	// 长度11

	__ cmp(r2, BoyiaOperand(0));
	__ b(&armend, le);
	__ bind(&armmemcpy);
	__ ldrb(r3, MemOperand(r1, 1, PostIndex));
	__ strb(r3, MemOperand(r0, 1, PostIndex));
	__ sub(r2, r2, BoyiaOperand(1), SetCC);
	__ b(&armmemcpy, ne);
	__ bind(&armend);
//    __ ret();


	//__ mov(r1, BoyiaOperand(70000));
	//__ ldr(r1, MemOperand(r1, 8));
	//__ add(r0, r0, BoyiaOperand(r1));
	__ ret();
	CodeDesc desc;
	masm.getCode(&desc);
	//__ mov(pc, BoyiaOperand(r0), LeaveCC);

	// mmap可以在代码段申请一段内存
	//LByte* mem = (LByte*) mmap(NULL, 100, PROT_WRITE | PROT_EXEC,
	 //                  MAP_ANON | MAP_PRIVATE, -1, 0);

	//NativeCode code;
	//memmove(mem, desc.buffer, desc.instr_size);
	NativeCode code;
	code.copyCode(desc.buffer, desc.instr_size);
	F2 f = FUNCTION_CAST<F2>(code.codeStart());

	LInt res = reinterpret_cast<LInt>(CALL_GENERATED_CODE(f, (LInt)testBuffer, (LInt)testStr, 11, 0, 0));

	//__android_log_print(ANDROID_LOG_INFO, "BoyiaVM", "BoyiaVM call addr=%d", res);
	__android_log_print(ANDROID_LOG_INFO, "BoyiaVM", "BoyiaVM call str=%s", testBuffer);
}
}
