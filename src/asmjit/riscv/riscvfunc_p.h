// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCVFUNC_P_H_INCLUDED
#define ASMJIT_RISCV_RISCVFUNC_P_H_INCLUDED

#include "../core/func.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

//! \cond INTERNAL
//! \addtogroup asmjit_riscv
//! \{

//! RV64I-specific function API (calling conventions and other utilities).
namespace FuncInternal {

//! Initialize `CallConv` structure (RV64I specific).
Error initCallConv(CallConv& cc, CallConvId ccId, const Environment& environment) noexcept;

//! Initialize `FuncDetail` (RV64I specific).
Error initFuncDetail(FuncDetail& func, const FuncSignature& signature) noexcept;

} // {FuncInternal}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCVFUNC_P_H_INCLUDED
