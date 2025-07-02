// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/func.h"
#include "../riscv/riscvcompiler.h"
#include "../riscv/riscvrapass_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

// riscv::Compiler - Construction & Destruction
// ============================================

Compiler::Compiler(CodeHolder* code) noexcept : BaseCompiler() {
  if (code)
    attach(code);
}

Compiler::~Compiler() noexcept {}

// riscv::Compiler - Overrides
// ===========================

Error Compiler::onInit() noexcept {
  addPassT<RISCVRAPass>();
  return Base::onInit();
}

Error Compiler::onFini() noexcept {
  return Base::onFini();
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER