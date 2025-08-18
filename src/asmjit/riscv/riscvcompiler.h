// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCVCOMPILER_H_INCLUDED
#define ASMJIT_RISCV_RISCVCOMPILER_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/type.h"
#include "../riscv/riscvemitter.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

//! \addtogroup asmjit_riscv
//! \{

//! RISC-V compiler implementation.
class ASMJIT_VIRTAPI Compiler
  : public BaseCompiler,
    public EmitterExplicitT<Compiler> {
public:
  ASMJIT_NONCOPYABLE(Compiler)
  using Base = BaseCompiler;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API explicit Compiler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API ~Compiler() noexcept override;

  //! \}

  //! \name Virtual Registers
  //! \{

  //! \cond INTERNAL
  template<typename RegT, typename Type>
  ASMJIT_INLINE_NODEBUG RegT _newRegInternal(const Type& type) {
    RegT reg(Globals::NoInit);
    _newReg(&reg, type, nullptr);
    return reg;
  }

  template<typename RegT, typename Type>
  ASMJIT_INLINE_NODEBUG RegT _newRegInternal(const Type& type, const char* s) {
#ifndef ASMJIT_NO_LOGGING
    RegT reg(Globals::NoInit);
    _newReg(&reg, type, s);
    return reg;
#else
    DebugUtils::unused(s);
    return _newRegInternal<RegT>(type);
#endif
  }

  template<typename RegT, typename Type, typename... Args>
  ASMJIT_INLINE_NODEBUG RegT _newRegInternal(const Type& type, const char* s, Args&&... args) {
#ifndef ASMJIT_NO_LOGGING
    RegT reg(Globals::NoInit);
    _newRegFmt(&reg, type, s, std::forward<Args>(args)...);
    return reg;
#else
    DebugUtils::unused(s, std::forward<Args>(args)...);
    return _newRegInternal<RegT>(type);
#endif
  }
  //! \endcond

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Reg newReg(TypeId typeId, Args&&... args) { return _newRegInternal<Reg>(typeId, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGp(TypeId typeId, Args&&... args) { return _newRegInternal<Gp>(typeId, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newInt32(Args&&... args) { return _newRegInternal<Gp>(TypeId::kInt32, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newUInt32(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUInt32, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newInt64(Args&&... args) { return _newRegInternal<Gp>(TypeId::kInt64, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newUInt64(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUInt64, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newIntPtr(Args&&... args) { return _newRegInternal<Gp>(TypeId::kIntPtr, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newUIntPtr(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGp32(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUInt32, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGp64(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUInt64, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGpw(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUInt32, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGpx(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUInt64, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGpz(Args&&... args) { return _newRegInternal<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }

  //! \}

  //! \name Stack
  //! \{

  //! Creates a new memory chunk allocated on the current function's stack.
  ASMJIT_INLINE_NODEBUG Mem newStack(uint32_t size, uint32_t alignment, const char* name = nullptr) {
    Mem m(Globals::NoInit);
    _newStack(&m, size, alignment, name);
    return m;
  }

  //! \}

  //! \name Constants
  //! \{

  //! Put data to a constant-pool and get a memory reference to it.
  ASMJIT_INLINE_NODEBUG Mem newConst(ConstPoolScope scope, const void* data, size_t size) {
    Mem m(Globals::NoInit);
    _newConst(&m, scope, data, size);
    return m;
  }

  //! \}

  //! \name Compiler specific
  //! \{

  //! Special pseudo-instruction that can be used to load a memory address into `o0` GP register.
  //!
  //! \note At the moment this instruction is only useful to load a stack allocated address into a GP register
  //! for further use. It makes very little sense to use it for anything else. The semantics of this instruction
  //! is the same as X86 `LEA` (load effective address) instruction.
  Error loadAddressOf(const Gp& dst, const Mem& mem) {
    Gp dstX = Gp::make_x(dst.id());
    // dst 必须是 x 寄存器
    if (!mem.hasBaseReg()) {  // move offset to dst
      return mov(dstX, Imm(mem.offset()));
    }

    int64_t val = mem.offset();
    const Gp& baseReg = Gp::make_x(mem.baseId());
    if (val == 0) { // move baseReg to dst
      return addi(dstX, baseReg, Imm(0));
    }

    if (val >= -2048 && val < 2048) {
      return addi(dst, baseReg, Imm(val));
    }

    // 超过 12位 Imm, 需要分解
    if (dstX.id() != baseReg.id()) {
      // 可以使用 dst 作为临时寄存器
      ASMJIT_PROPAGATE(li(dstX, val));
      return add(dstX, dstX, baseReg);
    } else {
      printf("Have not support!");
      return DebugUtils::errored(kErrorInvalidState);
    }
  }

  Error loadImm(const Gp& dst, int64_t val) {
    Mem constAddr = newConst(ConstPoolScope::kGlobal, &val, sizeof(val));
    return ld(dst, constAddr);
  }

  //! \}

  //! \name Function Call & Ret Intrinsics
  //! \{

  ASMJIT_INLINE_NODEBUG Error invoke_(InvokeNode** out, const Operand_& target, const FuncSignature& signature) {
    return addInvokeNode(out, Inst::kIdJalr, target, signature);
  }

  ASMJIT_INLINE_NODEBUG Error invoke(InvokeNode** out, const Gp& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  ASMJIT_INLINE_NODEBUG Error invoke(InvokeNode** out, const Mem& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  ASMJIT_INLINE_NODEBUG Error invoke(InvokeNode** out, const Label& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  ASMJIT_INLINE_NODEBUG Error invoke(InvokeNode** out, const Imm& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  ASMJIT_INLINE_NODEBUG Error invoke(InvokeNode** out, uint64_t target, const FuncSignature& signature) { return invoke_(out, Imm(int64_t(target)), signature); }

  ASMJIT_INLINE_NODEBUG Error ret() { return addRet(Operand(), Operand()); }
  ASMJIT_INLINE_NODEBUG Error ret(const Reg& o0) { return addRet(o0, Operand()); }

  //! \}

protected:
  //! \name Overrides
  //! \{

  ASMJIT_API Error onAttach(CodeHolder& code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder& code) noexcept override;

  //! \}

public:
  ASMJIT_API Error finalize() override;
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_RISCV_RISCVCOMPILER_H_INCLUDED