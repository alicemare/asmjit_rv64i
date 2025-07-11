// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "asmjit/riscv/riscvassembler.h"
#include "asmjit_test_assembler.h"
#include <asmjit/core.h>
#if !defined(ASMJIT_NO_RISCV)

#include <asmjit/riscv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asmjit_test_compiler.h"
#include "cmdline.h"

#define TEST_INSTRUCTION(OPCODE, ...) \
    tester.testValidInstruction(#__VA_ARGS__, OPCODE, tester.assembler.__VA_ARGS__)

using namespace asmjit;

static void ASMJIT_NOINLINE testRISCVAssemblerBase(AssemblerTester<riscv::Assembler>& tester) noexcept {
    using namespace riscv;
    using namespace riscv::regs;

    // R-Type
    TEST_INSTRUCTION("B3003100", add(x1,x2,x3));
    TEST_INSTRUCTION("3B814100", addw(x2, x3, x4));
    TEST_INSTRUCTION("B3015240", sub(x3,x4,x5));
    TEST_INSTRUCTION("3B826240", subw(x4, x5, x6));
    TEST_INSTRUCTION("B3127300", sll(x5, x6, x7));
    TEST_INSTRUCTION("3B938300", sllw(x6, x7, x8));
    TEST_INSTRUCTION("B3539400", srl(x7, x8, x9));
    TEST_INSTRUCTION("3BD4A400", srlw(x8, x9, x10));
    TEST_INSTRUCTION("BB54B500", srlw(x9, x10, x11));
    TEST_INSTRUCTION("3BD5C540", sraw(x10, x11, x12));
    TEST_INSTRUCTION("B325D600", slt(x11, x12, x13));
    TEST_INSTRUCTION("33B6E600", sltu(x12, x13, x14));
    TEST_INSTRUCTION("B376F700", and_(x13, x14, x15));
    TEST_INSTRUCTION("33E70701", or_(x14, x15, x16));
    TEST_INSTRUCTION("B3471801", xor_(x15, x16, x17));
    // I-Type
    TEST_INSTRUCTION("13895901", addi(x18, x19, 21));
    TEST_INSTRUCTION("1B8A6A01", addiw(x20, x21, 22));
    TEST_INSTRUCTION("932A2B00", slti(x21, x22, 2));
    TEST_INSTRUCTION("13BB3B00", sltiu(x22, x23, 3));
    TEST_INSTRUCTION("937B9C01", andi(x23, x24, 25));
    TEST_INSTRUCTION("13ECAC01", ori(x24, x25, 26));
    TEST_INSTRUCTION("934CBD01", xori(x25, x26, 27));
    TEST_INSTRUCTION("139D1D00", slli(x26, x27, 1));
    TEST_INSTRUCTION("9B1D2E00", slliw(x27, x28, 2));
    TEST_INSTRUCTION("935E1F00", srli(x29, x30, 1));
    TEST_INSTRUCTION("1BDF2F00", srliw(x30, x31, 2));
    TEST_INSTRUCTION("935F0040", srai(x31, x0, 0));
    TEST_INSTRUCTION("9BD04040", sraiw(x1, x1, 4));
    TEST_INSTRUCTION("67800000", jalr(x0, x1, 0));
    ///Load
    TEST_INSTRUCTION("03810000", lb(x2, Mem(x1)));
    TEST_INSTRUCTION("03910000", lh(x2, Mem(x1)));
    TEST_INSTRUCTION("03A10000", lw(x2, Mem(x1)));
    TEST_INSTRUCTION("03C10000", lbu(x2, Mem(x1)));
    TEST_INSTRUCTION("03D10000", lhu(x2, Mem(x1)));
    TEST_INSTRUCTION("03E10000", lwu(x2, Mem(x1)));
    TEST_INSTRUCTION("03B10000", ld(x2, Mem(x1)));
    // S-Type
    TEST_INSTRUCTION("23802000", sb(x2, Mem(x1, 0)));
    TEST_INSTRUCTION("A3902000", sh(x2, Mem(x1, 1)));
    TEST_INSTRUCTION("A3AF20FE", sw(x2, Mem(x1, -1)));
    TEST_INSTRUCTION("23B4203E", sd(x2, Mem(x1, 1000)));
    // U-Type
    TEST_INSTRUCTION("37F10F00", lui(x2, 0xFF));
    TEST_INSTRUCTION("97F10F00", auipc(x3, 0xFF));
}

static void ASMJIT_NOINLINE testRISCVAssemblerRel(AssemblerTester<riscv::Assembler>& tester) noexcept {
    using namespace riscv;
    using namespace riscv::regs;
    
    // Must be a reference, because it's recreated after every `TEST_INSTRUCTION()`.
    const Label& L0 = tester.L0;
    // B-Type
    TEST_INSTRUCTION("63802000", beq(x1, x2, L0));
    TEST_INSTRUCTION("63902000", bne(x1, x2, L0));
    TEST_INSTRUCTION("63C02000", blt(x1, x2, L0));
    TEST_INSTRUCTION("63D02000", bge(x1, x2, L0));
    TEST_INSTRUCTION("63E02000", bltu(x1, x2, L0));
    TEST_INSTRUCTION("63F02000", bgeu(x1, x2, L0));
    // J-TYpe
    TEST_INSTRUCTION("6F000000", jal(x0, L0));
}


bool testRISCVAssembler(const TestSettings& settings) noexcept {
    using namespace riscv;
  
    AssemblerTester<Assembler> tester(Arch::kRISCV64, settings);
    tester.printHeader("RISCV");
  
    testRISCVAssemblerBase(tester);
    testRISCVAssemblerRel(tester);
    
    tester.printSummary();
    return tester.didPass();
}

#undef TEST_INSTRUCTION

#endif