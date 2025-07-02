// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCVINSTAPI_P_H_INCLUDED
#define ASMJIT_RISCV_RISCVINSTAPI_P_H_INCLUDED

#include "../core/inst.h"
#include "../core/operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

namespace InstInternal {

#ifndef ASMJIT_NO_TEXT
Error instIdToString(InstId instId, InstStringifyOptions options, String& output) noexcept;
InstId stringToInstId(const char* s, size_t len) noexcept;
#endif // !ASMJIT_NO_TEXT

#ifndef ASMJIT_NO_VALIDATION
Error validate(const BaseInst& inst, const Operand_* operands, size_t opCount, ValidationFlags validationFlags) noexcept;
#endif // !ASMJIT_NO_VALIDATION

#ifndef ASMJIT_NO_INTROSPECTION
Error queryRWInfo(const BaseInst& inst, const Operand_* operands, size_t opCount, InstRWInfo* out) noexcept;
Error queryFeatures(const BaseInst& inst, const Operand_* operands, size_t opCount, CpuFeatures* out) noexcept;
#endif // !ASMJIT_NO_INTROSPECTION

} // {InstInternal}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCVINSTAPI_P_H_INCLUDED