// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#ifndef ASMJIT_NO_COMPILER

// #include "../riscv/riscvassembler.h"
#include "../riscv/riscvcompiler.h"
#include "riscvemithelper_p.h"
#include "../riscv/riscvassembler.h"
#include "../riscv/riscvrapass_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

// riscv::Compiler - Construction & Destruction
// ============================================

Compiler::Compiler(CodeHolder* code) noexcept : BaseCompiler() {
  if (code)
    code->attach(this);
}

Compiler::~Compiler() noexcept {}

// riscv::Compiler - Overrides
// ===========================

Error Compiler::onAttach(CodeHolder& code) noexcept {
  ASMJIT_PROPAGATE(Base::onAttach(code));
  initEmitterFuncs(this);

  Error err = addPassT<RISCVRAPass>();
  if (err) {
    onDetach(code);
    return err;
  }

  return kErrorOk;
}

Error Compiler::onDetach(CodeHolder& code) noexcept {
  return Base::onDetach(code);
}

Error Compiler::finalize() {
  ASMJIT_PROPAGATE(runPasses());

  Assembler a(_code);
  a.addEncodingOptions(encodingOptions());

  return serializeTo(&a);
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER