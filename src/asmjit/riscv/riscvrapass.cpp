// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/func.h"
#include "../core/inst.h"
#include "../riscv/riscvcompiler.h"
#include "../riscv/riscvrapass_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

RISCVRAPass::RISCVRAPass() noexcept : BaseRAPass() {}
RISCVRAPass::~RISCVRAPass() noexcept {}

void RISCVRAPass::onInit() noexcept {
  BaseRAPass::onInit();
}

void RISCVRAPass::onDone() noexcept {
  BaseRAPass::onDone();
}

Error RISCVRAPass::emitMove(uint32_t workId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept {
  // TODO:
  return kErrorOk;
}

Error RISCVRAPass::emitSwap(uint32_t aWorkId, uint32_t aPhysId, uint32_t bWorkId, uint32_t bPhysId) noexcept {
  // TODO:
  return kErrorOk;
}

Error RISCVRAPass::emitLoad(uint32_t workId, uint32_t dstPhysId) noexcept {
  // TODO:
  return kErrorOk;
}

Error RISCVRAPass::emitSave(uint32_t workId, uint32_t srcPhysId) noexcept {
  // TODO:
  return kErrorOk;
}

Error RISCVRAPass::emitJump(const Label& label) noexcept {
  // TODO:
  return kErrorOk;
}

Error RISCVRAPass::emitPreCall(InvokeNode* invokeNode) noexcept {
  // TODO:
  return kErrorOk;
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER