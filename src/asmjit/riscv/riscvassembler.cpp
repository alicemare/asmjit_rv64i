// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#include "../core/support.h"
#include "../riscv/riscvassembler.h"
#include "../riscv/riscvinstdb.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

// riscv::Assembler - Construction & Destruction
// ============================================

Assembler::Assembler(CodeHolder* code) noexcept : BaseAssembler() {
  if (code)
    attach(code);
}

Assembler::~Assembler() noexcept {}

// riscv::Assembler - Emit
// =======================

Error Assembler::_emit(InstId instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* opExt) {
  const InstDB::InstInfo& instInfo = InstDB::instInfo(instId);
  const InstDB::EncodingData& enc = InstDB::encodingData[instInfo.encodingId()];

  switch (enc.type()) {
    case InstDB::EncodingType::kR:
      return _emitR(instId, o0.as<Gp>(), o1.as<Gp>(), o2.as<Gp>());
    case InstDB::EncodingType::kI:
      return _emitI(instId, o0.as<Gp>(), o1.as<Gp>(), o2.as<Imm>());
    case InstDB::EncodingType::kS:
      return _emitS(instId, o0.as<Mem>(), o1.as<Gp>());
    case InstDB::EncodingType::kB:
      return _emitB(instId, o0.as<Gp>(), o1.as<Gp>(), o2.as<Label>());
    case InstDB::EncodingType::kU:
      return _emitU(instId, o0.as<Gp>(), o1.as<Imm>());
    case InstDB::EncodingType::kJ:
      return _emitJ(instId, o0.as<Gp>(), o1.as<Label>());
    default:
      return DebugUtils::errored(kErrorInvalidInstruction);
  }
}

// riscv::Assembler - Align
// ========================

Error Assembler::align(AlignMode alignMode, uint32_t alignment) {
  if (ASMJIT_UNLIKELY(alignment > 64 || !Support::isPowerOf2(alignment)))
    return DebugUtils::errored(kErrorInvalidArgument);

  if (isAligning()) {
    if (alignment > _alignMaxAhead)
      _alignMaxAhead = alignment;
    return kErrorOk;
  }

  uint32_t i = Support::alignUp(offset(), alignment) - offset();
  if (i == 0)
    return kErrorOk;

  if (ASMJIT_UNLIKELY(cursor() + i > endOfBuffer()))
    return reportError(kErrorNoHeapMemory);

  for (; i != 0; i -= 4)
    nop();

  return kErrorOk;
}

// riscv::Assembler - Events
// =========================

Error Assembler::onAttach(CodeHolder& code) noexcept {
  Arch arch = code.arch();
  if (!Arch::isRiscvFamily(arch))
    return DebugUtils::errored(kErrorInvalidArch);

  ASMJIT_PROPAGATE(Base::onAttach(code));
  return kErrorOk;
}

Error Assembler::onDetach(CodeHolder& code) noexcept {
  return Base::onDetach(code);
}

ASMJIT_END_SUB_NAMESPACE