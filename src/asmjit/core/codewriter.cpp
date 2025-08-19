// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#include "../core/codeholder.h"
#include "../core/codewriter_p.h"
#include "../arm/armutils.h"
#include <cstdio>

ASMJIT_BEGIN_NAMESPACE

bool CodeWriterUtils::encodeOffset32(uint32_t* dst, int64_t offset64, const OffsetFormat& format) noexcept {
  uint32_t bitCount = format.immBitCount();
  uint32_t bitShift = format.immBitShift();
  uint32_t discardLsb = format.immDiscardLsb();

  // Invalid offset (should not happen).
  if (!bitCount || bitCount > format.valueSize() * 8u) {
    return false;
  }

  uint32_t value;
  uint32_t u = 0;
  bool unsignedLogic = format.type() == OffsetType::kUnsignedOffset;

  // First handle all offsets that use additional field for their sign and the offset is encoded as its
  // absolute value.
  if (format.hasSignBit()) {
    u = uint32_t(offset64 >= 0);
    if (u == 0) {
      offset64 = -offset64;
    }
    unsignedLogic = true;
  }

  // First handle all unsigned offset types.
  if (unsignedLogic) {
    if (discardLsb) {
      ASMJIT_ASSERT(discardLsb <= 32);
      if ((offset64 & Support::lsbMask<uint32_t>(discardLsb)) != 0) {
        return false;
      }
      offset64 = int64_t(uint64_t(offset64) >> discardLsb);
    }

    value = uint32_t(offset64 & Support::lsbMask<uint32_t>(bitCount));
    if (value != offset64) {
      return false;
    }
  }
  else {
    // The rest of OffsetType options are all signed.
    if (discardLsb) {
      ASMJIT_ASSERT(discardLsb <= 32);
      if ((offset64 & Support::lsbMask<uint32_t>(discardLsb)) != 0) {
        return false;
      }
      offset64 >>= discardLsb;
    }

    if (!Support::isInt32(offset64)) {
      return false;
    }

    value = uint32_t(int32_t(offset64));
    if (!Support::isEncodableOffset32(int32_t(value), bitCount)) {
      return false;
    }
  }

  switch (format.type()) {
    case OffsetType::kSignedOffset:
    case OffsetType::kUnsignedOffset: {
      *dst = (value & Support::lsbMask<uint32_t>(bitCount)) << bitShift;
      return true;
    }

    // Opcode: {.....|imm:1|..N.N|......|imm:3|....|imm:8}
    case OffsetType::kThumb32_ADR: {
      // Sanity checks.
      if (format.valueSize() != 4 || bitCount != 12 || bitShift != 0) {
        return false;
      }

      uint32_t imm8 = (value & 0x00FFu);
      uint32_t imm3 = (value & 0x0700u) << (12 - 8);
      uint32_t imm1 = (value & 0x0800u) << (26 - 11);
      uint32_t n = u ^ 1u;

      *dst = imm8 | imm3 | imm1 | (n << 21) | (n << 23);
      return true;
    }

    // Opcode: {....|.|imm[22]|imm[19:10]|..|ja|.|jb|imm[9:0]|.}
    case OffsetType::kThumb32_BLX:
      // The calculation is the same as `B`, but the first LSB bit must be zero, so account for that.
      value <<= 1;
      [[fallthrough]];

    // Opcode: {....|.|imm[23]|imm[20:11]|..|ja|.|jb|imm[10:0]}
    case OffsetType::kThumb32_B: {
      // Sanity checks.
      if (format.valueSize() != 4) {
        return false;
      }

      uint32_t ia = (value & 0x0007FFu);
      uint32_t ib = (value & 0x1FF800u) << (16 - 11);
      uint32_t ic = (value & 0x800000u) << (26 - 23);
      uint32_t ja = ((~value >> 23) ^ (value >> 22)) & 1u;
      uint32_t jb = ((~value >> 23) ^ (value >> 21)) & 1u;

      *dst = ia | ib | ic | (ja << 14) | (jb << 11);
      return true;
    }

    // Opcode: {....|.|imm[19]|....|imm[16:11]|..|ja|.|jb|imm[10:0]}
    case OffsetType::kThumb32_BCond: {
      // Sanity checks.
      if (format.valueSize() != 4 || bitCount != 20 || bitShift != 0) {
        return false;
      }

      uint32_t ia = (value & 0x0007FFu);
      uint32_t ib = (value & 0x01F800u) << (16 - 11);
      uint32_t ic = (value & 0x080000u) << (26 - 19);
      uint32_t ja = ((~value >> 19) ^ (value >> 22)) & 1u;
      uint32_t jb = ((~value >> 19) ^ (value >> 21)) & 1u;

      *dst = ia | ib | ic | (ja << 14) | (jb << 11);
      return true;
    }

    case OffsetType::kAArch32_ADR: {
      uint32_t encodedImm;
      if (!arm::Utils::encodeAArch32Imm(value, &encodedImm)) {
        return false;
      }

      *dst = (Support::bitMask(22) << u) | (encodedImm << bitShift);
      return true;
    }

    case OffsetType::kAArch32_U23_SignedOffset: {
      *dst = (value << bitShift) | (u << 23);
      return true;
    }

    case OffsetType::kAArch32_U23_0To3At0_4To7At8: {
      // Sanity checks.
      if (format.valueSize() != 4 || bitCount != 8 || bitShift != 0) {
        return false;
      }

      uint32_t immLo = (value & 0x0Fu);
      uint32_t immHi = (value & 0xF0u) << (8 - 4);

      *dst = immLo | immHi | (u << 23);
      return true;
    }

    case OffsetType::kAArch32_1To24At0_0At24: {
      // Sanity checks.
      if (format.valueSize() != 4 || bitCount != 25 || bitShift != 0) {
        return false;
      }

      uint32_t immLo = (value & 0x0000001u) << 24;
      uint32_t immHi = (value & 0x1FFFFFEu) >> 1;

      *dst = immLo | immHi;
      return true;
    }

    case OffsetType::kAArch64_ADR:
    case OffsetType::kAArch64_ADRP: {
      // Sanity checks.
      if (format.valueSize() != 4 || bitCount != 21 || bitShift != 5) {
        return false;
      }

      uint32_t immLo = value & 0x3u;
      uint32_t immHi = (value >> 2) & Support::lsbMask<uint32_t>(19);

      *dst = (immLo << 29) | (immHi << 5);
      return true;
    }

    case OffsetType::kRISCV64_BType: {
      // RISC-V B-Type
      if (format.valueSize() != 4 || bitCount != 13 || bitShift != 0) {
        return false;
      }

      if ((offset64 & 1) != 0) {
        return false;
      }

      if (offset64 < -4096 || offset64 > 4095) {
        return false;
      }
      printf("RISCV-B Type Offset64: %ld\n", offset64);
      uint32_t imm = static_cast<uint32_t>(offset64) & 0x1FFE;
      uint32_t imm_12 = (imm >> 12) & 1;
      uint32_t imm_11 = (imm >> 11) & 1;
      uint32_t imm_10_5 = (imm >> 5) & 0x3F;
      uint32_t imm_4_1 = (imm >> 1) & 0xF;

      *dst &= 0x01FFF07F; // 保留除立即数外的所有位
      *dst |= (imm_12 << 31);    // imm[12]
      *dst |= (imm_10_5 << 25);  // imm[10:5]
      *dst |= (imm_4_1 << 8);    // imm[4:1]
      *dst |= (imm_11 << 7);     // imm[11]
      
      return true;
    }

    // I-Type 指令重定位
    case OffsetType::kRISCV64_I_Lo12: {
      printf("lo12 offset %ld\n", offset64);
      // 参数检查: 值大小为4字节, 12位立即数, 无位移
      if (format.valueSize() != 4 || bitCount != 12 || bitShift != 20) {
          return false;
      }

      // 检查偏移量是否在12位有符号范围内 [-2048, 2047]
      if (offset64 < -2048 || offset64 > 2047) {
        return false;
      }

      // 提取低 12 位（带符号扩展）
      int32_t lo12 = offset64 & 0xFFF;
      if (lo12 & 0x800) {
        // 如果第 12 位为 1，进行符号扩展
        lo12 |= 0xFFFFF000;
      }
      
      // 清除指令中的立即数字段 [31:20]
      *dst &= 0x000FFFFF;
      
      // 将立即数写入 [31:20]
      *dst |= ((uint32_t(lo12) & 0xFFF) << 20);
      printf("offset code %x, opcode %x\n", lo12, *dst);

      
      return true;
    }

    // U-Type 指令重定位 (e.g., auipc, lui)
    case OffsetType::kRISCV64_U_Hi20: {
      printf("RISCV-U Type: offset=%ld\n", offset64);
      if (format.valueSize() != 4 || bitCount != 20 || bitShift != 12) {
          return false;
      }

      // 检查偏移量是否在32位有符号范围内 [-0x80000000, 0x7FFFFFFF]
      if (offset64 < static_cast<int64_t>(0xFFFFFFFF80000000) ||
          offset64 > 0x7FFFFFFF) {
          return false;
      }
      int64_t adjusted_offset = offset64;
  
      // 提取调整后的高 20 位
      uint32_t hi20 = (offset64 >> 12) & 0xFFFFF;

      // 清除目标指令中的立即数字段 (位[31:12])
      *dst &= 0x00000FFF;

      // 将立即数写入指令的 [31:12] 位置
      *dst |= (hi20 << 12);
      printf("offset code %x, opcode %x\n", hi20, *dst);

      return true;
    }


    // 新增：RISC-V AUIPC+LD 组合处理
    case OffsetType::kRISCV64_AUIPC_LD: {
      printf("Processing RISCV AUIPC+LD fixup: offset64=%ld (0x%lx)\n", offset64, offset64);
      
      // dst 指向 AUIPC 指令
      uint32_t* auipc_ptr = dst;
      uint32_t* ld_ptr = dst + 1;  // LD 指令在 AUIPC 后 4 字节
      
      // 计算高 20 位和低 12 位
      int64_t adjusted_offset = offset64;
      
      // 如果低 12 位会被符号扩展为负数，需要调整高位
      if (offset64 & 0x800) {
        adjusted_offset += 0x1000;
      }
      
      uint32_t hi20 = (adjusted_offset >> 12) & 0xFFFFF;
      uint32_t lo12 = offset64 & 0xFFF;
      
      // 更新 AUIPC 指令的立即数（位 [31:12]）
      uint32_t auipc = *auipc_ptr;
      auipc = (auipc & 0x00000FFF) | (hi20 << 12);
      *auipc_ptr = auipc;
      
      // 更新 LD 指令的立即数（位 [31:20]）
      uint32_t ld = *ld_ptr;
      ld = (ld & 0x000FFFFF) | (lo12 << 20);
      *ld_ptr = ld;
      
      printf("Updated AUIPC: 0x%08x (hi20=0x%x)\n", auipc, hi20);
      printf("Updated LD: 0x%08x (lo12=0x%x)\n", ld, lo12);
      
      return true;
    }
    default:
      return false;
  }
}

bool CodeWriterUtils::encodeOffset64(uint64_t* dst, int64_t offset64, const OffsetFormat& format) noexcept {
  uint32_t bitCount = format.immBitCount();
  uint32_t discardLsb = format.immDiscardLsb();

  if (!bitCount || bitCount > format.valueSize() * 8u) {
    return false;
  }

  uint64_t value;

  // First handle all unsigned offset types.
  if (format.type() == OffsetType::kUnsignedOffset) {
    if (discardLsb) {
      ASMJIT_ASSERT(discardLsb <= 32);
      if ((offset64 & Support::lsbMask<uint32_t>(discardLsb)) != 0) {
        return false;
      }
      offset64 = int64_t(uint64_t(offset64) >> discardLsb);
    }

    value = uint64_t(offset64) & Support::lsbMask<uint64_t>(bitCount);
    if (value != uint64_t(offset64)) {
      return false;
    }
  }
  else {
    // The rest of OffsetType options are all signed.
    if (discardLsb) {
      ASMJIT_ASSERT(discardLsb <= 32);
      if ((offset64 & Support::lsbMask<uint32_t>(discardLsb)) != 0) {
        return false;
      }
      offset64 >>= discardLsb;
    }

    if (!Support::isEncodableOffset64(offset64, bitCount)) {
      return false;
    }

    value = uint64_t(offset64);
  }

  switch (format.type()) {
    case OffsetType::kSignedOffset:
    case OffsetType::kUnsignedOffset: {
      *dst = (value & Support::lsbMask<uint64_t>(bitCount)) << format.immBitShift();
      return true;
    }

    default:
      return false;
  }
}

bool CodeWriterUtils::writeOffset(void* dst, int64_t offset64, const OffsetFormat& format) noexcept {
  // 特殊处理 AUIPC+LD 组合
  if (format.type() == OffsetType::kRISCV64_AUIPC_LD) {
    // 对于 AUIPC+LD，我们需要处理 8 字节（两条指令）
    uint32_t* instructions = static_cast<uint32_t*>(dst);
    return encodeOffset32(instructions, offset64, format);
  }
    
  dst = static_cast<char*>(dst) + format.valueOffset();

  // Offset the destination by ValueOffset so the `dst` points to the
  // patched word instead of the beginning of the patched region.
  dst = static_cast<char*>(dst) + format.valueOffset();
  //printf("dst %p, offset64: %ld, switch: %d\n", dst, offset64, format.valueSize());
  switch (format.valueSize()) {
    case 1: {
      uint32_t mask;
      if (!encodeOffset32(&mask, offset64, format)) {
        return false;
      }

      Support::store_u8(dst, uint8_t(Support::load_u8(dst) | mask));
      return true;
    }

    case 2: {
      uint32_t mask;
      if (!encodeOffset32(&mask, offset64, format)) {
        return false;
      }

      Support::storeu_u16_le(dst, uint16_t(Support::loadu_u16_le(dst) | mask));
      return true;
    }

    case 4: {
      uint32_t mask;
      printf("case4\n");
      if (!encodeOffset32(&mask, offset64, format)) {
      printf("case4 failed\n");
        return false;
      }

      Support::storeu_u32_le(dst, Support::loadu_u32_le(dst) | mask);
      return true;
    }

    case 8: {
      uint64_t mask;
      if (!encodeOffset64(&mask, offset64, format)) {
        return false;
      }

      Support::storeu_u64_le(dst, Support::loadu_u64_le(dst) | mask);
      return true;
    }

    default:
      return false;
  }
}

ASMJIT_END_NAMESPACE
