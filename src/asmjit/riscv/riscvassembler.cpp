// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#include "../core/support.h"
#include "../core/codewriter_p.h"
#include "../core/emitterutils_p.h"
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
  if (instId == riscv::Inst::kIdAdr) {
    /// should not emit adr instruction!
    return DebugUtils::errored(kErrorInvalidInstruction);
  }

  const InstDB::InstInfo& info = InstDB::infoById(instId);
  uint32_t opcode = 0;
  CodeWriter writer(this);
  Error err = kErrorOk;
  err = writer.ensureSpace(this, 4);  // reserve 32bit for each Instruction

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
      // [shtyp[11:5]] | imm[4:0] | rs1 | func3 | rd | opcode] 
      uint32_t rd_id, rs1_id, imm12;
      if (info.hasFlag(InstDB::kIsLoad)) {
        rd_id = o0.as<Gp>().id();
        const Mem& mem = o1.as<Mem>();
        if (!mem.hasBaseReg()) {  // todo
          ASMJIT_PROPAGATE(writer.ensureSpace(this, 8));
          printf("Processing constAddr with label!\n");
          LabelEntry& le = _code->labelEntry(o1.as<Label>());
          // 为 AUIPC+LD 组合创建特殊的 fixup
          size_t auipcOffset = writer.offsetFrom(_bufferData);
                
          // 使用新的组合类型
          OffsetFormat format;
          format._type = OffsetType::kRISCV64_AUIPC_LD;
          format._flags = 0;
          format._regionSize = 8;    // 两条指令共 8 字节
          format._valueSize = 4;      // 每条指令 4 字节
          format._valueOffset = 0;
          format._immBitCount = 32;   // 完整的 32 位偏移
          format._immBitShift = 0;
          format._immDiscardLsb = 0;

          Fixup* fixup = _code->newFixup(le, _section->sectionId(), auipcOffset, 0, format);

          // 发射占位指令
          uint32_t auipc_opcode = 0b0010111 | (rd_id << 7);  // AUIPC rd, 0
          writer.emit32uLE(auipc_opcode);

          uint32_t ld_opcode = 0b0000011 | (rd_id << 7) | (0b011 << 12) | (rd_id << 15);  // LD rd, 0(rd)
          writer.emit32uLE(ld_opcode);

          //printf("Created AUIPC+LD fixup at offset %zu for label %u\n", auipcOffset, le.id());
          goto EmitDone;

        } else {
          rs1_id = mem.baseId();
          imm12 = uint32_t(mem.offset());
        }
      } else {
        rd_id = o0.as<Gp>().id();
        rs1_id = o1.as<Gp>().id();
        imm12 = (o2.as<Imm>().value() & 0xfff) | (info.funct7() << 5);
      }

      //if (!Support::isSigned<12>(immValue))
      //  return DebugUtils::errored(kErrorInvalidImmediate);

      opcode = info.opcode() |
               (rd_id << 7) |
               (info.funct3() << 12) |
               (rs1_id << 15) |
               (imm12 << 20);
      break;
    }

    case InstDB::EncodingType::kS: {
      // [immediate[11:5] | rs2 | rs1 | funct3 | immediate[4:0] | opcode]
      const Mem& rs1 = o1.as<Mem>();
      const Gp& rs2 = o0.as<Gp>();

      if (!rs1.hasBaseReg())
        return DebugUtils::errored(kErrorInvalidInstruction);

      uint32_t rs1_id = rs1.baseId();
      int64_t immValue = rs1.offset();

      //if (!Support::isSigned<12>(offset))
      //  return DebugUtils::errored(kErrorInvalidDisplacement);

      uint32_t imm11_5 = (uint32_t(immValue) >> 5) & 0x7F;
      uint32_t imm4_0 = uint32_t(immValue) & 0x1F;

      opcode = info.opcode() |
               (imm4_0 << 7) |
               (info.funct3() << 12) |
               (uint32_t(rs1_id) << 15) |
               (uint32_t(rs2.id()) << 20) |
               (imm11_5 << 25);
      break;
    }

    case InstDB::EncodingType::kB: {
      // [immediate[12|10:5] | rs2 | rs1 | funct3 | immediate[4:1|11] | opcode]
      const Gp& rs1 = o0.as<Gp>();
      const Gp& rs2 = o1.as<Gp>();
      const Label& label = o2.as<Label>();
      LabelEntry& le = _code->labelEntry(label.id());
      uint64_t offsetValue = 0;
      if (le.isBoundTo(_section)) {
        // target - currentPc
        offsetValue = le.offset() - uint64_t(offset());
      } else {
        printf("Create a fixup referencing an non-bound label\n");
        // [[TODO]]
        size_t codeOffset = writer.offsetFrom(_bufferData);
        // RISCV B-Type OffsetFormat sign(imm[12:1]] << 1)
        OffsetFormat offsetFormat;
        offsetFormat.resetToImmValue(OffsetType::kRISCV64_BType, 4, 0, 13, 1);
        
        Fixup* fixup = _code->newFixup(le, _section->sectionId(), codeOffset, 0, offsetFormat);
      }

      uint32_t imm11   = (offsetValue >> 11) & 1;
      uint32_t imm4_1  = (offsetValue >> 1) & 0xF;
      uint32_t imm10_5 = (offsetValue >> 5) & 0x3F;
      uint32_t imm12   = (offsetValue >> 12) & 1;

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
               (uint32_t(immValue) << 12);
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
      printf("auipc opcode: %x\n", opcode);
      break;
    }

    default:
      return DebugUtils::errored(kErrorInvalidInstruction);
  }
  writer.emit32uLE(opcode);

#ifndef ASMJIT_NO_LOGGING
  if (_logger) {
    EmitterUtils::logInstructionEmitted(this, instId, InstOptions::kReserved, o0, o1, o2, opExt, 0, 0, writer.cursor());
  }
#endif
  resetState();
  writer.done(this);

  return err;

EmitDone:
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