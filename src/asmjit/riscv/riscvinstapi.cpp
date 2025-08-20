// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#include <cstdint>
#if !defined(ASMJIT_NO_COMPILER)

#include "../core/cpuinfo.h"
#include "../core/misc_p.h"
#include "../core/support_p.h"
#include "../riscv/riscvinstapi_p.h"
#include "../riscv/riscvinstdb_p.h"
#include "../riscv/riscvoperand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

namespace InstInternal {

// rv64::InstInternal - Text
// ========================

#ifndef ASMJIT_NO_TEXT
Error instIdToString(InstId instId, InstStringifyOptions options, String& output) noexcept {
  uint32_t realId = instId & uint32_t(InstIdParts::kRealId);
  if (ASMJIT_UNLIKELY(!Inst::isDefinedId(realId))) {
    return DebugUtils::errored(kErrorInvalidInstruction);
  }
  char nameData[6];
  for (uint32_t i = 0 ; i < 6 ; i++) {
    auto index = instId * 6 + i;
    nameData[i] = InstDB::_instNameStringTable[index];
  }
  return output.append(nameData, sizeof(nameData));
  return InstNameUtils::decode(InstDB::_instNameIndexTable[realId], options, InstDB::_instNameStringTable, output);
}

InstId stringToInstId(const char* s, size_t len) noexcept {
  if (ASMJIT_UNLIKELY(!s)) {
    return BaseInst::kIdNone;
  }

  if (len == SIZE_MAX) {
    len = strlen(s);
  }

  if (len == 0u || len > InstDB::instNameIndex.maxNameLength) {
    return BaseInst::kIdNone;
  }

  return InstNameUtils::findInstruction(s, len, InstDB::_instNameIndexTable, InstDB::_instNameStringTable, InstDB::instNameIndex);
}
#endif // !ASMJIT_NO_TEXT

// rv64::InstInternal - Validate
// ============================

#ifndef ASMJIT_NO_VALIDATION
ASMJIT_FAVOR_SIZE Error validate(const BaseInst& inst, const Operand_* operands, size_t opCount, ValidationFlags validationFlags) noexcept {
  // TODO: Implement validation for RISC-V instructions
  DebugUtils::unused(inst, operands, opCount, validationFlags);
  return kErrorOk;
}
#endif // !ASMJIT_NO_VALIDATION

// rv64::InstInternal - QueryRWInfo
// ===============================

#ifndef ASMJIT_NO_INTROSPECTION
#define kRISCVMaxOpCount 3

struct InstRWInfoData {
  uint8_t rwx[kRISCVMaxOpCount];
};

static const InstRWInfoData instRWInfoData[] = {
  #define R uint8_t(OpRWFlags::kRead)
  #define W uint8_t(OpRWFlags::kWrite)
  #define X uint8_t(OpRWFlags::kRW)   // RV64I don't have Atomic inst

  {{ R, R, R}}, // kRWI_RRR
  {{ W, R, R}}, // kRWI_WRR
  {{ R, W, R}}, // kRWI_RWR

  #undef R
  #undef W
  #undef X
};

Error queryRWInfo(const BaseInst& inst, const Operand_* operands, size_t opCount, InstRWInfo* out) noexcept {
  // Get the instruction data.
  uint32_t realId = inst.id() & uint32_t(InstIdParts::kRealId);

  if (ASMJIT_UNLIKELY(!Inst::isDefinedId(realId))) {
    return DebugUtils::errored(kErrorInvalidInstruction);
  }

  out->_instFlags = InstRWFlags::kNone;
  out->_opCount = uint8_t(opCount);
  out->_rmFeature = 0;
  out->_extraReg.reset();
  out->_readFlags = CpuRWFlags::kNone;
  out->_writeFlags = CpuRWFlags::kNone;

  const InstDB::InstInfo& instInfo = InstDB::_instInfoTable[realId];
  const InstRWInfoData& rwInfo = instRWInfoData[instInfo.rwInfoIndex()];

  for (uint32_t i = 0; i < opCount; i++) {
    OpRWInfo& op = out->_operands[i];
    const Operand_& srcOp = operands[i];

    if (!srcOp.isRegOrMem()) {
      op.reset();
      continue;
    }

    OpRWFlags rwFlags = (OpRWFlags)rwInfo.rwx[i];

    op._opFlags = rwFlags & ~(OpRWFlags::kZExt);
    op._physId = Reg::kIdBad;
    op._rmSize = 0;
    op._resetReserved();

    uint64_t rByteMask = op.isRead() ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;
    uint64_t wByteMask = op.isWrite() ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;

    op._readByteMask = rByteMask;
    op._writeByteMask = wByteMask;
    op._extendByteMask = 0;
    op._consecutiveLeadCount = 0;

    if (srcOp.isReg()) {
      // RISC-V doesn't have element access like ARM
      // So we don't need the element access handling code here
    }
    else {
      const Mem& memOp = srcOp.as<Mem>();

      if (memOp.hasBase()) {
        op.addOpFlags(OpRWFlags::kMemBaseRead);
        // RISC-V doesn't have pre/post increment addressing modes
      }

      // RISC-V doesn't support index registers in memory operands
      // So we don't need the index register handling code here
    }
  }

  return kErrorOk;
}
#endif // !ASMJIT_NO_INTROSPECTION

// rv64::InstInternal - QueryFeatures
// =================================

#ifndef ASMJIT_NO_INTROSPECTION
Error queryFeatures(const BaseInst& inst, const Operand_* operands, size_t opCount, CpuFeatures* out) noexcept {
  // TODO: Implement feature detection for RISC-V instructions
  DebugUtils::unused(inst, operands, opCount, out);
  return kErrorOk;
}
#endif // !ASMJIT_NO_INTROSPECTION

} // {InstInternal}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER