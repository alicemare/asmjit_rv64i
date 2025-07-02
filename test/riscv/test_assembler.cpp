

#include "asmjit/core/codebuffer.h"
#include <asmjit/riscv.h>
#include <asmjit/core/operand.h>
#include <asmjit/riscv/riscvinstdb.h>
#include <asmjit/riscv/riscvassembler.h>
#include <asmjit/riscv/riscvcompiler.h>

#include <iostream>
using namespace asmjit;

void printCodeBuffer(const CodeHolder& code) {
    const CodeBuffer cb = code.textSection()->buffer();
    const uint8_t* data = cb.data();
    const size_t size = cb.size();
    
    printf("Code buffer size: %zu bytes\n", size);
    printf("Buffer content (32-bit hex):\n");
    
    for (size_t i = 0; i < size; i += 4) {
        if (i + 4 <= size) {
            uint32_t instruction = *(uint32_t*)(data + i);
            printf("%08x\n", instruction);
        } else {
            // shuold not happen
            uint32_t instruction = 0;
            for (size_t j = i; j < size; j++) {
                instruction |= (uint32_t)data[j] << ((j - i) * 8);
            }
            printf("%08x (partial)\n", instruction);
        }
    }
}


int main() {
    // 重要：创建RISC-V 64位环境，不然 CodeHolder 是 nullptr
    asmjit::Environment env = asmjit::Environment(asmjit::Arch::kRISCV64);
    asmjit::CodeHolder code;
    asmjit::Error err = code.init(env);
    if (err) {
        std::cerr << "CodeHolder初始化失败: " << asmjit::DebugUtils::errorAsString(err) << std::endl;
        return 1;
    }
    riscv::Assembler assembler(&code);

    std::cout << "--- Testing RV64I Assembler Functionality ---" << std::endl;

    using namespace riscv::Inst;
    using namespace riscv::regs;
    // --- R-Type 指令测试 ---
    std::cout << "\nGenerating R-Type instructions:" << std::endl;
    Operand_* opExt;
    
    std::cout << "  ADD x1, x2, x3" << std::endl;
    assembler._emit(kIdAdd, x1, x2, x3, opExt);
    std::cout << "  ADDW x1, x2, x3" << std::endl;
    assembler._emit(kIdAddw, x1, x2, x3, opExt);
    std::cout << "  SUB x4, x5, x6" << std::endl;
    assembler._emit(kIdSub, x4, x5, x6, opExt);
    std::cout << "  SUBW x4, x5, x6" << std::endl;
    assembler._emit(kIdSubw, x4, x5, x6, opExt);
    std::cout << "  SLL x7, x8, x9" << std::endl;
    assembler._emit(kIdSll, x7, x8, x9, opExt);
    std::cout << "  SLLW x7, x8, x9" << std::endl;
    assembler._emit(kIdSllw, x7, x8, x9, opExt);
    std::cout << "  SRL x7, x8, x9" << std::endl;
    assembler._emit(kIdSrl, x7, x8, x9, opExt);
    std::cout << "  SRLW x7, x8, x9" << std::endl;
    assembler._emit(kIdSrlw, x7, x8, x9, opExt);
    std::cout << "  SRA x7, x8, x9" << std::endl;
    assembler._emit(kIdSra, x7, x8, x9, opExt);
    std::cout << "  SRAW x7, x8, x9" << std::endl;
    assembler._emit(kIdSraw, x7, x8, x9, opExt);
    std::cout << "  SLT x7, x8, x9" << std::endl;
    assembler._emit(kIdSlt, x7, x8, x9, opExt);
    std::cout << "  SLTU x7, x8, x9" << std::endl;
    assembler._emit(kIdSltu, x7, x8, x9, opExt);
    std::cout << "  AND x7, x8, x9" << std::endl;
    assembler._emit(kIdAnd, x7, x8, x9, opExt);
    std::cout << "  OR x7, x8, x9" << std::endl;
    assembler._emit(kIdOr, x7, x8, x9, opExt);
    std::cout << "  XOR x7, x8, x9" << std::endl;
    assembler._emit(kIdXor, x7, x8, x9, opExt);
    // --- I-Type 指令测试 ---
    std::cout << "\nGenerating I-Type instructions:" << std::endl;

    std::cout << "  ADDI x7, x8, 100" << std::endl;
    assembler._emit(kIdAddi, x7, x8, Imm(100), opExt);
    std::cout << "  ADDIW x7, x8, 99" << std::endl;
    assembler._emit(kIdAddiw, x7, x8, Imm(99), opExt);
    std::cout << "  SLTI x7, x8, 88" << std::endl;
    assembler._emit(kIdSlti, x7, x8, Imm(88), opExt);
    std::cout << "  SLTIU x7, x8, 77" << std::endl;
    assembler._emit(kIdSltiu, x7, x8, Imm(77), opExt);
    // todo
    std::cout << "  LD x9, 16(x10)" << std::endl;
    assembler._emit(kIdLd, x9, x10, Imm(16), opExt);
    std::cout << "  JALR x11, 24(x12)" << std::endl;
    assembler._emit(kIdJalr, x11, x12, Imm(24), opExt);

    // --- S-Type 指令测试 ---
    std::cout << "\nGenerating S-Type instructions:" << std::endl;

    std::cout << "  SB x13, 32(x14)" << std::endl;
    assembler._emit(kIdSb, x13, x14, Imm(32), opExt);
    std::cout << "  SH x13, 32(x14)" << std::endl;
    assembler._emit(kIdSh, x13, x14, Imm(32), opExt);
    std::cout << "  SW x13, 32(x14)" << std::endl;
    assembler._emit(kIdSw, x13, x14, Imm(32), opExt);
    std::cout << "  SD x13, 32(x14)" << std::endl;
    assembler._emit(kIdSd, x13, x14, Imm(32), opExt);

    // --- B-Type 指令测试 ---
    std::cout << "\nGenerating B-Type instructions:" << std::endl;

    std::cout << "  BEQ x15, x16, 40" << std::endl;
    assembler._emit(kIdBeq, x15, x16, Imm(40), opExt);
    std::cout << "  BNE x15, x16, 40" << std::endl;
    assembler._emit(kIdBne, x15, x16, Imm(40), opExt);
    std::cout << "  BLT x15, x16, 40" << std::endl;
    assembler._emit(kIdBlt, x15, x16, Imm(40), opExt);
    std::cout << "  BGE x15, x16, 40" << std::endl;
    assembler._emit(kIdBge, x15, x16, Imm(40), opExt);
    std::cout << "  BLTU x15, x16, 40" << std::endl;
    assembler._emit(kIdBltu, x15, x16, Imm(40), opExt);
    std::cout << "  BGEU x15, x16, 40" << std::endl;
    assembler._emit(kIdBgeu, x15, x16, Imm(40), opExt);

    // --- J-Type 指令测试 ---
    Operand_ dummy;
    std::cout << "\nGenerating J-Type instructions:" << std::endl;
    std::cout << "  JAL x19, 64" << std::endl;
    assembler._emit(kIdJal, x19, Imm(64), dummy, opExt);

    // --- U-Type 指令测试 ---
    std::cout << "\nGenerating U-Type instructions:" << std::endl;
    std::cout << "  AUIPC x17, 0x10000000" << std::endl;
    assembler._emit(kIdAuipc, x17, Imm(0x10000000), dummy, opExt);
    std::cout << "  LUI x18, 0x20000000" << std::endl;
    assembler._emit(kIdLui, x18, Imm(0x20000000), dummy, opExt);


    std::cout << "--- Test Cases Finished ---" << std::endl;
    printCodeBuffer(code);
    std::cout << "Please verify the generated machine codes manually using https://luplab.gitlab.io/rvcodecjs/" << std::endl;

    return 0;
}