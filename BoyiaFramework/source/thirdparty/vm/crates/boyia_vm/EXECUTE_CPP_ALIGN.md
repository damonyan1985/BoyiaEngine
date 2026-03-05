# execute.rs 与 BoyiaCore.cpp 对照检查

本文档记录 execute.rs 中各函数与 BoyiaCore.cpp 对应实现的对照结论及已修复项。

## 已按 C++ 严格对齐或已修复

### reset_scene / ResetScene
- 已对齐：mLValSize, mLoopSize, mResultNum, mContext, mPC, mLast, mTopTask, mFrameIndex, mFun.mValue.mObj.mPtr/mSuper, mFun.mValueType=BY_ARG, mWait=LFalse, AssignStateClass(state, null)。

### handle_push_obj / HandlePushObj
- **已修复**：C++ 为 `mExecStack[mFrameIndex].mClass = mStackFrame.mClass`（写入**当前**帧），原 Rust 误写为上一帧 (frame_index-1)，已改为写入当前帧；并补充 kBoyiaSuper 判断，仅非 super 时 AssignStateClass(Reg0)。

### handle_push_scene / HandlePushScene
- **已修复**：C++ 不在此处设置 mClass（mClass 仅由 HandlePushObj 设置），已从 handle_push_scene 中移除对 mClass 的赋值。

### handle_fun_create / HandleFunCreate
- 已按 C++ 分支顺序实现：类内非匿名 → 类内匿名 (SetIntResult/补丁 inst) → 否则 CreateFunVal；IsObjectPropFunc 含四种类型。

### handle_create_executor / HandleCreateExecutor
- 逻辑一致：分配 CommandTable，用 mOPLeft/mOPRight 作偏移填 mBegin/mEnd，写回 mFunTable[mFunSize-1].mFuncBody。

### handle_call_function / HandleCallFunction
- 逻辑一致：取 locals[start]、ValueCopy(mFun)、BY_NAV_FUNC/BY_NAV_PROP → LocalPush+navFun；BY_PROP_FUNC/BY_ANONYM_FUNC → AssignStateClass(val)；最后 cmds = mFuncBody，设 mContext/mPC，return kOpResultJumpFun。

### handle_push_params / HandlePushParams
- 逻辑一致：仅对 BY_FUNC/BY_PROP_FUNC/BY_ASYNC_PROP/BY_ANONYM_FUNC 且 mParamSize>0 时，从 start+1 起给 mLocals[idx].mNameKey 赋形参 key。Rust 多写了 mValueType/mValue，语义等价（C++ 仅改 mNameKey）。

### handle_pop_scene / HandlePopScene
- 一致：--mFrameIndex，从 mExecStack[idx] 恢复 mLValSize, mPC, mContext, mLoopSize, mClass，并 AssignStateClass(mClass)。

### exec_instruction / ExecInstruction
- 一致：若 mPC 为空则 ExecPopFunction 并 return；循环内 dispatch，kOpResultEnd 则 break；若 mPC 非空且 kOpResultJumpFun 则 continue；否则 mPC = NextInstruction(mPC)，ExecPopFunction。

### exec_pop_function / ExecPopFunction
- 一致：若 mPC 非空直接 return；然后处理 mFrameIndex<=0 与 mLast 的切换；再若 mFrameIndex>0 则 HandlePopScene，若 mPC 非空则 mPC=NextInstruction 并递归 ExecPopFunction。

### handle_assignment / HandleAssignment
- 一致：OP_CONST_STRING/OP_CONST_NUMBER/OP_VAR(FindVal, BY_VAR 则 ValueCopy 否则 ValueCopyNoName+left->mNameKey=val)/OP_REG0/OP_REG1，无最后拷贝到 Reg0。

### handle_assign_var / HandleAssignVar
- 一致：left 为 BY_VAR 则 value=FindVal，否则 value=(BoyiaValue*)left->mNameKey；ValueCopyNoName(value, result)；ValueCopy(Reg0, value)。

### handle_create_class / HandleCreateClass
- 一致：OP_NONE 则 AssignStateClass(null)；否则 AssignStateClass(CreateFunVal(hashKey, BY_CLASS, vm))。

### handle_extend / HandleExtend
- 一致：FindGlobal 取 class/extend，classVal->mValue.mObj.mSuper = extendVal。

### handle_create_param / HandleCreateParam
- 一致：function = mFunTable[mFunSize-1]，value = &function->mParams[function->mParamSize++]，value->mNameKey = hashKey。Rust 多设了 mValueType=BY_ARG、mValue=0，与 C++ 零初始化等价。

### handle_create_prop / HandleCreateProp
- 一致：func = mStackFrame.mClass.mValue.mObj.mPtr，func->mParams[func->mParamSize].mNameKey = inst->mOPLeft.mValue，mValue.mIntVal = 0，mParamSize++。

### handle_return / HandleReturn
- 一致：mStackFrame.mPC = mContext->mEnd。

### handle_break / HandleBreak
- 一致：无操作，仅 return kOpResultSuccess。

### handle_get_prop / HandleGetProp
- 一致：GetOpValue(Left)，FindObjProp(lval, rval)，ValueCopy(Reg0, result)，Reg0.mNameKey = result.mNameKey。

### assign_state_class / AssignStateClass
- 一致：value 为空则 mClass.mValue.mObj.mPtr/Super=0，否则 ValueCopyNoName(mClass, value)；mClass.mValueType = BY_CLASS。

---

## 与 C++ 的已知差异（未改）

### HandleAdd
- C++ 除整数外还处理字符串与类（GetBoyiaClassId、StringAdd）；Rust 仅实现整数加法，字符串/类加法未实现。

### HandlePushParams 局部变量写入
- C++ 只写 `mLocals[idx].mNameKey = vKey`；Rust 写整个 BoyiaValue（mNameKey, mValueType=BY_ARG, mValue=0）。行为等价，仅实现方式不同。

### SntxError / GetCodeRow
- HandlePushScene 在 C++ 中在嵌套过深时调用 SntxError(NEST_FUNC, GetCodeRow(vm))；Rust 仅 return kOpResultEnd，未做错误报告。

### HandleCallFunction 中 BY_ASYNC_PROP / BY_ASYNC
- C++ 有 HandleCallAsyncFunction 及空分支；Rust 未实现 HandleCallAsyncFunction，BY_ASYNC_PROP/BY_ASYNC 未单独处理。

---

## 建议后续对齐项

1. **HandleAdd**：按 C++ 增加字符串/类加法（GetBoyiaClassId、StringAdd）。
2. **HandlePushParams**：若追求与 C++ 完全一致，可改为只写 mNameKey。
3. **HandlePushScene**：嵌套过深时增加与 SntxError 等价的错误上报（若运行时需要）。
4. **HandleCallAsyncFunction**：实现异步调用分支，并处理 BY_ASYNC_PROP/BY_ASYNC。
