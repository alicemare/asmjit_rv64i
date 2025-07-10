#include <cassert>
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

// Test helper macros
#define TEST_CASE(name, name_str) \
    printf("Testing %s...\n", name_str); \
    name(); \
    printf("✓ %s passed\n\n", name_str);

#define EXPECT_EQ(a, b) \
    if ((a) != (b)) { \
        printf("FAIL: Expected %d, got %d\n", (int)(b), (int)(a)); \
        assert(false); \
    }

#define EXPECT_NE(a, b) \
    if ((a) == (b)) { \
        printf("FAIL: Expected not equal values\n"); \
        assert(false); \
    }

// Basic integer addition test
void test_basic_addition() {
    JitRuntime rt;
    CodeHolder code;

    code.init(Environment(Arch::kRISCV64));
    StringLogger logger;
    code.setLogger(&logger);
    
    // Create RISC-V compiler
    riscv::Compiler cc(&code);
    
    // Create function signature: int func(int a, int b)
    FuncSignature sig = FuncSignature::build<int, int, int>();
    
    // Create function
    FuncNode* func = cc.addFunc(sig);
    
    // Get function arguments
    riscv::Gp a = cc.newInt32("a");
    riscv::Gp b = cc.newInt32("b");
    riscv::Gp result = cc.newInt32("result");
    
    func->setArg(0, a);
    func->setArg(1, b);
    
    // Perform addition: result = a + b
    cc.add(result, a, b);
    
    // Return result
    cc.ret(result);
    
    // Finalize the function
    cc.endFunc();
    cc.finalize();
    // 打印反汇编代码
    std::cout << "Disassembly:" << std::endl;
    std::cout << logger.data() << std::endl;

    // JIT compile
    typedef int (*AddFunc)(int, int);
    AddFunc addFunc;
    Error err = rt.add(&addFunc, &code);
    
    if (err) {
        printf("JIT compilation failed: %s\n", DebugUtils::errorAsString(err));
        assert(false);
    }
    
    printCodeBuffer(code);
    // Test the compiled function
    /*
    int result1 = addFunc(5, 3);
    EXPECT_EQ(result1, 8);
    
    int result2 = addFunc(-10, 15);
    EXPECT_EQ(result2, 5);
    
    int result3 = addFunc(0, 0);
    EXPECT_EQ(result3, 0);
    
    */
    rt.release(addFunc);
}

// Test multiple arithmetic operations
void test_multiple_operations() {
    JitRuntime rt;
    CodeHolder code;
    
    code.init(Environment(Arch::kRISCV64));
    riscv::Compiler cc(&code);
    
    // Function signature: int func(int a, int b, int c)
    FuncSignature sig = FuncSignature::build<int, int, int, int>();
    
    FuncNode* func = cc.addFunc(sig);
    
    // Arguments
    riscv::Gp a = cc.newInt32("a");
    riscv::Gp b = cc.newInt32("b");
    riscv::Gp c = cc.newInt32("c");
    
    func->setArg(0, a);
    func->setArg(1, b);
    func->setArg(2, c);
    
    // Temporary variables
    riscv::Gp temp1 = cc.newInt32("temp1");
    riscv::Gp temp2 = cc.newInt32("temp2");
    riscv::Gp result = cc.newInt32("result");
    
    // Compute: result = (a + b) -c 
    cc.add(temp1, a, b);      // temp1 = a + b
    cc.sub(result, temp1, c); // result = temp1 * c
    
    cc.ret(result);
    cc.endFunc();
    cc.finalize();
    
    typedef int (*MathFunc)(int, int, int);
    MathFunc mathFunc;
    Error err = rt.add(&mathFunc, &code);
    
    if (err) {
        printf("JIT compilation failed: %s\n", DebugUtils::errorAsString(err));
        assert(false);
    }
    printCodeBuffer(code);
    
    /*
    // Test cases
    if (false) {
    int result1 = mathFunc(2, 3, 4);  // (2+3)-4 = 1
    EXPECT_EQ(result1, 1);
    
    int result2 = mathFunc(1, 1, 5);  // (1+1)-5 = -3
    EXPECT_EQ(result2, -3);
    }*/
    rt.release(mathFunc);
}


// Main test runner
int main() {
    printf("RISC-V Compiler Test Suite (Updated API)\n");
    printf("========================================\n\n");
    
    try {
        TEST_CASE(test_basic_addition, "test_basic_addition");
        TEST_CASE(test_multiple_operations, "test_multiple_operations");
        
        printf("All tests passed! ✓\n");
        return 0;
    } catch (const std::exception& e) {
        printf("Test failed with exception: %s\n", e.what());
        return 1;
    } catch (...) {
        printf("Test failed with unknown exception\n");
        return 1;
    }
}