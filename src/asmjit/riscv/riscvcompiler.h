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
  //! \endcond

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp newGpr(Args&&... args) { return _newRegInternal<Gp>(TypeId::kIntPtr, std::forward<Args>(args)...); }

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

protected:
  //! \name Overrides
  //! \{

  ASMJIT_API Error onInit() noexcept override;
  ASMJIT_API Error onFini() noexcept override;

  //! \}
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_RISCV_RISCVCOMPILER_H_INCLUDED