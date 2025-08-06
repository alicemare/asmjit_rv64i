// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib


#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_RISCV) && !defined(ASMJIT_NO_LOGGING)

#include "../core/formatter_p.h"
#include "../core/misc_p.h"
#include "../core/support.h"
#include "../riscv/riscvformatter_p.h"
#include "../riscv/riscvinstapi_p.h"
#include "../riscv/riscvinstdb_p.h"
#include "../riscv/riscvoperand.h"

#ifndef ASMJIT_NO_COMPILER
  #include "../core/compiler.h"
#endif

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)


// riscv::FormatterInternal - Format Instruction
// =============================================

Error FormatterInternal::formatFeature(String& sb, uint32_t featureId) noexcept {
  static const char sFeatureString[] = {
    "None\0"
    "RV32I\0"
    "RV64I\0"
    "<Unknown>\0"
  };
  static const uint16_t sFeatureIndex[] = {
    0, 5,11,17
  };
  return sb.append(sFeatureString + sFeatureIndex[Support::min<uint32_t>(featureId, uint32_t(CpuFeatures::RISCV::kMaxValue) + 1)]);
}

// riscv::FormatterInternal - Format Register
// ==========================================

struct FormatElementData {
  char letter;
  uint8_t elementCount;
  uint8_t onlyIndex;
  uint8_t reserved;
};

ASMJIT_FAVOR_SIZE Error FormatterInternal::formatRegister(
  String& sb,
  FormatFlags flags,
  const BaseEmitter* emitter,
  Arch arch,
  RegType regType,
  uint32_t rId,
  uint32_t elementType,
  uint32_t elementIndex) noexcept {

  DebugUtils::unused(flags, arch);
  DebugUtils::unused(elementType, elementIndex);

  bool virtRegFormatted = false;

#ifndef ASMJIT_NO_COMPILER
  if (Operand::isVirtId(rId)) {
    if (emitter && emitter->isCompiler()) {
      const BaseCompiler* cc = static_cast<const BaseCompiler*>(emitter);
      if (cc->isVirtIdValid(rId)) {
        VirtReg* vReg = cc->virtRegById(rId);
        ASMJIT_ASSERT(vReg != nullptr);

        ASMJIT_PROPAGATE(Formatter::formatVirtRegName(sb, vReg));
        virtRegFormatted = true;
      }
    }
  }
#else
  DebugUtils::unused(emitter, flags);
#endif

  if (!virtRegFormatted) {
    char letter = '\0';
    switch (regType) {
      case RegType::kGp32:
        letter = 'w';
        break;
      case RegType::kGp64:
        letter = 'x';
        break;
      default:
        ASMJIT_PROPAGATE(sb.appendFormat("<Reg-%u>?%u", uint32_t(regType), rId));
        break;
    }
    if (letter)
      ASMJIT_PROPAGATE(sb.appendFormat("%c%u", letter, rId));
  }

  return kErrorOk;
}

// riscv::FormatterInternal - Format Operand
// =============================================

ASMJIT_FAVOR_SIZE Error FormatterInternal::formatOperand(
  String& sb,
  FormatFlags flags,
  const BaseEmitter* emitter,
  Arch arch,
  const Operand_& op) noexcept {

  if (op.isReg()) {
    const Reg& reg = op.as<Reg>();

    if (op.isVec()) {
      // rvv not support
    }
    uint32_t notUsed = 0xFFFFFFFFu;
    return formatRegister(sb, flags, emitter, arch, reg.regType(), reg.id(), notUsed, notUsed);
  }

  if (op.isMem()) {
    const riscv::Mem& m = op.as<riscv::Mem>();
    // RISC-V格式：offset(base)
    int64_t off = m.hasOffset() ? int64_t(m.offset()) : 0;
    uint32_t base = 10;

    if (Support::test(flags, FormatFlags::kHexOffsets) && uint64_t(off) > 9) {
      base = 16;
      ASMJIT_PROPAGATE(sb.append("0x"));
      ASMJIT_PROPAGATE(sb.appendUInt(uint64_t(off), base));
    } else {
      ASMJIT_PROPAGATE(sb.appendInt(off, base));
    }

    if (m.hasBase()) {
      ASMJIT_PROPAGATE(sb.append("("));
      if (m.hasBaseLabel()) {
        ASMJIT_PROPAGATE(Formatter::formatLabel(sb, flags, emitter, m.baseId()));
      } else {
        ASMJIT_PROPAGATE(formatRegister(sb, flags, emitter, arch, m.baseType(), m.baseId()));
      }
      ASMJIT_PROPAGATE(sb.append(")"));
    }
    return kErrorOk;
  }

  if (op.isImm()) {
    const Imm& i = op.as<Imm>();
    int64_t val = i.value();

    if (Support::test(flags, FormatFlags::kHexImms) && uint64_t(val) > 9) {
      ASMJIT_PROPAGATE(sb.append("0x"));
      return sb.appendUInt(uint64_t(val), 16);
    }
    else {
      return sb.appendInt(val, 10);
    }
  }

  if (op.isLabel()) {
    return Formatter::formatLabel(sb, flags, emitter, op.id());
  }

  if (op.isRegList()) {
    return sb.append("RISCV has Reg Lists????");
  }

  return sb.append("<None>");
};

// riscv::FormatterInternal - Format Instruction
// =============================================

ASMJIT_FAVOR_SIZE Error FormatterInternal::formatInstruction(
  String& sb,
  FormatFlags formatFlags,
  const BaseEmitter* emitter,
  Arch arch,
  const BaseInst& inst, const Operand_* operands, size_t opCount) noexcept {

  // Format instruction options and instruction mnemonic.
  InstId instId = inst.realId();
  if (instId != Inst::kIdNone && instId < Inst::kIdCount) {
    InstStringifyOptions stringifyOptions =
      Support::test(formatFlags, FormatFlags::kShowAliases)
        ? InstStringifyOptions::kAliases
        : InstStringifyOptions::kNone;
    ASMJIT_PROPAGATE(InstInternal::instIdToString(instId, stringifyOptions, sb));
  }
  else {
    ASMJIT_PROPAGATE(sb.appendFormat("[InstId=#%u]", unsigned(instId)));
  }

  for (uint32_t i = 0; i < opCount; i++) {
    const Operand_& op = operands[i];
    if (op.isNone()) {
      break;
    }

    ASMJIT_PROPAGATE(sb.append(i == 0 ? " " : ", "));
    ASMJIT_PROPAGATE(formatOperand(sb, formatFlags, emitter, arch, op));
  }

  return kErrorOk;
}

ASMJIT_END_SUB_NAMESPACE

#endif