//! Full compiler for Boyia: lexer, ParseStatement, and all instruction emission.
//! CompileState matches BoyiaCore.cpp (mProg, mLineNum, mColumnNum, mToken, mVm, mCmds).

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(dead_code)]

use crate::core::get_runtime_from_vm;
use crate::types::*;
use std::ptr;
use std::mem;

const MAX_IDENT_LEN: usize = 256;

/// Syntax error codes (match BoyiaCore.cpp SntxError / SEMI_EXPECTED etc.).
#[derive(Clone, Copy)]
enum SntxError {
    SemiExpected,
    CreateMapError,
    MapKeyValueError,
    Syntax,
    LptrExpected,
    RptrExpected,
}

unsafe fn sntx_error_build(err: SntxError, cs: *const CompileState) {
    let line = if cs.is_null() { 0 } else { (*cs).mLineNum };
    let msg = match err {
        SntxError::SemiExpected => "semicolon expected",
        SntxError::CreateMapError => "create map error: identifier or string expected",
        SntxError::MapKeyValueError => "map key-value error: colon or assign expected",
        SntxError::Syntax => "syntax error",
        SntxError::LptrExpected => "'(' expected",
        SntxError::RptrExpected => "')' expected",
    };
    eprintln!("syntax error (line {}): {}", line, msg);
}

fn hash_ident(s: &[u8]) -> LUintPtr {
    let mut h: LUintPtr = 5381;
    for &b in s {
        h = h.wrapping_mul(33).wrapping_add(b as LUintPtr);
    }
    h
}

/// Match C++ AddColumn(CompileState* cs, LInt number): advance mProg and mLineNum by number.
#[inline]
unsafe fn add_column(cs: *mut CompileState, number: LInt) {
    (*cs).mProg = (*cs).mProg.offset(number as isize);
    (*cs).mLineNum += number;
}

#[inline]
unsafe fn cur_byte(cs: *mut CompileState) -> u8 {
    if (*cs).mProg.is_null() {
        return 0;
    }
    *(*cs).mProg as u8
}

/// Match C++ NewLine(CompileState* cs): ++mLineNum; mColumnNum = 0.
#[inline]
unsafe fn new_line(cs: *mut CompileState) {
    (*cs).mLineNum += 1;
    (*cs).mColumnNum = 0;
}

/// Match C++ NextToken leading loop: while (LIsSpace(*mProg) && *mProg) { if \n NewLine(cs); AddColumn(cs, 1); }
unsafe fn skip_spaces(cs: *mut CompileState) {
    loop {
        let c = cur_byte(cs);
        if c != b' ' && c != b'\t' && c != b'\r' && c != b'\n' {
            break;
        }
        if c == 0 {
            break;
        }
        if c == b'\n' {
            new_line(cs);
        } else {
            (*cs).mColumnNum += 1;
        }
        add_column(cs, 1);
    }
}

/// Match C++ SkipComment(CompileState* cs): filter /* */ and // comments.
unsafe fn skip_comment(cs: *mut CompileState) {
    if cur_byte(cs) != b'/' {
        return;
    }
    let next = (*cs).mProg.add(1).read() as u8;
    if next == b'*' {
        add_column(cs, 2);
        loop {
            if cur_byte(cs) == 0 {
                return;
            }
            if cur_byte(cs) == b'*' && (*cs).mProg.add(1).read() as u8 == b'/' {
                add_column(cs, 2);
                break;
            }
            if cur_byte(cs) == b'\n' {
                new_line(cs);
            }
            add_column(cs, 1);
        }
        skip_comment(cs);
    } else if next == b'/' {
        add_column(cs, 2);
        while cur_byte(cs) != 0 && cur_byte(cs) != b'\n' {
            add_column(cs, 1);
        }
        if cur_byte(cs) == b'\n' {
            new_line(cs);
            add_column(cs, 1);
        }
        skip_comment(cs);
    }
}

/// Allocate one instruction in vm->mVMCode; returns index or None if full.
unsafe fn allocate_instruction(vm: *mut BoyiaVM) -> Option<usize> {
    if vm.is_null() || (*vm).mVMCode.is_null() || (*vm).mVMCode.as_ref().unwrap().mCode.is_null() {
        return None;
    }
    let vmcode = &mut *(*vm).mVMCode;
    let size = vmcode.mSize as usize;
    if size >= CODE_CAPACITY {
        return None;
    }
    vmcode.mSize += 1;
    let inst = vmcode.mCode.add(size);
    (*inst).mOPCode = CmdType::kCmdJmpTrue; // 0, placeholder until overwritten by put_instruction
    (*inst).mOPLeft.mType = OpType::OP_NONE;
    (*inst).mOPLeft.mValue = 0;
    (*inst).mOPRight.mType = OpType::OP_NONE;
    (*inst).mOPRight.mValue = 0;
    (*inst).mNext = kInvalidInstruction;
    (*inst).mCache = ptr::null_mut();
    Some(size)
}

/// Get pointer to instruction at index.
unsafe fn get_instruction_mut(vm: *mut BoyiaVM, index: usize) -> *mut Instruction {
    if vm.is_null() || (*vm).mVMCode.is_null() {
        return ptr::null_mut();
    }
    let code = (*(*vm).mVMCode).mCode;
    if code.is_null() {
        return ptr::null_mut();
    }
    code.add(index)
}

/// Instruction pointer to index in mVMCode->mCode (matches C++ pointer difference).
unsafe fn inst_ptr_to_index(vm: *mut BoyiaVM, inst: *mut Instruction) -> usize {
    if vm.is_null() || (*vm).mVMCode.is_null() {
        return 0;
    }
    let code = (*(*vm).mVMCode).mCode;
    if code.is_null() || inst.is_null() {
        return 0;
    }
    (inst as usize - code as usize) / mem::size_of::<Instruction>()
}

/// SetCodePosition(codeIndex, row, column, vm) in BoyiaValue.cpp: records debug position. No-op when no debugger.
#[inline]
unsafe fn set_code_position(_code_index: usize, _row: LInt, _column: LInt, _vm: *mut BoyiaVM) {
    // C++: GetRuntime(vm)->debugger()->setCodePosition(...). Rust has no debugger; stub for API match.
}

/// PutInstruction(left, right, op, cs) per BoyiaCore.cpp: allocate, init operands from OpCommand, set op, link chain, SetCodePosition.
/// Use OpCommand::none() for no operand; OpCommand::const_number(v), OpCommand::reg0(), etc. for operands. Returns new instruction index.
unsafe fn put_instruction(
    cs: *mut CompileState,
    left: OpCommand,
    right: OpCommand,
    op: CmdType,
) -> Option<usize> {
    let vm = (*cs).mVm;
    let idx = allocate_instruction(vm)?;
    let new_ins = get_instruction_mut(vm, idx);

    // Init member (match C++ newIns->mOPLeft = *left; newIns->mOPRight = *right when not none)
    (*new_ins).mOPLeft.mType = OpType::OP_NONE;
    (*new_ins).mOPLeft.mValue = 0;
    (*new_ins).mOPRight.mType = OpType::OP_NONE;
    (*new_ins).mOPRight.mValue = 0;

    if left.mType != OpType::OP_NONE {
        (*new_ins).mOPLeft = left;
    }
    if right.mType != OpType::OP_NONE {
        (*new_ins).mOPRight = right;
    }

    (*new_ins).mOPCode = op;
    (*new_ins).mNext = kInvalidInstruction;
    (*new_ins).mCache = ptr::null_mut();

    // CommandTable* cmds = cs->mCmds; Instruction* inst = cmds->mEnd;
    let cmds = &mut (*cs).mCmds;
    let inst = cmds.mEnd;
    if inst.is_null() {
        cmds.mBegin = new_ins;
    } else {
        // inst->mNext = cs->mVm->mVMCode->mSize - 1 (index of new instruction)
        (*inst).mNext = idx as LIntPtr;
    }
    cmds.mEnd = new_ins;

    set_code_position(idx, (*cs).mLineNum, (*cs).mColumnNum, vm);
    Some(idx)
}

/// Patch instruction at index: set left or right to (OP_CONST_NUMBER, offset).
unsafe fn patch_offset(vm: *mut BoyiaVM, index: usize, is_right: bool, offset: LIntPtr) {
    let inst = get_instruction_mut(vm, index);
    if inst.is_null() {
        return;
    }
    if is_right {
        (*inst).mOPRight.mType = OpType::OP_CONST_NUMBER;
        (*inst).mOPRight.mValue = offset;
    } else {
        (*inst).mOPLeft.mType = OpType::OP_CONST_NUMBER;
        (*inst).mOPLeft.mValue = offset;
    }
}

/// Create executor: clear mCmds (matches C++ CreateExecutor).
unsafe fn create_executor(cs: *mut CompileState) {
    (*cs).mCmds.mBegin = ptr::null_mut();
    (*cs).mCmds.mEnd = ptr::null_mut();
}

/// Append current chain to mEntry (matches C++ AppendEntry).
unsafe fn append_entry(cs: *mut CompileState) {
    let vm = (*cs).mVm;
    if vm.is_null() || (*vm).mEntry.is_null() {
        return;
    }
    let begin = (*cs).mCmds.mBegin;
    if begin.is_null() {
        return;
    }
    let idx = inst_ptr_to_index(vm, begin);
    let entry = &mut *(*vm).mEntry;
    let size = entry.mSize as usize;
    if size >= ENTRY_CAPACITY {
        return;
    }
    entry.mTable[size] = idx as LInt;
    entry.mSize += 1;
}

/// Slice of current token name (mToken.mTokenName). Valid until next NextToken/Putback.
unsafe fn token_name_slice(cs: *mut CompileState) -> &'static [u8] {
    let name = &(*cs).mToken.mTokenName;
    if name.mPtr.is_null() || name.mLen <= 0 {
        return &[];
    }
    let len = name.mLen as usize;
    std::slice::from_raw_parts(name.mPtr as *const u8, len)
}

/// Parse number from current token (NUMBER). Matches C++ STR2_INT(mToken.mTokenName).
unsafe fn parse_number_from_token(cs: *mut CompileState) -> LIntPtr {
    let s = std::str::from_utf8(token_name_slice(cs)).unwrap_or("0");
    s.parse::<LInt>().unwrap_or(0) as LIntPtr
}

/// Resolve identifier from current token name via [Runtime::gen_identifier].
unsafe fn gen_identifier(cs: *mut CompileState) -> LUintPtr {
    let vm = (*cs).mVm;
    if vm.is_null() {
        return hash_ident(token_name_slice(cs));
    }
    let rt = get_runtime_from_vm(vm as *mut LVoid);
    if rt.is_null() {
        return hash_ident(token_name_slice(cs));
    }
    let slice = token_name_slice(cs);
    let s = std::str::from_utf8(slice).unwrap_or("");
    (*rt).gen_identifier(s)
}

/// GenIdentifier for object literal key: STRING_VALUE uses content length (mLen - 2), else full token. Match EvalObject keyStr in BoyiaCore.cpp.
unsafe fn gen_identifier_object_key(cs: *mut CompileState) -> LUintPtr {
    let name = &(*cs).mToken.mTokenName;
    let slice = if (*cs).mToken.mTokenType == TokenType::STRING_VALUE {
        let len = (name.mLen - 2).max(0) as usize;
        if name.mPtr.is_null() || len == 0 {
            &[]
        } else {
            std::slice::from_raw_parts(name.mPtr as *const u8, len)
        }
    } else {
        token_name_slice(cs)
    };
    let vm = (*cs).mVm;
    if vm.is_null() {
        return hash_ident(slice);
    }
    let rt = get_runtime_from_vm(vm as *mut LVoid);
    if rt.is_null() {
        return hash_ident(slice);
    }
    let s = std::str::from_utf8(slice).unwrap_or("");
    (*rt).gen_identifier(s)
}

/// Putback: rewind by current token length (matches C++ Putback -> AddColumn(cs, -cs->mToken.mTokenName.mLen)).
unsafe fn putback(cs: *mut CompileState) {
    add_column(cs, -(*cs).mToken.mTokenName.mLen);
}

/// GetIdentifer: match C++ LIsAlpha first char, then _/alpha/digit. LookUp for keyword. Set mTokenName inside.
unsafe fn get_identifier(cs: *mut CompileState) -> bool {
    let c = cur_byte(cs);
    if !c.is_ascii_alphabetic() && c != b'_' {
        return false;
    }
    (*cs).mToken.mTokenName.mPtr = (*cs).mProg;
    let mut len = 0usize;
    while len < MAX_IDENT_LEN - 1 {
        let ch = cur_byte(cs);
        if ch == b'_' || ch.is_ascii_alphanumeric() {
            len += 1;
            add_column(cs, 1);
        } else {
            break;
        }
    }
    (*cs).mToken.mTokenName.mLen = len as LInt;
    let start = (*cs).mProg.offset(-(len as isize));
    let slice = std::slice::from_raw_parts(start as *const u8, len);
    (*cs).mToken.mTokenType = TokenType::IDENTIFIER;
    (*cs).mToken.mTokenValue = TokenValue::NONE;
    if slice == b"var" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_VAR;
    } else if slice == b"fun" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_FUNC;
    } else if slice == b"class" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_CLASS;
    } else if slice == b"if" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_IF;
    } else if slice == b"elif" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_ELIF;
    } else if slice == b"else" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_ELSE;
    } else if slice == b"do" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_DO;
    } else if slice == b"while" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_WHILE;
    } else if slice == b"for" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_FOR;
    } else if slice == b"break" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_BREAK;
    } else if slice == b"extends" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_EXTEND;
    } else if slice == b"prop" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_PROP;
    } else if slice == b"return" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_RETURN;
    } else if slice == b"async" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_ASYNC;
    } else if slice == b"await" {
        (*cs).mToken.mTokenType = TokenType::KEYWORD;
        (*cs).mToken.mTokenValue = TokenValue::BY_AWAIT;
    }
    true
}

/// GetLogicValue: match C++ MStrchr("&|!<>="). Set mTokenName inside. Return true if matched.
unsafe fn get_logic_value(cs: *mut CompileState) -> bool {
    let c = cur_byte(cs);
    (*cs).mToken.mTokenName.mPtr = (*cs).mProg;
    let (len, val) = match c {
        b'=' => {
            if (*cs).mProg.add(1).read() as u8 == b'=' {
                add_column(cs, 2);
                (2, TokenValue::EQ)
            } else {
                (0, TokenValue::NONE)
            }
        }
        b'!' => {
            if (*cs).mProg.add(1).read() as u8 == b'=' {
                add_column(cs, 2);
                (2, TokenValue::NE)
            } else {
                add_column(cs, 1);
                (1, TokenValue::NOT)
            }
        }
        b'<' => {
            if (*cs).mProg.add(1).read() as u8 == b'=' {
                add_column(cs, 2);
                (2, TokenValue::LE)
            } else {
                add_column(cs, 1);
                (1, TokenValue::LT)
            }
        }
        b'>' => {
            if (*cs).mProg.add(1).read() as u8 == b'=' {
                add_column(cs, 2);
                (2, TokenValue::GE)
            } else {
                add_column(cs, 1);
                (1, TokenValue::GT)
            }
        }
        b'&' => {
            if (*cs).mProg.add(1).read() as u8 == b'&' {
                add_column(cs, 2);
                (2, TokenValue::AND)
            } else {
                (0, TokenValue::NONE)
            }
        }
        b'|' => {
            if (*cs).mProg.add(1).read() as u8 == b'|' {
                add_column(cs, 2);
                (2, TokenValue::OR)
            } else {
                (0, TokenValue::NONE)
            }
        }
        _ => (0, TokenValue::NONE),
    };
    if len != 0 {
        (*cs).mToken.mTokenName.mLen = len as LInt;
        (*cs).mToken.mTokenType = TokenType::DELIMITER;
        (*cs).mToken.mTokenValue = val;
        return true;
    }
    false
}

/// GetStringValue: match C++ only '"', stop at '"' or '\r'. mTokenName.mPtr = first char inside (after quote), mLen = len+2.
unsafe fn get_string_value(cs: *mut CompileState) -> bool {
    if cur_byte(cs) != b'"' {
        return false;
    }
    add_column(cs, 1);
    (*cs).mToken.mTokenName.mPtr = (*cs).mProg;
    let mut len = 0i32;
    while cur_byte(cs) != b'"' && cur_byte(cs) != b'\r' && cur_byte(cs) != 0 {
        len += 1;
        add_column(cs, 1);
    }
    if cur_byte(cs) == b'\r' {
        add_column(cs, 1);
    } else {
        add_column(cs, 1);
    }
    (*cs).mToken.mTokenType = TokenType::STRING_VALUE;
    (*cs).mToken.mTokenValue = TokenValue::QUOTE;
    (*cs).mToken.mTokenName.mLen = len + 2;
    true
}

/// GetDelimiter: match C++ single char from "+-*/%^=;,'.:()[]{}". Set mTokenName.mPtr/mLen = 1 inside.
unsafe fn get_delimiter(cs: *mut CompileState) -> bool {
    let c = cur_byte(cs);
    let val = match c {
        b'+' => TokenValue::ADD,
        b'-' => TokenValue::SUB,
        b'*' => TokenValue::MUL,
        b'/' => TokenValue::DIV,
        b'%' => TokenValue::MOD,
        b'^' => TokenValue::POW,
        b'=' => TokenValue::ASSIGN,
        b';' => TokenValue::SEMI,
        b',' => TokenValue::COMMA,
        b'\'' => TokenValue::QUOTE,
        b'.' => TokenValue::DOT,
        b':' => TokenValue::COLON,
        b'(' => TokenValue::LPTR,
        b')' => TokenValue::RPTR,
        b'[' => TokenValue::ARRAY_BEGIN,
        b']' => TokenValue::ARRAY_END,
        b'{' => TokenValue::BLOCK_START,
        b'}' => TokenValue::BLOCK_END,
        _ => return false,
    };
    (*cs).mToken.mTokenName.mPtr = (*cs).mProg;
    (*cs).mToken.mTokenName.mLen = 1;
    add_column(cs, 1);
    (*cs).mToken.mTokenType = TokenType::DELIMITER;
    (*cs).mToken.mTokenValue = val;
    true
}

/// GetNumberValue: match C++ LIsDigit only (no leading minus). Set mTokenName inside.
unsafe fn get_number_value(cs: *mut CompileState) -> bool {
    if !cur_byte(cs).is_ascii_digit() {
        return false;
    }
    (*cs).mToken.mTokenName.mPtr = (*cs).mProg;
    let mut len = 0usize;
    while cur_byte(cs).is_ascii_digit() {
        len += 1;
        add_column(cs, 1);
    }
    (*cs).mToken.mTokenName.mLen = len as LInt;
    (*cs).mToken.mTokenType = TokenType::NUMBER;
    (*cs).mToken.mTokenValue = TokenValue::NONE;
    true
}

/// NextToken: match C++ NextToken exactly.
/// Clear token; skip spaces (NewLine on \n); if \0 set DELIMITER+BY_END and mTokenName; SkipComment; Get* set mTokenName inside.
unsafe fn next_token(cs: *mut CompileState) {
    (*cs).mToken.mTokenType = TokenType::NONE;
    (*cs).mToken.mTokenValue = TokenValue::NONE;
    (*cs).mToken.mTokenName.mPtr = ptr::null_mut();
    (*cs).mToken.mTokenName.mLen = 0;

    skip_spaces(cs);
    if cur_byte(cs) == 0 {
        (*cs).mToken.mTokenType = TokenType::DELIMITER;
        (*cs).mToken.mTokenValue = TokenValue::BY_END;
        (*cs).mToken.mTokenName.mPtr = (*cs).mProg;
        (*cs).mToken.mTokenName.mLen = 0;
        return;
    }

    skip_comment(cs);

    if get_identifier(cs) {
        return;
    }
    if get_logic_value(cs) {
        return;
    }
    if get_string_value(cs) {
        return;
    }
    if get_delimiter(cs) {
        return;
    }
    if get_number_value(cs) {
        return;
    }
}

/// Add string from current token (STRING_VALUE) to VM string table; return index. C++ CopyStringFromToken: mPtr points to content (after opening quote), mLen = content_len+2.
unsafe fn add_string_from_token(cs: *mut CompileState) -> Option<usize> {
    let vm = (*cs).mVm;
    if vm.is_null() || (*vm).mStrTable.is_null() {
        return None;
    }
    let st = &mut *(*vm).mStrTable;
    let idx = st.mSize as usize;
    if idx >= CONST_CAPACITY {
        return None;
    }
    let name = &(*cs).mToken.mTokenName;
    let inner_len = (name.mLen - 2).max(0) as usize;
    let src = name.mPtr;
    let layout = std::alloc::Layout::array::<LInt8>(inner_len + 1).ok()?;
    let ptr = std::alloc::alloc(layout) as *mut LInt8;
    if ptr.is_null() {
        return None;
    }
    for i in 0..inner_len {
        *ptr.add(i) = *src.add(i);
    }
    *ptr.add(inner_len) = 0;
    st.mTable[idx].mPtr = ptr;
    st.mTable[idx].mLen = inner_len as LInt;
    st.mSize += 1;
    Some(idx)
}

// ---------------------------------------------------------------------------
// Declare global (for GlobalStatement) and create_fun_val (for Fun/Class)
// ---------------------------------------------------------------------------
unsafe fn declare_global_var(vm: *mut BoyiaVM, name_key: LUintPtr) {
    if vm.is_null() || (*vm).mGlobals.is_null() {
        return;
    }
    if (*vm).mGValSize as usize >= NUM_GLOBAL_VARS {
        return;
    }
    let val = (*vm).mGlobals.add((*vm).mGValSize as usize);
    (*vm).mGValSize += 1;
    (*val).mNameKey = name_key;
    (*val).mValueType = ValueType::BY_ARG;
    (*val).mValue.mIntVal = 0;
}

unsafe fn init_params_with_anonym(cs: *mut CompileState, is_anonym: bool) {
    (*cs).mFunctionScopes.push(FunctionScope::new_with_anonym(is_anonym));
    loop {
        next_token(cs);
        if (*cs).mToken.mTokenValue == TokenValue::RPTR {
            break;
        }
        if (*cs).mToken.mTokenType != TokenType::IDENTIFIER {
            break;
        }
        let name_key = gen_identifier(cs);
        if let Some(fs) = (*cs).mFunctionScopes.last_mut() {
            fs.add_param(name_key);
        }
        let key = name_key as LIntPtr;
        let _ = put_instruction(cs, OpCommand::const_number(key), OpCommand::none(), CmdType::kCmdParamCreate);
        next_token(cs);
        if (*cs).mToken.mTokenValue != TokenValue::COMMA {
            break;
        }
    }
    // Local scopes are pushed on `{` in [block_statement] and popped on `}`.
}

unsafe fn init_params(cs: *mut CompileState) {
    init_params_with_anonym(cs, false);
}

/// Emit OP_LOCAL when `key` is in the innermost [FunctionScope], else OP_VAR (globals / outer / unknown).
unsafe fn compile_var_operand(cs: *mut CompileState, key: LUintPtr) -> OpCommand {
    if let Some(fs) = (*cs).mFunctionScopes.last() {
        if let Some(off) = fs.resolve_local_frame_offset(key) {
            return OpCommand::op_local(off);
        }
        if fs.mIsAnonym {
            if let Some(i) = fs.mCaptures.iter().position(|&k| k == key) {
                println!("get capture idx: {}", i);
                return OpCommand::op_capture(i as LIntPtr);
            }
        }
    }
    OpCommand::op_var(key as LIntPtr)
}

// Expression chain (must be before statements that call eval_expression)
// Resolve native by key via runtime dispatcher (see core::find_native_func).
fn find_native_func(key: LUintPtr, vm: *mut BoyiaVM) -> LInt {
    unsafe { crate::core::find_native_func(vm as *mut crate::types::LVoid, key) }
}

/// PushArgStatement(needPushFunction, cs) per BoyiaCore.cpp: if needPushFunction { ++argCount; PutInstruction PushArg; NextToken; if RPTR Assign(argCount) return; Putback; } do { EvalExpression; PutInstruction PushArg; ++argCount; } while (COMMA); PutInstruction Assign(argCount).
unsafe fn push_arg_statement(cs: *mut CompileState, need_push_function: bool) {
    let mut arg_count: LIntPtr = 0;
    if need_push_function {
        arg_count += 1;
        let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPushArg);
        next_token(cs); // if token == ')' exit
        if (*cs).mToken.mTokenValue == TokenValue::RPTR {
            let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::const_number(arg_count), CmdType::kCmdAssign);
            return;
        }
        putback(cs);
    }
    // do { EvalExpression; PutInstruction PushArg; ++argCount; } while (COMMA). EvalExpression does NextToken at start so COMMA is consumed when looping.
    loop {
        eval_expression(cs);
        let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPushArg);
        arg_count += 1;
        if (*cs).mToken.mTokenValue != TokenValue::COMMA {
            break;
        }
    }
    let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::const_number(arg_count), CmdType::kCmdAssign);
}

unsafe fn call_statement(cs: *mut CompileState, obj_type: OpType, obj_value: LIntPtr) {
    push_arg_statement(cs, true);
    if obj_type == OpType::OP_VAR {
        let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPop);
    }
    let _ = put_instruction(cs, OpCommand { mType: obj_type, mValue: obj_value }, OpCommand::none(), CmdType::kCmdPushObj);
    let push_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdPushScene).unwrap_or(0);
    let _ = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdPushParams);
    let fun_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdCallFunction).unwrap_or(0);
    patch_offset((*cs).mVm, push_idx, false, (fun_idx as LIntPtr).wrapping_sub(push_idx as LIntPtr));
}

unsafe fn call_native_statement(cs: *mut CompileState, idx: LInt) {
    next_token(cs);
    push_arg_statement(cs, false);
    let _ = put_instruction(cs, OpCommand::const_number(0), OpCommand::none(), CmdType::kCmdPushObj);
    let push_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdPushScene).unwrap_or(0);
    let _ = put_instruction(cs, OpCommand::const_number(idx as LIntPtr), OpCommand::none(), CmdType::kCmdCallNative);
    let pop_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdPopScene).unwrap_or(0);
    patch_offset((*cs).mVm, push_idx, false, (pop_idx as LIntPtr).wrapping_sub(push_idx as LIntPtr));
}

unsafe fn eval_get_value(cs: *mut CompileState, obj_key: LUintPtr) {
    let src = compile_var_operand(cs, obj_key);
    let _ = put_instruction(cs, OpCommand::reg0(), src, CmdType::kCmdAssign);
    if (*cs).mToken.mTokenValue == TokenValue::DOT {
        eval_get_prop(cs);
    }
}

/// EvalGetProp(cs) per BoyiaCore.cpp: NextToken; if not IDENTIFIER return; Push; GenIdentifier; GetProp; NextToken (last must next); LPTR->CallStatement+NextToken+DOT?EvalGetProp; DOT->Pop+EvalGetProp; else Pop.
unsafe fn eval_get_prop(cs: *mut CompileState) {
    next_token(cs);
    if (*cs).mToken.mTokenType != TokenType::IDENTIFIER {
        return;
    }
    // Push class context for callstatement (same order as C++)
    let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPush);
    let prop_key = gen_identifier(cs);
    let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::const_number(prop_key as LIntPtr), CmdType::kCmdGetProp);
    // Last must next
    next_token(cs);
    if (*cs).mToken.mTokenValue == TokenValue::LPTR {
        call_statement(cs, OpType::OP_VAR, 0);
        next_token(cs);
        if (*cs).mToken.mTokenValue == TokenValue::DOT {
            eval_get_prop(cs);
        }
    } else if (*cs).mToken.mTokenValue == TokenValue::DOT {
        let _ = put_instruction(cs, OpCommand::const_number(0), OpCommand::none(), CmdType::kCmdPop);
        eval_get_prop(cs);
    } else {
        let _ = put_instruction(cs, OpCommand::const_number(0), OpCommand::none(), CmdType::kCmdPop);
    }
}

unsafe fn atom(cs: *mut CompileState) {
    match (*cs).mToken.mTokenType {
        TokenType::IDENTIFIER => {
            let key = gen_identifier(cs);
            let idx = find_native_func(key, (*cs).mVm);
            if idx >= 0 {
                call_native_statement(cs, idx);
                next_token(cs);
            } else {
                next_token(cs);
                if (*cs).mToken.mTokenValue == TokenValue::LPTR {
                    let src = compile_var_operand(cs, key);
                    let _ = put_instruction(cs, OpCommand::reg0(), src, CmdType::kCmdAssign);
                    call_statement(cs, OpType::OP_NONE, 0);
                    next_token(cs);
                    if (*cs).mToken.mTokenValue == TokenValue::DOT {
                        eval_get_prop(cs);
                    }
                } else {
                    eval_get_value(cs, key);
                }
            }
        }
        TokenType::NUMBER => {
            let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::const_number(parse_number_from_token(cs)), CmdType::kCmdAssign);
            next_token(cs);
        }
        TokenType::STRING_VALUE => {
            if let Some(str_idx) = add_string_from_token(cs) {
                let _ = put_instruction(cs, OpCommand::const_number(str_idx as LIntPtr), OpCommand::none(), CmdType::kCmdConstStr);
            }
            next_token(cs);
        }
        _ => {
            if (*cs).mToken.mTokenValue == TokenValue::RPTR {
                return;
            }
        }
    }
}

unsafe fn eval_subexpr(cs: *mut CompileState) {
    if (*cs).mToken.mTokenValue == TokenValue::LPTR {
        eval_expression(cs);
        next_token(cs);
    } else {
        atom(cs);
    }
}

unsafe fn eval_minus(cs: *mut CompileState) {
    let op = (*cs).mToken.mTokenValue;
    if op == TokenValue::ADD || op == TokenValue::SUB {
        next_token(cs);
    }
    eval_subexpr(cs);
    if op == TokenValue::SUB {
        let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::const_number(-1), CmdType::kCmdAssign);
        let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::reg0(), CmdType::kCmdMul);
    }
}

unsafe fn eval_arith(cs: *mut CompileState) {
    eval_minus(cs);
    loop {
        let op = (*cs).mToken.mTokenValue;
        if op != TokenValue::MUL && op != TokenValue::DIV && op != TokenValue::MOD {
            break;
        }
        let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPush);
        next_token(cs);
        eval_minus(cs);
        let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::none(), CmdType::kCmdPop);
        let cmd = match op {
            TokenValue::MUL => CmdType::kCmdMul,
            TokenValue::DIV => CmdType::kCmdDiv,
            _ => CmdType::kCmdMod,
        };
        let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::reg0(), cmd);
    }
}

unsafe fn eval_add_sub(cs: *mut CompileState) {
    eval_arith(cs);
    loop {
        let op = (*cs).mToken.mTokenValue;
        if op != TokenValue::ADD && op != TokenValue::SUB {
            break;
        }
        let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPush);
        next_token(cs);
        eval_arith(cs);
        let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::none(), CmdType::kCmdPop);
        let cmd = if op == TokenValue::ADD { CmdType::kCmdAdd } else { CmdType::kCmdSub };
        let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::reg0(), cmd);
    }
}

unsafe fn eval_relational_impl(op: TokenValue, cs: *mut CompileState) {
    let cmd = match op {
        TokenValue::NOT => CmdType::kCmdNotRelation,
        TokenValue::LT => CmdType::kCmdLtRelation,
        TokenValue::LE => CmdType::kCmdLeRelation,
        TokenValue::GT => CmdType::kCmdGtRelation,
        TokenValue::GE => CmdType::kCmdGeRelation,
        TokenValue::EQ => CmdType::kCmdEqRelation,
        TokenValue::NE => CmdType::kCmdNeRelation,
        _ => return,
    };
    let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::reg0(), cmd);
}

unsafe fn eval_relational(cs: *mut CompileState) {
    if (*cs).mToken.mTokenValue != TokenValue::NOT {
        eval_add_sub(cs);
    }
    let op = (*cs).mToken.mTokenValue;
    const RELOPS: [TokenValue; 7] = [
        TokenValue::NOT,
        TokenValue::LT,
        TokenValue::LE,
        TokenValue::GT,
        TokenValue::GE,
        TokenValue::EQ,
        TokenValue::NE,
    ];
    if !RELOPS.contains(&op) {
        return;
    }
    if op != TokenValue::NOT {
        let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPush);
    }
    next_token(cs);
    eval_add_sub(cs);
    if op != TokenValue::NOT {
        let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::none(), CmdType::kCmdPop);
    }
    eval_relational_impl(op, cs);
}

unsafe fn eval_logic(cs: *mut CompileState) {
    eval_relational(cs);
    loop {
        let op = (*cs).mToken.mTokenValue;
        if op != TokenValue::AND && op != TokenValue::OR {
            break;
        }
        let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPush);
        next_token(cs);
        eval_relational(cs);
        let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::none(), CmdType::kCmdPop);
        let cmd = if op == TokenValue::AND { CmdType::kCmdAndLogic } else { CmdType::kCmdOrLogic };
        let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::reg0(), cmd);
    }
}

/// EvalObject per BoyiaCore.cpp: if not BLOCK_START then EvalLogic; else CreateMap, do { NextToken; if } return; if not id/string SntxError(CREATE_MAP_ERROR); SetMapKey; NextToken; if not :/= SntxError(MAP_KEY_VALUE_ERROR); Push; EvalExpression; Pop; SetMapValue; } while (COMMA); NextToken.
unsafe fn eval_object(cs: *mut CompileState) {
    if (*cs).mToken.mTokenValue != TokenValue::BLOCK_START {
        eval_logic(cs);
        return;
    }
    let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdCreateMap);
    loop {
        next_token(cs);
        if (*cs).mToken.mTokenValue == TokenValue::BLOCK_END {
            return;
        }
        if (*cs).mToken.mTokenType != TokenType::IDENTIFIER && (*cs).mToken.mTokenType != TokenType::STRING_VALUE {
            sntx_error_build(SntxError::CreateMapError, cs);
            return;
        }
        let key = gen_identifier_object_key(cs);
        let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::const_number(key as LIntPtr), CmdType::kCmdSetMapKey);
        next_token(cs);
        if (*cs).mToken.mTokenValue != TokenValue::COLON && (*cs).mToken.mTokenValue != TokenValue::ASSIGN {
            sntx_error_build(SntxError::MapKeyValueError, cs);
            return;
        }
        let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPush);
        eval_expression(cs);
        let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::none(), CmdType::kCmdPop);
        let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::reg1(), CmdType::kCmdSetMapValue);
        if (*cs).mToken.mTokenValue != TokenValue::COMMA {
            break;
        }
    }
    next_token(cs);
}

unsafe fn eval_array(cs: *mut CompileState) {
    if (*cs).mToken.mTokenValue != TokenValue::ARRAY_BEGIN {
        eval_object(cs);
        return;
    }
    let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdCreateArray);
    next_token(cs);
    if (*cs).mToken.mTokenValue == TokenValue::ARRAY_END {
        return;
    }
    putback(cs);
    loop {
        let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPush);
        eval_expression(cs);
        let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::none(), CmdType::kCmdPop);
        let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::reg1(), CmdType::kCmdAddArrayItem);
        if (*cs).mToken.mTokenValue != TokenValue::COMMA {
            break;
        }
    }
}

unsafe fn eval_assignment(cs: *mut CompileState) {
    eval_array(cs);
    if (*cs).mToken.mTokenValue != TokenValue::ASSIGN {
        return;
    }
    let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPush);
    next_token(cs);
    eval_array(cs);
    let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::none(), CmdType::kCmdPop);
    let _ = put_instruction(cs, OpCommand::reg1(), OpCommand::reg0(), CmdType::kCmdAssignVar);
}

/// Collect capture candidates for current anonymous function scope from the immediate parent scope:
/// parent function params + all parent local scopes' locals.
unsafe fn collect_parent_scope_captures_for_anonym(cs: *mut CompileState) {
    let scope_len = (*cs).mFunctionScopes.len();
    println!("call collect_parent_scope_captures_for_anonym scope_len={}", scope_len);
    if scope_len < 2 {
        return;
    }

    println!("call collect_parent_scope_captures_for_anonym");
    let parent_idx = scope_len - 2;
    let parent_params = (*cs).mFunctionScopes[parent_idx].mParams.clone();
    for key in parent_params {
        if let Some(current) = (*cs).mFunctionScopes.last_mut() {
            if !current.mCaptures.iter().any(|&k| k == key) {
                current.add_capture(key);
            }
        }
    }
    let parent_scopes_len = (*cs).mFunctionScopes[parent_idx].mLocalScopes.len();
    for si in 0..parent_scopes_len {
        let locals = (*cs).mFunctionScopes[parent_idx].mLocalScopes[si]
            .mLocals
            .clone();
        for key in locals {
            if let Some(current) = (*cs).mFunctionScopes.last_mut() {
                if !current.mCaptures.iter().any(|&k| k == key) {
                    current.add_capture(key);
                }
            }
        }
    }
}

unsafe fn anonym_fun_statement(cs: *mut CompileState) {
    next_token(cs);
    if (*cs).mToken.mTokenValue != TokenValue::LPTR {
        return;
    }
    let _ = put_instruction(cs, OpCommand::none(), OpCommand::const_number(TokenValue::BY_ANONYM_FUNC as LIntPtr), CmdType::kCmdCreateFunction);
    let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPush);
    let logic_idx = put_instruction(cs, OpCommand::const_number(LTrue as LIntPtr), OpCommand::none(), CmdType::kCmdOnceJmpTrue).unwrap_or(0);
    init_params_with_anonym(cs, true);
    collect_parent_scope_captures_for_anonym(cs);
    body_statement(cs, true);
    let end_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdIfEnd).unwrap_or(0);
    patch_offset((*cs).mVm, logic_idx, true, (end_idx as LIntPtr).wrapping_sub(logic_idx as LIntPtr));
    let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdPop);
    let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdSetAnonym);
}

unsafe fn eval_anonym_func(cs: *mut CompileState) -> bool {
    if (*cs).mToken.mTokenType == TokenType::KEYWORD && (*cs).mToken.mTokenValue == TokenValue::BY_FUNC {
        anonym_fun_statement(cs);
        next_token(cs);
        return true;
    }
    false
}

unsafe fn eval_await(cs: *mut CompileState) {
    next_token(cs);
    atom(cs);
    let _ = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdAwait);
}

unsafe fn eval_expression(cs: *mut CompileState) {
    next_token(cs);
    if (*cs).mToken.mTokenValue == TokenValue::BY_AWAIT {
        eval_await(cs);
        return;
    }
    if eval_anonym_func(cs) {
        return;
    }
    if (*cs).mToken.mTokenValue == TokenValue::SEMI {
        return;
    }
    eval_assignment(cs);
}

/// GlobalStatement(cs) per BoyiaCore.cpp: type = mToken; do { NextToken; PutInstruction(DeclGlobal); Putback; EvalExpression; } while (COMMA); if != SEMI SntxError.
unsafe fn global_statement(cs: *mut CompileState) {
    let type_val = (*cs).mToken.mTokenValue;
    loop {
        next_token(cs); /* get ident */
        let name_key = gen_identifier(cs);
        let _ = put_instruction(
            cs,
            OpCommand::const_number(type_val as LIntPtr),
            OpCommand::const_number(name_key as LIntPtr),
            CmdType::kCmdDeclGlobal,
        );
        putback(cs);
        eval_expression(cs);
        if (*cs).mToken.mTokenValue != TokenValue::COMMA {
            break;
        }
    }
    if (*cs).mToken.mTokenValue != TokenValue::SEMI {
        sntx_error_build(SntxError::SemiExpected, cs);
    }
}

/// LocalStatement: strictly matches BoyiaCore.cpp. type = cs->mToken.mTokenValue; do { NextToken; PutInstruction(DeclLocal, type, GenIdentifier); Putback; EvalExpression; } while (mToken.mTokenValue == COMMA); if (mToken.mTokenValue != SEMI) SntxErrorBuild(SEMI_EXPECTED).
unsafe fn local_statement(cs: *mut CompileState) {
    let type_val = (*cs).mToken.mTokenValue;
    loop {
        next_token(cs);
        if (*cs).mToken.mTokenType != TokenType::IDENTIFIER {
            break;
        }
        let name_key = gen_identifier(cs);
        if let Some(fs) = (*cs).mFunctionScopes.last_mut() {
            fs.add_local(name_key);
        }
        let _ = put_instruction(
            cs,
            OpCommand::const_number(type_val as LIntPtr),
            OpCommand::const_number(name_key as LIntPtr),
            CmdType::kCmdDeclLocal,
        );
        putback(cs);
        eval_expression(cs);
        if (*cs).mToken.mTokenValue != TokenValue::COMMA {
            break;
        }
    }
    if (*cs).mToken.mTokenValue != TokenValue::SEMI {
        sntx_error_build(SntxError::SemiExpected, cs);
    }
}

unsafe fn return_statement(cs: *mut CompileState) {
    next_token(cs);
    if (*cs).mToken.mTokenValue != TokenValue::SEMI && (*cs).mToken.mTokenType != TokenType::KEYWORD && (*cs).mToken.mTokenValue != TokenValue::BY_END {
        putback(cs);
        eval_expression(cs);
    }
    let _ = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdReturn);
}

/// BreakStatement per BoyiaCore.cpp: PutInstruction(kBoyiaNull, kBoyiaNull, kCmdBreak, cs); no NextToken.
unsafe fn break_statement(cs: *mut CompileState) {
    let _ = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdBreak);
}

unsafe fn if_statement(cs: *mut CompileState) {
    next_token(cs);
    eval_expression(cs);
    let logic_idx = put_instruction(cs, OpCommand::reg0(), OpCommand::none(), CmdType::kCmdJmpTrue).unwrap_or(0);
    block_statement(cs);
    let end_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdIfEnd).unwrap_or(0);
    patch_offset((*cs).mVm, logic_idx, true, (end_idx as LIntPtr).wrapping_sub(logic_idx as LIntPtr));
}

unsafe fn else_statement(cs: *mut CompileState) {
    let logic_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdElse).unwrap_or(0);
    block_statement(cs);
    let end_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdElEnd).unwrap_or(0);
    patch_offset((*cs).mVm, logic_idx, true, (end_idx as LIntPtr).wrapping_sub(logic_idx as LIntPtr));
}

unsafe fn while_statement(cs: *mut CompileState) {
    let begin_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdLoop).unwrap_or(0);
    next_token(cs);
    eval_expression(cs);
    let logic_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdLoopTrue).unwrap_or(0);
    block_statement(cs);
    let end_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdJmpTo).unwrap_or(0);
    let vm = (*cs).mVm;
    patch_offset(vm, begin_idx, false, (end_idx as LIntPtr).wrapping_sub(begin_idx as LIntPtr));
    patch_offset(vm, logic_idx, true, (end_idx as LIntPtr).wrapping_sub(logic_idx as LIntPtr));
    patch_offset(vm, end_idx, false, (end_idx as LIntPtr).wrapping_sub(begin_idx as LIntPtr));
}

unsafe fn do_statement(cs: *mut CompileState) {
    let begin_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdLoop).unwrap_or(0);
    block_statement(cs);
    next_token(cs);
    eval_expression(cs);
    let logic_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdLoopTrue).unwrap_or(0);
    let end_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdJmpTo).unwrap_or(0);
    let vm = (*cs).mVm;
    patch_offset(vm, begin_idx, false, (end_idx as LIntPtr).wrapping_sub(begin_idx as LIntPtr));
    patch_offset(vm, logic_idx, true, (end_idx as LIntPtr).wrapping_sub(logic_idx as LIntPtr));
    patch_offset(vm, end_idx, false, (end_idx as LIntPtr).wrapping_sub(begin_idx as LIntPtr));
}

/// ForStatement(cs) per BoyiaCore.cpp: NextToken('('); require LPTR; NextToken;
/// first: BY_VAR->LocalStatement else Putback+EvalExpression;
/// beginInst=kCmdLoop; EvalExpression; logicInst=kCmdLoopTrue; EvalExpression; require RPTR;
/// lastInst=kCmdJmpTo, lastInst->mOPLeft=(lastInst-beginInst); logicInst->mOPLeft=(lastInst-logicInst);
/// BlockStatement; endInst=kCmdJmpTo; beginInst->mOPLeft=(endInst-beginInst);
/// logicInst->mOPRight=(endInst-logicInst); endInst->mOPLeft=(endInst-logicInst).
unsafe fn for_statement(cs: *mut CompileState) {
    next_token(cs); // '('
    if (*cs).mToken.mTokenValue != TokenValue::LPTR {
        sntx_error_build(SntxError::LptrExpected, cs);
    }
    next_token(cs);
    // First expression
    if (*cs).mToken.mTokenValue == TokenValue::BY_VAR {
        local_statement(cs);
    } else {
        putback(cs);
        eval_expression(cs);
    }
    let begin_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdLoop).unwrap_or(0);
    // Second expression (condition)
    eval_expression(cs);
    let logic_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdLoopTrue).unwrap_or(0);
    // Third expression (e.g. i++)
    eval_expression(cs);
    if (*cs).mToken.mTokenValue != TokenValue::RPTR {
        sntx_error_build(SntxError::RptrExpected, cs);
    }
    let last_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdJmpTo).unwrap_or(0);
    let vm = (*cs).mVm;
    patch_offset(vm, last_idx, false, (last_idx as LIntPtr).wrapping_sub(begin_idx as LIntPtr));
    patch_offset(vm, logic_idx, false, (last_idx as LIntPtr).wrapping_sub(logic_idx as LIntPtr));
    block_statement(cs);
    let end_idx = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdJmpTo).unwrap_or(0);
    patch_offset(vm, begin_idx, false, (end_idx as LIntPtr).wrapping_sub(begin_idx as LIntPtr));
    patch_offset(vm, logic_idx, true, (end_idx as LIntPtr).wrapping_sub(logic_idx as LIntPtr));
    patch_offset(vm, end_idx, false, (end_idx as LIntPtr).wrapping_sub(logic_idx as LIntPtr));
}

/// BlockStatement: parse block {} contents. Matches C++ BlockStatement exactly.
/// do { NextToken; if IDENTIFIER -> Putback, EvalExpression, require SEMI; BLOCK_START -> block=true;
///      BLOCK_END -> block=false, return; KEYWORD -> switch; } while (token != BY_END && block);
///
/// When compiling inside a [FunctionScope], each `{` pushes a [LocalScope] and each matching `}` pops it
/// (nested blocks stack). Class / file-level [body_statement] with no active function skips push/pop.
unsafe fn block_statement(cs: *mut CompileState) {
    let mut block = false;
    loop {
        next_token(cs);
        if (*cs).mToken.mTokenType == TokenType::IDENTIFIER {
            putback(cs);
            eval_expression(cs);
            if (*cs).mToken.mTokenValue != TokenValue::SEMI {
                sntx_error_build(SntxError::SemiExpected, cs);
            }
        } else if (*cs).mToken.mTokenValue == TokenValue::BLOCK_START {
            block = true;
            if let Some(fs) = (*cs).mFunctionScopes.last_mut() {
                fs.push_local_scope();
            }
        } else if (*cs).mToken.mTokenValue == TokenValue::BLOCK_END {
            if let Some(fs) = (*cs).mFunctionScopes.last_mut() {
                let n = fs
                    .mLocalScopes
                    .last()
                    .map(|ls| ls.mLocals.len())
                    .unwrap_or(0);
                if n > 0 {
                    let _ = put_instruction(
                        cs,
                        OpCommand::const_number(n as LIntPtr),
                        OpCommand::none(),
                        CmdType::kCmdPopLocals,
                    );
                }
                let _ = fs.pop_local_scope();
            }
            return; // C++ sets block = LFalse then return; we skip the dead assign
        } else if (*cs).mToken.mTokenType == TokenType::KEYWORD {
            let v = (*cs).mToken.mTokenValue;
            if v == TokenValue::BY_VAR {
                local_statement(cs);
            } else if v == TokenValue::BY_FUNC || v == TokenValue::BY_ASYNC {
                let kw = if v == TokenValue::BY_FUNC { ValueType::BY_FUNC } else { ValueType::BY_ASYNC };
                fun_statement(cs, kw);
            } else if v == TokenValue::BY_PROP {
                prop_statement(cs);
            } else if v == TokenValue::BY_RETURN {
                return_statement(cs);
            } else if v == TokenValue::BY_IF || v == TokenValue::BY_ELIF {
                if_statement(cs);
            } else if v == TokenValue::BY_ELSE {
                else_statement(cs);
            } else if v == TokenValue::BY_WHILE {
                while_statement(cs);
            } else if v == TokenValue::BY_DO {
                do_statement(cs);
            } else if v == TokenValue::BY_FOR {
                for_statement(cs);
            } else if v == TokenValue::BY_BREAK {
                break_statement(cs);
            } else if v == TokenValue::BY_AWAIT {
                eval_await(cs);
            }
        }
        // C++: } while (cs->mToken.mTokenValue != BY_END && block);
        if (*cs).mToken.mTokenValue == TokenValue::BY_END || !block {
            break;
        }
    }
}

/// BodyStatement(CompileState* cs, LBool isFunction) per BoyiaCore.cpp (1:1).
/// CommandTable* cmds = cs->mCmds;
/// CommandTable tmpTable = { kBoyiaNull, kBoyiaNull };
/// Instruction* funInst = kBoyiaNull;
/// if (isFunction) { funInst = PutInstruction(ExecCreate); cs->mCmds = &tmpTable; }
/// BlockStatement(cs);
/// if (funInst && tmpTable.mBegin) { patch funInst; }
/// cs->mCmds = cmds;
/// In Rust, mCmds is embedded (no pointer switch); we clear mCmds when isFunction to simulate tmpTable.
/// Restore mBegin/mEnd only when isFunction. Restore mEnd to the ExecCreate instruction (not the pre-ExecCreate end)
/// so that the next instruction (e.g. CreateClass) is linked after ExecCreate and does not overwrite ParamCreate->mNext.
unsafe fn body_statement(cs: *mut CompileState, is_function: bool) {
    // CommandTable* cmds = cs->mCmds;
    let saved_begin = (*cs).mCmds.mBegin;
    let _saved_end = (*cs).mCmds.mEnd;
    // CommandTable tmpTable = { kBoyiaNull, kBoyiaNull }; Instruction* funInst = kBoyiaNull;
    let (fun_inst_idx, exec_create_end): (Option<usize>, *mut crate::types::Instruction) = if is_function {
        // if (isFunction) { funInst = PutInstruction(ExecCreate); cs->mCmds = &tmpTable; }
        let idx = put_instruction(
            cs,
            OpCommand::const_number(-1),
            OpCommand::const_number(-1),
            CmdType::kCmdExecCreate,
        );
        let exec_create_end = (*cs).mCmds.mEnd; // save ExecCreate as chain end before clearing
        create_executor(cs);
        (idx, exec_create_end)
    } else {
        (None, ptr::null_mut())
    };
    // BlockStatement(cs);
    block_statement(cs);
    // if (funInst && tmpTable.mBegin) { patch funInst; }
    if let Some(fun_idx) = fun_inst_idx {
        let tmp_begin = (*cs).mCmds.mBegin;
        let tmp_end = (*cs).mCmds.mEnd;
        if !tmp_begin.is_null() && !tmp_end.is_null() {
            let vm = (*cs).mVm;
            patch_offset(vm, fun_idx, false, inst_ptr_to_index(vm, tmp_begin) as LIntPtr);
            patch_offset(vm, fun_idx, true, inst_ptr_to_index(vm, tmp_end) as LIntPtr);
        }
    }
    // cs->mCmds = cmds; when isFunction restore so next instruction links after ExecCreate
    if is_function {
        (*cs).mCmds.mBegin = saved_begin;
        (*cs).mCmds.mEnd = exec_create_end; // restore end to ExecCreate, not saved_end (ParamCreate)
        let _ = (*cs).mFunctionScopes.pop();
    }
}

unsafe fn fun_statement(cs: *mut CompileState, fun_type: ValueType) {
    next_token(cs);
    let name_key = gen_identifier(cs);
    // Match C++ FunStatement: only emit instruction; runtime HandleFunCreate creates slot / global.
    let _ = put_instruction(
        cs,
        OpCommand::const_number(name_key as LIntPtr),
        OpCommand::const_number(fun_type as u8 as LIntPtr),
        CmdType::kCmdCreateFunction,
    );
    next_token(cs);
    init_params(cs);
    body_statement(cs, true);
}

unsafe fn prop_statement(cs: *mut CompileState) {
    next_token(cs);
    if (*cs).mToken.mTokenType == TokenType::IDENTIFIER {
        let name_key = gen_identifier(cs);
        let _ = put_instruction(
            cs,
            OpCommand::const_number(name_key as LIntPtr),
            OpCommand::none(),
            CmdType::kCmdPropCreate,
        );
        putback(cs);
        eval_expression(cs);
        if (*cs).mToken.mTokenValue != TokenValue::SEMI {
            sntx_error_build(SntxError::SemiExpected, cs);
        }
        return;
    }
    if (*cs).mToken.mTokenType == TokenType::KEYWORD {
        if (*cs).mToken.mTokenValue == TokenValue::BY_FUNC {
            fun_statement(cs, ValueType::BY_PROP_FUNC);
        } else if (*cs).mToken.mTokenValue == TokenValue::BY_ASYNC {
            fun_statement(cs, ValueType::BY_ASYNC_PROP);
        }
    } else {
        sntx_error_build(SntxError::Syntax, cs);
    }
}

/// ClassStatement(CompileState* cs) per BoyiaCore.cpp.
/// NextToken; classKey = GenIdentifier; PutInstruction(&cmd, kBoyiaNull, kCmdCreateClass);
/// NextToken; if (BY_EXTEND) NextToken, extendKey = GenIdentifier; else Putback(cs);
/// BodyStatement(cs, LFalse); if (extendKey) PutInstruction(&cmd, &extendCmd, kCmdClassExtend);
/// PutInstruction(&cmdEnd, kBoyiaNull, kCmdCreateClass) so CLASS is set to kBoyiaNull.
unsafe fn class_statement(cs: *mut CompileState) {
    next_token(cs);
    let class_key = gen_identifier(cs);
    let _ = put_instruction(
        cs,
        OpCommand::const_number(class_key as LIntPtr),
        OpCommand::none(),
        CmdType::kCmdCreateClass,
    );

    next_token(cs);
    let extend_key: LUintPtr = if (*cs).mToken.mTokenValue == TokenValue::BY_EXTEND {
        next_token(cs);
        gen_identifier(cs)
    } else {
        putback(cs);
        0
    };

    body_statement(cs, false);

    if extend_key != 0 {
        let _ = put_instruction(
            cs,
            OpCommand::const_number(class_key as LIntPtr),
            OpCommand::const_number(extend_key as LIntPtr),
            CmdType::kCmdClassExtend,
        );
    }

    let _ = put_instruction(cs, OpCommand::none(), OpCommand::none(), CmdType::kCmdCreateClass);
}

unsafe fn parse_statement(cs: *mut CompileState) {
    create_executor(cs);
    let mut brace: LInt = 0;
    loop {
        while brace > 0 {
            next_token(cs);
            if (*cs).mToken.mTokenValue == TokenValue::BLOCK_START {
                brace += 1;
            }
            if (*cs).mToken.mTokenValue == TokenValue::BLOCK_END {
                brace -= 1;
            }
        }
        next_token(cs);
        if (*cs).mToken.mTokenValue == TokenValue::BY_END {
            break;
        }
        if (*cs).mToken.mTokenValue == TokenValue::BY_VAR {
            global_statement(cs);
        } else if (*cs).mToken.mTokenValue == TokenValue::BY_FUNC {
            fun_statement(cs, ValueType::BY_FUNC);
        } else if (*cs).mToken.mTokenValue == TokenValue::BY_CLASS {
            class_statement(cs);
        } else if (*cs).mToken.mTokenType == TokenType::IDENTIFIER {
            putback(cs);
            eval_expression(cs);
            if (*cs).mToken.mTokenValue != TokenValue::SEMI {
                sntx_error_build(SntxError::SemiExpected, cs);
            }
        } else if (*cs).mToken.mTokenValue == TokenValue::BLOCK_START {
            brace += 1;
        }
    }
    // Nested compile / execute global (match ParseStatement after do-while).
    let vm = (*cs).mVm;
    let es = (*vm).mEState;
    if !es.is_null() && !(*es).mStackFrame.mContext.is_null() {
        let state = crate::core::create_exec_state(vm);
        crate::core::switch_exec_state(state, vm);
    }
    if !(*vm).mEState.is_null() {
        (*(*vm).mEState).mStackFrame.mContext = &mut (*cs).mCmds as *mut CommandTable;
        crate::execute::execute_code(vm as *mut LVoid);
    }
    if (*vm).mEState != es && !es.is_null() {
        crate::core::destroy_exec_state((*vm).mEState, vm);
        crate::core::switch_exec_state(es, vm);
    }
    append_entry(cs);
}

/// Debug: dump compiled instruction opcodes (when BOYIA_DEBUG_COMPILE=1).
unsafe fn dump_compiled_opcodes(cs: *const CompileState) {
    let vm = (*cs).mVm;
    let mut inst = (*cs).mCmds.mBegin;
    let mut n = 0usize;
    let mut n_call_native = 0usize;
    let mut n_after_last_call_native = 0usize;
    while !inst.is_null() {
        let op = (*inst).mOPCode;
        if op == CmdType::kCmdCallNative {
            n_call_native += 1;
            n_after_last_call_native = 0;
        } else {
            n_after_last_call_native += 1;
        }
        eprintln!("  [{}] {:?}", n, op);
        n += 1;
        let next_idx = (*inst).mNext;
        if next_idx == kInvalidInstruction {
            break;
        }
        inst = get_instruction_mut(vm, next_idx as usize);
    }
    eprintln!("[compile] total instructions={}, kCmdCallNative count={}, instructions after last kCmdCallNative={}", n, n_call_native, n_after_last_call_native);
}

/// Full compile: parse source, emit all instructions, register globals, append entry.
pub(crate) unsafe fn parse_and_register(code: *mut LInt8, vm: *mut BoyiaVM) {
    if code.is_null() || vm.is_null() {
        return;
    }
    let mut cs = CompileState {
        mProg: code as *mut LInt8,
        mLineNum: 1,
        mColumnNum: 0,
        mToken: BoyiaToken {
            mTokenName: BoyiaStr {
                mPtr: ptr::null_mut(),
                mLen: 0,
            },
            mTokenType: TokenType::NONE,
            mTokenValue: TokenValue::NONE,
        },
        mVm: vm,
        mCmds: CommandTable {
            mBegin: ptr::null_mut(),
            mEnd: ptr::null_mut(),
        },
        mFunctionScopes: Vec::new(),
    };
    parse_statement(&mut cs);
    if std::env::var("BOYIA_DEBUG_COMPILE").is_ok() {
        dump_compiled_opcodes(&cs);
    }
}
