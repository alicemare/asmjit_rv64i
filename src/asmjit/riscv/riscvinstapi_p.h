// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCVINSTAPI_P_H_INCLUDED
#define ASMJIT_RISCV_RISCVINSTAPI_P_H_INCLUDED

#include "../core/inst.h"
#include "../riscv/riscvinstdb.h"
#include "../riscv/riscvoperand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

namespace InstInternal {
#ifndef ASMJIT_NO_TEXT
//! Converts an instruction ID into a null terminated string.
//!
//! \param instId Instruction ID.
//! \param options Options, reserved for future use.
//! \param output A string builder that will receive the output.
//!
//! \return Error code.

Error instIdToString(InstId instId, InstStringifyOptions options, String& output) noexcept;

//! Converts a string into an instruction ID.
//!
//! \param s String to convert to an instruction ID.
//! \param len Length of the string in bytes.
//!
//! \return Instruction ID or `BaseInst::kIdNone` if the instruction name is invalid.

InstId stringToInstId(const char* s, size_t len) noexcept;
#endif // !ASMJIT_NO_TEXT

#ifndef ASMJIT_NO_VALIDATION
//! Validates an instruction.
//!
//! \param inst Instruction to validate.
//! \param operands Instruction operands.
//! \param opCount Number of operands.
//! \param validationFlags Validation flags, see \ref ValidationFlags.
//!
//! \return Error code.

Error validate(const BaseInst& inst, const Operand_* operands, size_t opCount, ValidationFlags validationFlags) noexcept;
#endif // !ASMJIT_NO_VALIDATION

#ifndef ASMJIT_NO_INTROSPECTION
//! Queries information about an instruction.
//!
//! \param inst Instruction to query.
//! \param operands Instruction operands.
//! \param opCount Number of operands.
//! \param out Information to fill.
//!
//! \return Error code.

Error queryRWInfo(const BaseInst& inst, const Operand_* operands, size_t opCount, InstRWInfo* out) noexcept;

//! Queries CPU features required by an instruction.
//!
//! \param inst Instruction to query.
//! \param operands Instruction operands.
//! \param opCount Number of operands.
//! \param out Information to fill.
//!
//! \return Error code.

Error queryFeatures(const BaseInst& inst, const Operand_* operands, size_t opCount, CpuFeatures* out) noexcept;
#endif // !ASMJIT_NO_INTROSPECTION

} // {InstInternal}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCVINSTAPI_P_H_INCLUDED