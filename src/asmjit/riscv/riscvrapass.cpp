// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information.
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/func.h"
#include "../core/ralocal_p.h"
#include "../core/formatter_p.h"
#include "../riscv/riscvrapass_p.h"
#include "../riscv/riscvinstapi_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

// riscv::RISCVRAPass - Helpers
// ==========================

[[maybe_unused]]
static inline uint64_t raImmMaskFromSize(uint32_t size) noexcept {
  ASMJIT_ASSERT(size > 0 && size < 256);
  static const uint64_t masks[] = {
    0x00000000000000FFu, //   1
    0x000000000000FFFFu, //   2
    0x00000000FFFFFFFFu, //   4
    0xFFFFFFFFFFFFFFFFu, //   8
    0x0000000000000000u, //  16
    0x0000000000000000u, //  32
    0x0000000000000000u, //  64
    0x0000000000000000u, // 128
    0x0000000000000000u  // 256
  };
  return masks[Support::ctz(size)];
}

[[nodiscard]]
static inline RATiedFlags raUseOutFlagsFromRWFlags(OpRWFlags rwFlags) noexcept {
  static constexpr RATiedFlags map[] = {
    RATiedFlags::kNone,
    RATiedFlags::kRead  | RATiedFlags::kUse, // kRead
    RATiedFlags::kWrite | RATiedFlags::kOut, // kWrite
    RATiedFlags::kRW    | RATiedFlags::kUse, // kRW
  };

  return map[uint32_t(rwFlags & OpRWFlags::kRW)];
}

[[nodiscard]]
static inline RATiedFlags raRegRwFlags(OpRWFlags flags) noexcept {
  return raUseOutFlagsFromRWFlags(flags);
}

[[nodiscard]]
static inline RATiedFlags raMemBaseRwFlags(OpRWFlags flags) noexcept {
  constexpr uint32_t shift = Support::ConstCTZ<uint32_t(OpRWFlags::kMemBaseRW)>::value;
  return raUseOutFlagsFromRWFlags(OpRWFlags(uint32_t(flags) >> shift) & OpRWFlags::kRW);
}

[[nodiscard]]
static inline RATiedFlags raMemIndexRwFlags(OpRWFlags flags) noexcept {
  constexpr uint32_t shift = Support::ConstCTZ<uint32_t(OpRWFlags::kMemIndexRW)>::value;
  return raUseOutFlagsFromRWFlags(OpRWFlags(uint32_t(flags) >> shift) & OpRWFlags::kRW);
}

// riscv::RACFGBuilder
// ===================

class RACFGBuilder : public RACFGBuilderT<RACFGBuilder> {
public:
  Arch _arch;
  inline RACFGBuilder(RISCVRAPass* pass) noexcept
    : RACFGBuilderT<RACFGBuilder>(pass),
      _arch(pass->cc()->arch()) {}

  [[nodiscard]]
  inline Compiler* cc() const noexcept { return static_cast<Compiler*>(_cc); }

  [[nodiscard]]
  Error onInst(InstNode* inst, InstControlFlow& controlType, RAInstBuilder& ib) noexcept;

  [[nodiscard]]
  Error onBeforeInvoke(InvokeNode* invokeNode) noexcept;

  [[nodiscard]]
  Error onInvoke(InvokeNode* invokeNode, RAInstBuilder& ib) noexcept;

  [[nodiscard]]
  Error moveImmToRegArg(InvokeNode* invokeNode, const FuncValue& arg, const Imm& imm_, Reg* out) noexcept;

  [[nodiscard]]
  Error moveImmToStackArg(InvokeNode* invokeNode, const FuncValue& arg, const Imm& imm_) noexcept;

  [[nodiscard]]
  Error moveRegToStackArg(InvokeNode* invokeNode, const FuncValue& arg, const Reg& reg) noexcept;

  [[nodiscard]]
  Error onBeforeRet(FuncRetNode* funcRet) noexcept;

  [[nodiscard]]
  Error onRet(FuncRetNode* funcRet, RAInstBuilder& ib) noexcept;
};

// riscv::RACFGBuilder - OnInst
// ==========================

// RISC-V instruction control flow type analysis
static InstControlFlow getControlFlowType(const InstNode* inst) noexcept {
  InstId realInstId = BaseInst::extractRealId(inst->id());
  switch (realInstId) {

    case Inst::kIdJal: // todo, 区分 kCall 和 kJump, kRet,
      return InstControlFlow::kCall;

    case Inst::kIdJalr:
      if (inst->isInvoke()) {
        return InstControlFlow::kCall;
      }
      return InstControlFlow::kReturn;
    case Inst::kIdBeq:
    case Inst::kIdBne:
    case Inst::kIdBlt:
    case Inst::kIdBge:
    case Inst::kIdBltu:
    case Inst::kIdBgeu:
      return InstControlFlow::kBranch;

    default:
      return InstControlFlow::kRegular;
  }
}

Error RACFGBuilder::onInst(InstNode* inst, InstControlFlow& controlType, RAInstBuilder& ib) noexcept {
  InstRWInfo rwInfo;

  if (Inst::isDefinedId(inst->realId())) {
    InstId instId = inst->id();
    uint32_t opCount = inst->opCount();
    const Operand* opArray = inst->operands();
    ASMJIT_PROPAGATE(InstInternal::queryRWInfo(inst->baseInst(), opArray, opCount, &rwInfo));

    const InstDB::InstInfo& instInfo = InstDB::infoById(instId);
    uint32_t singleRegOps = 0;

    ib.addInstRWFlags(rwInfo.instFlags());

    if (opCount) {
      for (uint32_t i = 0; i < opCount; i++) {
        const Operand& op = opArray[i];
        const OpRWInfo& opRwInfo = rwInfo.operand(i);

        if (op.isReg()) {
          // Register Operand
          // ----------------
          const Reg& reg = op.as<Reg>();

          RATiedFlags flags = raRegRwFlags(opRwInfo.opFlags());
          uint32_t vIndex = Operand::virtIdToIndex(reg.id());

          if (vIndex < Operand::kVirtIdCount) {
            RAWorkReg* workReg;
            ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

            // Use RW instead of Write in case that not the whole register is overwritten. This is important for
            // liveness as we cannot kill a register that will be used.
            if ((flags & RATiedFlags::kRW) == RATiedFlags::kWrite) {
              if (workReg->regByteMask() & ~(opRwInfo.writeByteMask() | opRwInfo.extendByteMask())) {
                // Not write-only operation.
                flags = (flags & ~RATiedFlags::kOut) | (RATiedFlags::kRead | RATiedFlags::kUse);
              }
            }

            RegGroup group = workReg->group();

            RegMask useRegs = _pass->_availableRegs[group];
            RegMask outRegs = useRegs;

            uint32_t useId = Reg::kIdBad;
            uint32_t outId = Reg::kIdBad;

            uint32_t useRewriteMask = 0;
            uint32_t outRewriteMask = 0;

            // RISC-V doesn't support consecutive registers in the same way as ARM
            // So we don't need the consecutive register handling code here

            if (Support::test(flags, RATiedFlags::kUse)) {
              useRewriteMask = Support::bitMask(inst->_getRewriteIndex(&reg._baseId));
              if (opRwInfo.hasOpFlag(OpRWFlags::kRegPhysId)) {
                useId = opRwInfo.physId();
                flags |= RATiedFlags::kUseFixed;
              }
            }
            else {
              outRewriteMask = Support::bitMask(inst->_getRewriteIndex(&reg._baseId));
              if (opRwInfo.hasOpFlag(OpRWFlags::kRegPhysId)) {
                outId = opRwInfo.physId();
                flags |= RATiedFlags::kOutFixed;
              }
            }

            // RISC-V doesn't have the same element access constraints as ARM
            // So we don't need the element access handling code here

            ASMJIT_PROPAGATE(ib.add(workReg, flags, useRegs, useId, useRewriteMask, outRegs, outId, outRewriteMask, opRwInfo.rmSize()));
            if (singleRegOps == i) {
              singleRegOps++;
            }
          }
        }
        else if (op.isMem()) {
          // Memory Operand
          // --------------
          const Mem& mem = op.as<Mem>();

          if (mem.isRegHome()) {
            RAWorkReg* workReg;
            ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::virtIdToIndex(mem.baseId()), &workReg));
            if (ASMJIT_UNLIKELY(!_pass->getOrCreateStackSlot(workReg))) {
              return DebugUtils::errored(kErrorOutOfMemory);
            }
          }
          else if (mem.hasBaseReg()) {
            if (instId == Inst::kIdAdr && i == 1) {
              // Adr dst, mem, mem should not be consider to use
              continue;
            }  
            uint32_t vIndex = Operand::virtIdToIndex(mem.baseId());
            if (vIndex < Operand::kVirtIdCount) {
              RAWorkReg* workReg;
              ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

              RATiedFlags flags = raMemBaseRwFlags(opRwInfo.opFlags());
              RegGroup group = workReg->group();
              RegMask allocable = _pass->_availableRegs[group];

              // Base registers have never fixed id on RISC-V.
              const uint32_t useId = Reg::kIdBad;
              const uint32_t outId = Reg::kIdBad;

              uint32_t useRewriteMask = 0;
              uint32_t outRewriteMask = 0;

              if (Support::test(flags, RATiedFlags::kUse)) {
                useRewriteMask = Support::bitMask(inst->_getRewriteIndex(&mem._baseId));
              }
              else {
                outRewriteMask = Support::bitMask(inst->_getRewriteIndex(&mem._baseId));
              }

              ASMJIT_PROPAGATE(ib.add(workReg, flags, allocable, useId, useRewriteMask, allocable, outId, outRewriteMask));
            }
          }

          // RISC-V doesn't support index registers in memory operands
          // So we don't need the index register handling code here
        }
      }
    }

    controlType = getControlFlowType(inst);
  }

  return kErrorOk;
}


// riscv::RACFGBuilder - OnInvoke
// ============================

Error RACFGBuilder::onBeforeInvoke(InvokeNode* invokeNode) noexcept {
  const FuncDetail& fd = invokeNode->detail();
  uint32_t argCount = invokeNode->argCount();

  cc()->_setCursor(invokeNode->prev());

  // (valueIndex = 0)
  for (uint32_t argIndex = 0; argIndex < argCount; argIndex++) {
    const FuncValue& arg = fd.arg(argIndex, 0);  // 直接使用第一个值
    const Operand& op = invokeNode->arg(argIndex, 0);

    if (op.isNone()) {
      continue;
    }

    if (op.isReg()) {
      const Reg& reg = op.as<Reg>();
      RAWorkReg* workReg;
      ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::virtIdToIndex(reg.id()), &workReg));

      if (arg.isReg()) {
        RegGroup regGroup = workReg->group();
        RegGroup argGroup = RegUtils::groupOf(arg.regType());

        if (regGroup != argGroup) {
          return DebugUtils::errored(kErrorInvalidAssignment);
        }
      }
      else {
        ASMJIT_PROPAGATE(moveRegToStackArg(invokeNode, arg, reg));
      }
    }
    else if (op.isImm()) {
      if (arg.isReg()) {
        Reg reg;
        ASMJIT_PROPAGATE(moveImmToRegArg(invokeNode, arg, op.as<Imm>(), &reg));
        invokeNode->_args[argIndex][0] = reg;  // 只设置第一个值
      }
      else {
        ASMJIT_PROPAGATE(moveImmToStackArg(invokeNode, arg, op.as<Imm>()));
      }
    }
  }

  cc()->_setCursor(invokeNode);

  // valueIndex = 0
  if (fd.hasRet()) {
    const FuncValue& ret = fd.ret(0);  // 只处理第一个返回值
    const Operand& op = invokeNode->ret(0);
    
    if (op.isReg() && ret.isReg()) {
      const Reg& reg = op.as<Reg>();
      RAWorkReg* workReg;
      ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::virtIdToIndex(reg.id()), &workReg));

      RegGroup regGroup = workReg->group();
      RegGroup retGroup = RegUtils::groupOf(ret.regType());

      if (regGroup != retGroup) {
        return DebugUtils::errored(kErrorInvalidAssignment);
      }
    }
  }

  // This block has function call(s).
  _curBlock->addFlags(RABlockFlags::kHasFuncCalls);
  _pass->func()->frame().addAttributes(FuncAttributes::kHasFuncCalls);
  _pass->func()->frame().updateCallStackSize(fd.argStackSize());

  return kErrorOk;
}

Error RACFGBuilder::onInvoke(InvokeNode* invokeNode, RAInstBuilder& ib) noexcept {
  printf("InvokeNode opCount: %u\n", invokeNode->opCount());
  for (uint32_t i = 0; i < invokeNode->opCount(); i++) {
    printf("  op[%u]: type=%u\n", i, invokeNode->op(i).opType());
  }
  // hack
  auto target = invokeNode->op(0);
  invokeNode->setOp(0, regs::ra);
  invokeNode->setOp(1, target);

  uint32_t argCount = invokeNode->argCount(); // func arg num
  const FuncDetail& fd = invokeNode->detail();

  // valueIndex = 1
  for (uint32_t argIndex = 0; argIndex < argCount; argIndex++) {
    const FuncValue& arg = fd.arg(argIndex, 0);
    const Operand& op = invokeNode->arg(argIndex, 0);

    if (op.isNone() || !arg.isInitialized()) {
      continue;
    }

    if (op.isReg()) {
      const Reg& reg = op.as<Reg>();
      RAWorkReg* workReg;
      ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::virtIdToIndex(reg.id()), &workReg));

      if (arg.isIndirect()) {
        RegGroup regGroup = workReg->group();
        if (regGroup != RegGroup::kGp) {
          return DebugUtils::errored(kErrorInvalidState);
        }
        ASMJIT_PROPAGATE(ib.addCallArg(workReg, arg.regId()));
      }
      else if (arg.isReg()) {
        RegGroup regGroup = workReg->group();
        RegGroup argGroup = RegUtils::groupOf(arg.regType());

        if (regGroup == argGroup) {
          ASMJIT_PROPAGATE(ib.addCallArg(workReg, arg.regId()));
        }
      }
    }
  }

  // valueIndex = 1
  if (fd.hasRet()) {
    const FuncValue& ret = fd.ret(0);
    const Operand& op = invokeNode->ret(0);
    
    if (op.isReg() && ret.isReg()) {
      const Reg& reg = op.as<Reg>();
      RAWorkReg* workReg;
      ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::virtIdToIndex(reg.id()), &workReg));

      RegGroup regGroup = workReg->group();
      RegGroup retGroup = RegUtils::groupOf(ret.regType());

      if (regGroup == retGroup) {
        ASMJIT_PROPAGATE(ib.addCallRet(workReg, ret.regId()));
      }
    }
  }

  ib._clobbered[0] = Support::lsbMask<RegMask>(_pass->_physRegCount[RegGroup(0)]) & ~fd.preservedRegs(RegGroup(0));
  ib._clobbered[1] = Support::lsbMask<RegMask>(_pass->_physRegCount[RegGroup(1)]) & ~fd.preservedRegs(RegGroup(1));
  ib._clobbered[2] = Support::lsbMask<RegMask>(_pass->_physRegCount[RegGroup(2)]) & ~fd.preservedRegs(RegGroup(2));
  ib._clobbered[3] = Support::lsbMask<RegMask>(_pass->_physRegCount[RegGroup(3)]) & ~fd.preservedRegs(RegGroup(3));

  return kErrorOk;
}

// rv64::RACFGBuilder - MoveImmToRegArg
// ===================================

Error RACFGBuilder::moveImmToRegArg(InvokeNode* invokeNode, const FuncValue& arg, const Imm& imm_, Reg* out) noexcept {
  DebugUtils::unused(invokeNode);
  ASMJIT_ASSERT(arg.isReg());

  Imm imm(imm_);
  TypeId typeId = TypeId::kVoid;

  switch (arg.typeId()) {
    case TypeId::kInt8  : typeId = TypeId::kUInt64; imm.signExtend8Bits(); break;
    case TypeId::kUInt8 : typeId = TypeId::kUInt64; imm.zeroExtend8Bits(); break;
    case TypeId::kInt16 : typeId = TypeId::kUInt64; imm.signExtend16Bits(); break;
    case TypeId::kUInt16: typeId = TypeId::kUInt64; imm.zeroExtend16Bits(); break;
    case TypeId::kInt32 : typeId = TypeId::kUInt64; imm.signExtend32Bits(); break;
    case TypeId::kUInt32: typeId = TypeId::kUInt64; imm.zeroExtend32Bits(); break;
    case TypeId::kInt64 : typeId = TypeId::kUInt64; break;
    case TypeId::kUInt64: typeId = TypeId::kUInt64; break;

    default:
      return DebugUtils::errored(kErrorInvalidAssignment);
  }

  ASMJIT_PROPAGATE(cc()->_newReg(out, typeId, nullptr));
  cc()->virtRegById(out->id())->setWeight(BaseRAPass::kCallArgWeight);
  return cc()->li(out->as<Gp>(), imm);
}

// rv64::RACFGBuilder - MoveImmToStackArg
// =====================================

Error RACFGBuilder::moveImmToStackArg(InvokeNode* invokeNode, const FuncValue& arg, const Imm& imm_) noexcept {
  Reg reg;

  ASMJIT_PROPAGATE(moveImmToRegArg(invokeNode, arg, imm_, &reg));
  ASMJIT_PROPAGATE(moveRegToStackArg(invokeNode, arg, reg));

  return kErrorOk;
}

// rv64::RACFGBuilder - MoveRegToStackArg
// =====================================

Error RACFGBuilder::moveRegToStackArg(InvokeNode* invokeNode, const FuncValue& arg, const Reg& reg) noexcept {
  DebugUtils::unused(invokeNode);
  Mem stackPtr = Mem::ptr(_pass->_sp.as<Gp>(), arg.stackOffset());

  if (reg.isGp()) {
    return cc()->sd(reg.as<Gp>(), stackPtr);
  }

  // RISC-V RV64I doesn't have vector registers
  // So we don't need the vector register handling code here

  return DebugUtils::errored(kErrorInvalidState);
}

// rv64::RACFGBuilder - OnRet
// =========================

Error RACFGBuilder::onBeforeRet(FuncRetNode* funcRet) noexcept {
  DebugUtils::unused(funcRet);
  return kErrorOk;
}

Error RACFGBuilder::onRet(FuncRetNode* funcRet, RAInstBuilder& ib) noexcept {
  const FuncDetail& funcDetail = _pass->func()->detail();
  const Operand* opArray = funcRet->operands();
  uint32_t opCount = funcRet->opCount();

  for (uint32_t i = 0; i < opCount; i++) {
    const Operand& op = opArray[i];
    if (op.isNone()) {
      continue;
    }

    const FuncValue& ret = funcDetail.ret(i);
    if (ASMJIT_UNLIKELY(!ret.isReg())) {
      return DebugUtils::errored(kErrorInvalidAssignment);
    }

    if (op.isReg()) {
      // Register return value.
      const Reg& reg = op.as<Reg>();
      uint32_t vIndex = Operand::virtIdToIndex(reg.id());

      if (vIndex < Operand::kVirtIdCount) {
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

        RegGroup group = workReg->group();
        RegMask allocable = _pass->_availableRegs[group];
        ASMJIT_PROPAGATE(ib.add(workReg, RATiedFlags::kUse | RATiedFlags::kRead, allocable, ret.regId(), 0, 0, Reg::kIdBad, 0));
      }
    }
    else {
      return DebugUtils::errored(kErrorInvalidAssignment);
    }
  }

  return kErrorOk;
}


// riscv::RISCVRAPass - Construction & Destruction
// ==============================================

RISCVRAPass::RISCVRAPass() noexcept : BaseRAPass() {
  _iEmitHelper = &_emitHelper;
}

RISCVRAPass::~RISCVRAPass() noexcept {}

// riscv::RISCVRAPass - Overrides
// ==============================

void RISCVRAPass::onInit() noexcept {
  Arch arch = cc()->arch();

  _emitHelper._emitter = _cb;

  _archTraits = &ArchTraits::byArch(arch);
  _physRegCount.set(RegGroup::kGp, 32);
  _physRegCount.set(RegGroup::kVec, 0); // RV64I doesn't have vector registers
  _physRegCount.set(RegGroup::kMask, 0);
  _physRegCount.set(RegGroup::kExtraVirt3, 0);
  _buildPhysIndex();

  _availableRegs[RegGroup::kGp] = Support::lsbMask<uint32_t>(_physRegCount.get(RegGroup::kGp));
  _availableRegs[RegGroup::kVec] = Support::lsbMask<uint32_t>(_physRegCount.get(RegGroup::kVec));
  _availableRegs[RegGroup::kMask] = Support::lsbMask<uint32_t>(_physRegCount.get(RegGroup::kMask));
  _availableRegs[RegGroup::kExtraVirt3] = Support::lsbMask<uint32_t>(_physRegCount.get(RegGroup::kExtraVirt3));

  // Use t0 and t1 as scratch registers
  _scratchRegIndexes[0] = uint8_t(5); // t0 (x5)
  _scratchRegIndexes[1] = uint8_t(6); // t1 (x6)

  const FuncFrame& frame = _func->frame();

  // Make unavailable all registers that are special and cannot be used in general.
  bool hasFP = frame.hasPreservedFP();

  // x0 is hardwired to zero, always unavailable
  makeUnavailable(RegGroup::kGp, 0);

  // If we have a frame pointer, make it unavailable
  makeUnavailable(RegGroup::kGp, 8); // s0/fp (x8)
  
  // Make ra (return address) unavailable
  makeUnavailable(RegGroup::kGp, 1); // ra (x1)

  // Make sp unavailable
  makeUnavailable(RegGroup::kGp, 2); // sp (x2)

  // Make gp (global pointer) unavailable
  makeUnavailable(RegGroup::kGp, 3); // gp (x3)

  // Make tp (thread pointer) unavailable
  makeUnavailable(RegGroup::kGp, 4); // tp (x4)
  
  // Make unavailable any other registers specified by the frame
  makeUnavailable(frame._unavailableRegs);

  _sp = regs::sp;
  _fp = regs::fp; // In RISC-V, s0 (x8) is also used as frame pointer
}

void RISCVRAPass::onDone() noexcept {}

// rv64::RISCVRAPass - BuildCFG
// =========================

Error RISCVRAPass::buildCFG() noexcept {
  return RACFGBuilder(this).run();
}

// rv64::RISCVRAPass - Rewrite
// ========================


ASMJIT_FAVOR_SPEED Error RISCVRAPass::_rewrite(BaseNode* first, BaseNode* stop) noexcept {
  uint32_t virtCount = cc()->_vRegArray.size();

  BaseNode* node = first;
  while (node != stop) {
    BaseNode* next = node->next();
    if (node->isInst()) {
      InstNode* inst = node->as<InstNode>();
      RAInst* raInst = node->passData<RAInst>();

      Operand* operands = inst->operands();
      uint32_t opCount = inst->opCount();

      uint32_t i;

      // Rewrite virtual registers into physical registers.
      if (raInst) {
        // If the instruction contains pass data (raInst) then it was a subject
        // for register allocation and must be rewritten to use physical regs.
        RATiedReg* tiedRegs = raInst->tiedRegs();
        uint32_t tiedCount = raInst->tiedCount();

        for (i = 0; i < tiedCount; i++) {
          RATiedReg* tiedReg = &tiedRegs[i];

          Support::BitWordIterator<uint32_t> useIt(tiedReg->useRewriteMask());
          uint32_t useId = tiedReg->useId();

          while (useIt.hasNext()) {
            inst->_rewriteIdAtIndex(useIt.next(), useId);
          }

          Support::BitWordIterator<uint32_t> outIt(tiedReg->outRewriteMask());
          uint32_t outId = tiedReg->outId();

          while (outIt.hasNext()) {
            inst->_rewriteIdAtIndex(outIt.next(), outId);
          }
        }

        // This data is allocated by Zone passed to `runOnFunction()`, which
        // will be reset after the RA pass finishes. So reset this data to
        // prevent having a dead pointer after the RA pass is complete.
        node->resetPassData();

        if (ASMJIT_UNLIKELY(node->type() != NodeType::kInst)) {
          // FuncRet terminates the flow, it must either be removed if the exit
          // label is next to it (optimization) or patched to an architecture
          // dependent jump instruction that jumps to the function's exit before
          // the epilog.
          if (node->type() == NodeType::kFuncRet) {
            RABlock* block = raInst->block();
            if (!isNextTo(node, _func->exitNode())) {
              cc()->_setCursor(node->prev());
              ASMJIT_PROPAGATE(emitJump(_func->exitNode()->label()));
            }

            BaseNode* prev = node->prev();
            cc()->removeNode(node);
            block->setLast(prev);
          }
        }
      }

      // Rewrite stack slot addresses.
      for (i = 0; i < opCount; i++) {
        Operand& op = operands[i];
        if (op.isMem()) {
          BaseMem& mem = op.as<BaseMem>();
          if (mem.isRegHome()) {
            uint32_t virtIndex = Operand::virtIdToIndex(mem.baseId());
            if (ASMJIT_UNLIKELY(virtIndex >= virtCount)) {
              return DebugUtils::errored(kErrorInvalidVirtId);
            }

            VirtReg* virtReg = cc()->virtRegByIndex(virtIndex);
            RAWorkReg* workReg = virtReg->workReg();
            ASMJIT_ASSERT(workReg != nullptr);

            RAStackSlot* slot = workReg->stackSlot();
            int32_t offset = slot->offset();

            mem._setBase(_sp.regType(), slot->baseRegId());
            mem.clearRegHome();
            mem.addOffsetLo32(offset);
          }
        }
      }

      // Handle RISC-V specific instructions
      // Rewrite `loadAddressOf(Gp, Mem)` construct.
      if (inst->realId() == Inst::kIdAdr && inst->opCount() == 2 && inst->op(1).isMem()) {
        printf("hack Adr!!!!!!\n");
        BaseMem mem = inst->op(1).as<BaseMem>();
        int64_t offset = mem.offset();

        if (!mem.hasBaseOrIndex()) {
          if (offset >= -2048 && offset <= 2047) {
            inst->setId(Inst::kIdAddi);
            inst->setOpCount(3);
            inst->setOp(1, regs::zero);
            inst->setOp(2, Imm(offset));
          } else {
            printf("not support for now!");
            // [TODO] 可以在 Assembler 级别加一个 Li 伪指令
            return DebugUtils::errored(kErrorInvalidState);
          }
        }
        else {
          if (mem.hasIndex()) {
            return DebugUtils::errored(kErrorInvalidAddressIndex);
          }

          // Gp dst = Gp::make_x(inst->op(0).as<Gp>().id());
          Gp base = Gp::make_x(mem.baseId());

          inst->setId(Inst::kIdAddi);
          inst->setOpCount(3);
          inst->setOp(1, base);
          inst->setOp(2, Imm(offset));

          // Use two operations if the offset cannot be encoded with ADD/SUB.
          if (offset < -2048 || offset > 2047) {
            printf("not support for now!");
            // [TODO] 先给 inst->prev() 发射一个 Add 再几条 Addi
            return DebugUtils::errored(kErrorInvalidState);
          }
        }
      }

      // Hack InvokeNode's Op for JALR
      if (inst->realId() == Inst::kIdJalr && inst->opCount() == 1 && inst->op(0).isReg()) {
        auto target = inst->op(0).as<Reg>(); // phy reg id
        inst->operands()[0] = regs::ra;
        inst->operands()[1] = target;
      }
    }

    node = next;
  }

  return kErrorOk;
}
// rv64::RISCVRAPass - Prolog & Epilog
// ================================

Error RISCVRAPass::updateStackFrame() noexcept {
  if (_func->frame().hasFuncCalls()) {
    // In RISC-V, the return address is stored in ra (x1)
    _func->frame().addDirtyRegs(RegGroup::kGp, Support::bitMask(1)); // ra (x1)
  }

  return BaseRAPass::updateStackFrame();
}

// rv64::RISCVRAPass - OnEmit
// =======================

Error RISCVRAPass::emitMove(uint32_t workId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept {
  RAWorkReg* wReg = workRegById(workId);
  Reg dst(wReg->signature(), dstPhysId);
  Reg src(wReg->signature(), srcPhysId);

  const char* comment = nullptr;

#ifndef ASMJIT_NO_LOGGING
  if (hasDiagnosticOption(DiagnosticOptions::kRAAnnotate)) {
    _tmpString.clear();
    Formatter::formatVirtRegNameWithPrefix(_tmpString, "<MOVE> ", 7u, wReg->virtReg());
    comment = _tmpString.data();
  }
#endif

  return _emitHelper.emitRegMove(dst, src, wReg->typeId(), comment);
}

Error RISCVRAPass::emitSwap(uint32_t aWorkId, uint32_t aPhysId, uint32_t bWorkId, uint32_t bPhysId) noexcept {
  // RISC-V doesn't have a direct swap instruction, so we need to use a temporary register
  // This is similar to the AArch64 implementation which also doesn't support direct swaps
  DebugUtils::unused(aWorkId, aPhysId, bWorkId, bPhysId);
  return DebugUtils::errored(kErrorInvalidState);
}

Error RISCVRAPass::emitLoad(uint32_t workId, uint32_t dstPhysId) noexcept {
  RAWorkReg* wReg = workRegById(workId);
  Reg dstReg(wReg->signature(), dstPhysId);
  BaseMem srcMem(workRegAsMem(wReg));

  const char* comment = nullptr;

#ifndef ASMJIT_NO_LOGGING
  if (hasDiagnosticOption(DiagnosticOptions::kRAAnnotate)) {
    _tmpString.clear();
    Formatter::formatVirtRegNameWithPrefix(_tmpString, "<LOAD> ", 7u, wReg->virtReg());
    comment = _tmpString.data();
  }
#endif

  return _emitHelper.emitRegMove(dstReg, srcMem, wReg->typeId(), comment);
}

Error RISCVRAPass::emitSave(uint32_t workId, uint32_t srcPhysId) noexcept {
  RAWorkReg* wReg = workRegById(workId);
  BaseMem dstMem(workRegAsMem(wReg));
  Reg srcReg(wReg->signature(), srcPhysId);

  const char* comment = nullptr;

#ifndef ASMJIT_NO_LOGGING
  if (hasDiagnosticOption(DiagnosticOptions::kRAAnnotate)) {
    _tmpString.clear();
    Formatter::formatVirtRegNameWithPrefix(_tmpString, "<SAVE> ", 7u, wReg->virtReg());
    comment = _tmpString.data();
  }
#endif

  return _emitHelper.emitRegMove(dstMem, srcReg, wReg->typeId(), comment);
}

Error RISCVRAPass::emitJump(const Label& label) noexcept {
  return cc()->j(label);
}

Error RISCVRAPass::emitPreCall(InvokeNode* invokeNode) noexcept {
  DebugUtils::unused(invokeNode);
  return kErrorOk;
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER