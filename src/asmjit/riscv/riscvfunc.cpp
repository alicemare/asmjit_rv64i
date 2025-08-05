// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_RISCV)

#include "../riscv/riscvfunc_p.h"
#include "../riscv/riscvoperand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

namespace FuncInternal {

static inline bool shouldTreatAsCDecl(CallConvId ccId) noexcept {
  return ccId == CallConvId::kCDecl ||
         ccId == CallConvId::kStdCall ||
         ccId == CallConvId::kFastCall ||
         ccId == CallConvId::kVectorCall ||
         ccId == CallConvId::kThisCall ||
         ccId == CallConvId::kRegParm1 ||
         ccId == CallConvId::kRegParm2 ||
         ccId == CallConvId::kRegParm3;
}

ASMJIT_FAVOR_SIZE Error initCallConv(CallConv& cc, CallConvId ccId, const Environment& environment) noexcept {
  cc.setArch(environment.arch());
  cc.setStrategy(CallConvStrategy::kDefault);

  // RISC-V 64-bit register save/restore configuration
  cc.setSaveRestoreRegSize(RegGroup::kGp, 8);
  cc.setSaveRestoreAlignment(RegGroup::kGp, 16);
  cc.setNaturalStackAlignment(16);

  if (shouldTreatAsCDecl(ccId)) {
    // RISC-V standard calling convention
    cc.setId(CallConvId::kCDecl);
    
    // Set argument passing order: a0-a7 (x10-x17)
    cc.setPassedOrder(RegGroup::kGp, 10, 11, 12, 13, 14, 15, 16, 17);
    
    // Set preserved (callee-saved) registers according to RISC-V ABI:
    // s0-s11 (x8-x9, x18-x27), ra (x1), sp (x2)
    // Note: sp is implicitly preserved, ra is handled separately
    cc.setPreservedRegs(RegGroup::kGp, Support::bitMask(1, 8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27));
  }
  else {
    cc.setId(ccId);
    
    // For non-standard calling conventions, use the same register layout
    cc.setPassedOrder(RegGroup::kGp, 10, 11, 12, 13, 14, 15, 16, 17);
    
    // More conservative preservation for non-standard conventions
    cc.setPreservedRegs(RegGroup::kGp, Support::bitMask(1, 8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31));
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error initFuncDetail(FuncDetail& func, const FuncSignature& signature) noexcept {
  DebugUtils::unused(signature);

  const CallConv& cc = func.callConv();
  uint32_t stackOffset = 0;

  uint32_t i;
  uint32_t argCount = func.argCount();

  // RISC-V standard specifies 8-byte minimum stack slot size
  uint32_t minStackArgSize = 8u;

  // Handle return values
  if (func.hasRet()) {
    for (uint32_t valueIndex = 0; valueIndex < Globals::kMaxValuePack; valueIndex++) {
      TypeId typeId = func._rets[valueIndex].typeId();

      // Terminate at the first void type (end of the pack)
      if (typeId == TypeId::kVoid)
        break;

      switch (typeId) {
        case TypeId::kInt8:
        case TypeId::kInt16:
        case TypeId::kInt32: {
          // Return values in a0, a1 (x10, x11) - sign-extended to 64-bit
          func._rets[valueIndex].initReg(RegType::kGp64, 10 + valueIndex, TypeId::kInt64);
          break;
        }

        case TypeId::kUInt8:
        case TypeId::kUInt16:
        case TypeId::kUInt32: {
          // Return values in a0, a1 (x10, x11) - zero-extended to 64-bit
          func._rets[valueIndex].initReg(RegType::kGp64, 10 + valueIndex, TypeId::kUInt64);
          break;
        }

        case TypeId::kInt64:
        case TypeId::kUInt64: {
          // Return values in a0, a1 (x10, x11)
          func._rets[valueIndex].initReg(RegType::kGp64, 10 + valueIndex, typeId);
          break;
        }

        default: {
          // For RV64I, we only support integer types
          return DebugUtils::errored(kErrorInvalidRegType);
        }
      }
    }
  }

  // Handle function arguments
  switch (cc.strategy()) {
    case CallConvStrategy::kDefault: {
      uint32_t gpPos = 0;

      for (i = 0; i < argCount; i++) {
        FuncValue& arg = func._args[i][0];
        TypeId typeId = arg.typeId();

        if (TypeUtils::isInt(typeId)) {
          uint32_t regId = Reg::kIdBad;

          // Try to assign to argument registers a0-a7 (x10-x17)
          if (gpPos < CallConv::kMaxRegArgsPerGroup) {
            regId = cc._passedOrder[RegGroup::kGp].id[gpPos];
          }

          if (regId != Reg::kIdBad) {
            // All integer arguments are passed in 64-bit registers
            RegType regType = RegType::kGp64;
            arg.assignRegData(regType, regId);
            func.addUsedRegs(RegGroup::kGp, Support::bitMask(regId));
            gpPos++;
          }
          else {
            // Argument passed on stack
            uint32_t size = Support::max<uint32_t>(TypeUtils::sizeOf(typeId), minStackArgSize);
            
            // Align stack offset based on argument size
            if (size >= 8) {
              stackOffset = Support::alignUp(stackOffset, 8);
            }
            arg.assignStackOffset(int32_t(stackOffset));
            stackOffset += size;
          }
          continue;
        }

        // For RV64I, only integer types are supported
        if (TypeUtils::isFloat(typeId) || TypeUtils::isVec(typeId)) {
          return DebugUtils::errored(kErrorInvalidRegType);
        }
      }
      break;
    }

    default:
      return DebugUtils::errored(kErrorInvalidState);
  }

  // Align final stack size to 16 bytes (RISC-V ABI requirement)
  func._argStackSize = Support::alignUp(stackOffset, 16u);
  return kErrorOk;
}

} // {FuncInternal}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_RISCV64