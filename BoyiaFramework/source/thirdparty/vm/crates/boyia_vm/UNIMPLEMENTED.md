# boyia_vm 未实现 / 占位项检查

## 已修复

- **execute_global_code**：原在 `core.rs` 中为空实现，已改为在 `execute.rs` 中按 C++ `ExecuteGlobalCode` 实现（按 entry 表设置 mContext 并调用 execute_code），并从 `execute` 导出。

---

## 1. core.rs

### native_call_impl（已实现）

- **位置**：`core.rs`  
- **行为**：与 C++ `NativeCallImpl` 一致：创建新 ExecState、切换、push_scene(null)、拷贝参数、根据 args[0] 类型调用原生或脚本、销毁 state、切回原 state。

---

## 2. execute.rs：指令 (CmdType)

`dispatch_instruction` 已为所有 CmdType 提供分支，不再落入未实现占位。

### 已实现指令（含本次补齐）

- **控制流**：JmpTrue, OnceJmpTrue, IfEnd, JmpTo, Loop, LoopTrue, Return, Break；**Elif, Else, ElEnd, End**（已实现，多为 no-op/最小逻辑）。
- **运算**：Add, Sub, Mul, Div, Mod；**关系运算** Gt/Lt/Ge/Le/Not/Eq/Ne（handle_relational）；**逻辑** And/Or（handle_logic）。
- **栈与场景**：Push, Pop（按 C++：非 REG0/REG1 仅减 mResultNum）, PushScene, PopScene, PushArg, PushObj, Assign。
- **变量与属性**：DeclGlobal, DeclLocal, **AssignVar**（含 get_val 写回）, **GetProp**（find_obj_prop + 写 Reg0）。
- **调用**：**PushParams**, **CallFunction**（含 BY_NAV_FUNC/BY_NAV_PROP 原生调用与 BY_PROP_FUNC/BY_ANONYM_FUNC 跳转）, CallNative。
- **类与函数**：**CreateClass**, **ClassExtend**, **FunCreate**, **ExecCreate** / **CreateFunction**（创建 CommandTable）, **ParamCreate**, **PropCreate**。
- **数据结构**：**ConstStr**（create_string_object）, **CreateMap**, **SetMapKey**, **SetMapValue**, **CreateArray**, **AddArrayItem**。
- **其它**：**DeclConstStr**（Success）, **Await**（已实现：mWait、MicroTask、mTopTask、SetNativeResult、SwitchExecState）, **SetAnonym**（已实现：从 mFunTable 取函数、BY_ANONYM_FUNC + mSuper 为当前 class 对象、SetNativeResult）。**Else/ElEnd/End** 为 no-op（与 C++ kBoyiaNull 一致）。

辅助逻辑已对齐 C++：**get_val**（先 locals 再 global）、**assign_state_class**、**handle_return** 使用 `mContext->mEnd`、**handle_pop** 仅对 REG0/REG1 写回寄存器。
