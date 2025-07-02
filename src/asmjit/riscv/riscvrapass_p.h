// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCVRAPASS_P_H_INCLUDED
#define ASMJIT_RISCV_RISCVRAPASS_P_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.hh"
#include "../core/rabuilders_p.h"
#include "../core/rapass_p.h"
#include "../riscv/riscvassembler.h"
#include "../riscv/riscvcompiler.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

//! \cond INTERNAL
//! \addtogroup asmjit_riscv
//! \{

//! RISC-V register allocation pass.
class RISCVRAPass : public BaseRAPass {
public:
  ASMJIT_NONCOPYABLE(RISCVRAPass)
  using Base = BaseRAPass;

  //! \name Construction & Destruction
  //! \{

  RISCVRAPass() noexcept;
  ~RISCVRAPass() noexcept override;

  //! \}

  //! \name Accessors
  //! \{

  ASMJIT_INLINE_NODEBUG Compiler* cc() const noexcept { return static_cast<Compiler*>(_cb); }

  //! \}

protected:
  //! \name Overrides
  //! \{

  void onInit() noexcept override;
  void onDone() noexcept override;

  Error emitMove(uint32_t workId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept override;
  Error emitSwap(uint32_t aWorkId, uint32_t aPhysId, uint32_t bWorkId, uint32_t bPhysId) noexcept override;

  Error emitLoad(uint32_t workId, uint32_t dstPhysId) noexcept override;
  Error emitSave(uint32_t workId, uint32_t srcPhysId) noexcept override;

  Error emitJump(const Label& label) noexcept override;
  Error emitPreCall(InvokeNode* invokeNode) noexcept override;

  //! \}
};

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_RISCV_RISCVRAPASS_P_H_INCLUDED