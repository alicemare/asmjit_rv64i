// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCVINSTDB_H_P_INCLUDED
#define ASMJIT_RISCV_RISCVINSTDB_H_P_INCLUDED

#include "../core/codeholder.h"
#include "../core/instdb_p.h"
#include "../riscv/riscvinstdb.h"
#include "../riscv/riscvoperand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv)

//! \cond INTERNAL
//! \addtogroup asmjit_riscv
//! \{

namespace InstDB {

// rv64::InstDB - Constants Used by Instructions
// ============================================

// GP register types supported by base instructions.
static constexpr uint32_t kW = 0x1;  // 32-bit register
static constexpr uint32_t kX = 0x2;  // 64-bit register
static constexpr uint32_t kWX = 0x3; // Both 32-bit and 64-bit registers

// Special register IDs
static constexpr uint32_t kZR = 0;   // Zero register (x0)
static constexpr uint32_t kRA = 1;   // Return address register (x1)
static constexpr uint32_t kSP = 2;   // Stack pointer register (x2)
static constexpr uint32_t kFP = 8;   // Frame pointer register (x8)

// rv64::InstDB - RWInfo
// ====================

enum RWInfoType : uint32_t {
  kRWI_R,      // Read
  kRWI_RW,     // Read-Write
  kRWI_RX,     // Read-ReadWrite
  kRWI_RRW,    // Read-Read-Write
  kRWI_RWX,    // Read-Write-ReadWrite
  kRWI_W,      // Write
  kRWI_WRW,    // Write-Read-Write
  kRWI_WRX,    // Write-Read-ReadWrite
  kRWI_WRRW,   // Write-Read-Read-Write
  kRWI_WRRX,   // Write-Read-Read-ReadWrite
  kRWI_WW,     // Write-Write
  kRWI_X,      // ReadWrite
  kRWI_XRX,    // ReadWrite-Read-ReadWrite
  kRWI_XXRRX,  // ReadWrite-ReadWrite-Read-Read-ReadWrite

  kRWI_LDn,    // Load instruction
  kRWI_STn,    // Store instruction

  kRWI_SpecialStart = kRWI_LDn
};

// rv64::InstDB - GpType
// ====================

enum GpType : uint8_t {
  kGp_W,       // 32-bit GP register
  kGp_X,       // 64-bit GP register
  kGp_X_SP     // 64-bit GP register, including SP
};

// rv64::InstDB - OPSig
// ===================

enum kOpSignature : uint32_t {
  kOp_GpW = RegTraits<RegType::kGp32>::kSignature,
  kOp_GpX = RegTraits<RegType::kGp64>::kSignature
};

// rv64::InstDB - EncodingId
// ========================

// ${EncodingId:Begin}
// ------------------- Automatically generated, do not edit -------------------
enum EncodingId : uint32_t {
  kEncodingNone = 0,
  kEncodingBaseR,      // R-Type: rd, rs1, rs2
  kEncodingBaseI,      // I-Type: rd, rs1, imm
  kEncodingBaseS,      // S-Type: rs1, rs2, imm
  kEncodingBaseB,      // B-Type: rs1, rs2, imm
  kEncodingBaseU,      // U-Type: rd, imm
  kEncodingBaseJ,      // J-Type: rd, imm
  kEncodingBaseLui,    // LUI: rd, imm
  kEncodingBaseAuipc,  // AUIPC: rd, imm
  kEncodingBaseJal,    // JAL: rd, imm
  kEncodingBaseJalr,   // JALR: rd, rs1, imm
  kEncodingBaseBranch, // Branch: rs1, rs2, imm
  kEncodingBaseLoad,   // Load: rd, rs1, imm
  kEncodingBaseStore,  // Store: rs1, rs2, imm
  kEncodingBaseOpImm,  // OP-IMM: rd, rs1, imm
  kEncodingBaseOp,     // OP: rd, rs1, rs2
  kEncodingBaseFence,  // FENCE: pred, succ
  kEncodingBaseSystem  // SYSTEM: rd, csr, rs1/zimm
};
// ----------------------------------------------------------------------------
// ${EncodingId:End}

// rv64::InstDB::EncodingData
// =========================

namespace EncodingData {

#define M_OPCODE(field, bits) \
  uint32_t _##field : bits; \
  ASMJIT_INLINE_CONSTEXPR uint32_t field() const noexcept { return uint32_t(_##field) << (32 - bits); }

struct BaseR {
  uint32_t opcode;
  uint32_t funct3;
  uint32_t funct7;
};

struct BaseI {
  uint32_t opcode;
  uint32_t funct3;
};

struct BaseS {
  uint32_t opcode;
  uint32_t funct3;
};

struct BaseB {
  uint32_t opcode;
  uint32_t funct3;
};

struct BaseU {
  uint32_t opcode;
};

struct BaseJ {
  uint32_t opcode;
};

struct BaseLui {
  uint32_t opcode;
};

struct BaseAuipc {
  uint32_t opcode;
};

struct BaseJal {
  uint32_t opcode;
};

struct BaseJalr {
  uint32_t opcode;
  uint32_t funct3;
};

struct BaseBranch {
  uint32_t opcode;
  uint32_t funct3;
};

struct BaseLoad {
  uint32_t opcode;
  uint32_t funct3;
};

struct BaseStore {
  uint32_t opcode;
  uint32_t funct3;
};

struct BaseOpImm {
  uint32_t opcode;
  uint32_t funct3;
};

struct BaseOp {
  uint32_t opcode;
  uint32_t funct3;
  uint32_t funct7;
};

struct BaseFence {
  uint32_t opcode;
  uint32_t funct3;
};

struct BaseSystem {
  uint32_t opcode;
  uint32_t funct3;
};

#undef M_OPCODE

// ${EncodingDataForward:Begin}
// ------------------- Automatically generated, do not edit -------------------
extern const BaseR baseR[16];
extern const BaseI baseI[16];
extern const BaseS baseS[8];
extern const BaseB baseB[8];
extern const BaseU baseU[2];
extern const BaseJ baseJ[1];
extern const BaseLui baseLui[1];
extern const BaseAuipc baseAuipc[1];
extern const BaseJal baseJal[1];
extern const BaseJalr baseJalr[1];
extern const BaseBranch baseBranch[6];
extern const BaseLoad baseLoad[5];
extern const BaseStore baseStore[3];
extern const BaseOpImm baseOpImm[8];
extern const BaseOp baseOp[10];
extern const BaseFence baseFence[1];
extern const BaseSystem baseSystem[4];
// ----------------------------------------------------------------------------
// ${EncodingDataForward:End}

} // {EncodingData}

// rv64::InstDB - Tables
// ====================

#ifndef ASMJIT_NO_TEXT
extern const InstNameIndex instNameIndex;
extern const char _instNameStringTable[];
extern const uint32_t _instNameIndexTable[];
#endif // !ASMJIT_NO_TEXT

} // {InstDB}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RV64INSTDB_H_P_INCLUDED