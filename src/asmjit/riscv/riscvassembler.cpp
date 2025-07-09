// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#include "../core/support.h"
#include "../core/codewriter_p.h"
#include "../riscv/riscvassembler.h"
#include "../riscv/riscvinstdb.h"
#include "../riscv/riscvemithelper_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

// riscv::Assembler - Construction & Destruction
// ============================================

Assembler::Assembler(CodeHolder* code) noexcept : BaseAssembler() {
  _archMask = uint64_t(1) << uint32_t(Arch::kRISCV64);
  initEmitterFuncs(this);
  if (code)
    code->attach(this);
}

Assembler::~Assembler() noexcept {}

// riscv::Assembler - Emit
// =======================

Error Assembler::_emit(InstId instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* opExt) {
  const InstDB::InstInfo& info = InstDB::infoById(instId);
  uint32_t opcode = 0;
  CodeWriter writer(this);
  Error err = kErrorOk;
  err = writer.ensureSpace(this, 4);  // reserve 32bit

  switch (info.encoding()) {
    case InstDB::EncodingType::kR: {
      // [funct7 | rs2 | rs1 | funct3 | rd | opcode]
      const Gp& rd = o0.as<Gp>();
      const Gp& rs1 = o1.as<Gp>();
      const Gp& rs2 = o2.as<Gp>();

      opcode = info.opcode() |
               (uint32_t(rd.id()) << 7) |
               (info.funct3() << 12) |
               (uint32_t(rs1.id()) << 15) |
               (uint32_t(rs2.id()) << 20) |
               (info.funct7() << 25);
      break;
    }

    case InstDB::EncodingType::kI: {
      // [immediate[11:0] | rs1 | funct3 | rd | opcode]
      const Gp& rd = o0.as<Gp>();
      uint32_t rs1_id;
      int64_t immValue;

      if (info.hasFlag(InstDB::kIsLoad)) {
        const Mem& mem = o1.as<Mem>();
        if (!mem.hasBaseReg())
          return DebugUtils::errored(kErrorInvalidInstruction);
        rs1_id = mem.baseId();
        immValue = mem.offset();
      } else {
        rs1_id = o1.as<Gp>().id();
        immValue = o2.as<Imm>().value();
      }

      //if (!Support::isSigned<12>(immValue))
      //  return DebugUtils::errored(kErrorInvalidImmediate);

      uint32_t imm12 = uint32_t(immValue) & 0xFFF;
      opcode = info.opcode() |
               (uint32_t(rd.id()) << 7) |
               (info.funct3() << 12) |
               (rs1_id << 15) |
               (imm12 << 20);
      break;
    }

    case InstDB::EncodingType::kS: {
      // [immediate[11:5] | rs2 | rs1 | funct3 | immediate[4:0] | opcode]
      const Mem& mem = o1.as<Mem>();  // rs2 是 o0
      const Gp& rs2 = o0.as<Gp>();

      if (!mem.hasBaseReg())
        return DebugUtils::errored(kErrorInvalidInstruction);

      const Gp& rs1 = mem.baseReg().as<Gp>(); // 脱裤子放屁，可以直接rs1 = o1.as<Gp>
      int64_t offset = o2.as<Imm>().value();

      //if (!Support::isSigned<12>(offset))
      //  return DebugUtils::errored(kErrorInvalidDisplacement);

      uint32_t imm11_5 = (uint32_t(offset) >> 5) & 0x7F;
      uint32_t imm4_0 = uint32_t(offset) & 0x1F;

      opcode = info.opcode() |
               (imm4_0 << 7) |
               (info.funct3() << 12) |
               (uint32_t(rs1.id()) << 15) |
               (uint32_t(rs2.id()) << 20) |
               (imm11_5 << 25);
      break;
    }

    case InstDB::EncodingType::kB: {
      // [immediate[12|10:5] | rs2 | rs1 | funct3 | immediate[4:1|11] | opcode]
      const Gp& rs1 = o0.as<Gp>();
      const Gp& rs2 = o1.as<Gp>();
      const Imm& imm = o2.as<Imm>();
      int64_t offset = imm.value();

      //if ((offset & 1) != 0 || !Support::isSigned<13>(offset))
      //  return DebugUtils::errored(kErrorInvalidDisplacement);

      uint32_t uimm = uint32_t(offset);
      uint32_t imm11   = (uimm >> 11) & 1;
      uint32_t imm4_1  = (uimm >> 1) & 0xF;
      uint32_t imm10_5 = (uimm >> 5) & 0x3F;
      uint32_t imm12   = (uimm >> 12) & 1;

      opcode = info.opcode() |
               (imm11 << 7) |
               (imm4_1 << 8) |
               (info.funct3() << 12) |
               (uint32_t(rs1.id()) << 15) |
               (uint32_t(rs2.id()) << 20) |
               (imm10_5 << 25) |
               (imm12 << 31);
      break;
    }

    case InstDB::EncodingType::kU: {
      // [immediate[31:12] | rd | opcode]
      const Gp& rd = o0.as<Gp>();
      const Imm& imm = o1.as<Imm>();
      int64_t immValue = imm.value();

      //if (immValue < 0 || immValue >= (1 << 20))
      //  return DebugUtils::errored(kErrorInvalidImmediate);

      opcode = info.opcode() |
               (uint32_t(rd.id()) << 7) |
               ((uint32_t(immValue) << 12) << 12);
      break;
    }

    case InstDB::EncodingType::kJ: {
      // [immediate[20|10:1|11|19:12] | rd | opcode]
      const Gp& rd = o0.as<Gp>();
      const Imm& imm = o1.as<Imm>();
      int64_t offset = imm.value();

      //if ((offset & 1) != 0 || !Support::isSigned<21>(offset))
      //  return DebugUtils::errored(kErrorInvalidDisplacement);

      uint32_t uimm = uint32_t(offset);
      uint32_t imm20    = (uimm >> 20) & 1;
      uint32_t imm19_12 = (uimm >> 12) & 0xFF;
      uint32_t imm11    = (uimm >> 11) & 1;
      uint32_t imm10_1  = (uimm >> 1) & 0x3FF;

      opcode = info.opcode() |
               (uint32_t(rd.id()) << 7) |
               (imm19_12 << 12) |
               (imm11 << 20) |
               (imm10_1 << 21) |
               (imm20 << 31);
      break;
    }

    default:
      return DebugUtils::errored(kErrorInvalidInstruction);
  }
  writer.emit32uLE(opcode);

  resetState();

  writer.done(this);
  return err;
}

// riscv::Assembler - Align
// ========================

Error Assembler::align(AlignMode alignMode, uint32_t alignment) {
  // todo, align write code
  return kErrorOk;
}

// riscv::Assembler - Events
// =========================

Error Assembler::onAttach(CodeHolder& code) noexcept {
  ASMJIT_PROPAGATE(Base::onAttach(code));

  _instructionAlignment = uint8_t(4);
  updateEmitterFuncs(this);

  return kErrorOk;
}

Error Assembler::onDetach(CodeHolder& code) noexcept {
  return Base::onDetach(code);
}

ASMJIT_END_SUB_NAMESPACE