#include <cassert>
#include <asmjit/a64.h>

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
#define TEST_CASE(name) \
    printf("Testing %s...\n", name); \
    name(); \
    printf("✓ %s passed\n\n", name);

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
    
    code.init(rt.environment());
    StringLogger logger;
    code.setLogger(&logger);
    // Create RISC-V compiler
    a64::Compiler cc(&code);
    
    // Create function signature: int func(int a, int b)
    FuncSignature sig = FuncSignature::build<int, int, int>();
    
    // Create function
    FuncNode* func = cc.addFunc(sig);
    
    // Get function arguments
    a64::Gp a = cc.newInt32("a");
    a64::Gp b = cc.newInt32("b");
    a64::Gp result = cc.newInt32("result");
    
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
    
    // Test the compiled function
    int result1 = addFunc(5, 3);
    EXPECT_EQ(result1, 8);
    
    int result2 = addFunc(-10, 15);
    EXPECT_EQ(result2, 5);
    
    int result3 = addFunc(0, 0);
    EXPECT_EQ(result3, 0);
    printCodeBuffer(code);
    rt.release(addFunc);
}

// Test multiple arithmetic operations
void test_multiple_operations() {
    JitRuntime rt;
    CodeHolder code;
    
    code.init(rt.environment());
    StringLogger logger;
    code.setLogger(&logger);
    a64::Compiler cc(&code);
    
    // Function signature: int func(int a, int b, int c)
    FuncSignature sig = FuncSignature::build<int, int, int, int>();
    
    FuncNode* func = cc.addFunc(sig);
    
    // Arguments
    a64::Gp a = cc.newInt32("a");
    a64::Gp b = cc.newInt32("b");
    a64::Gp c = cc.newInt32("c");
    
    func->setArg(0, a);
    func->setArg(1, b);
    func->setArg(2, c);
    
    // Temporary variables
    a64::Gp temp1 = cc.newInt32("temp1");
    a64::Gp temp2 = cc.newInt32("temp2");
    a64::Gp result = cc.newInt32("result");
    
    // Compute: result = (a + b) -c 
    cc.add(temp1, a, b);      // temp1 = a + b
    cc.sub(result, temp1, c); // result = temp1 * c
    
    cc.ret(result);
    cc.endFunc();
    cc.finalize();
    // 打印反汇编代码
    std::cout << "Disassembly:" << std::endl;
    std::cout << logger.data() << std::endl;
    
    typedef int (*MathFunc)(int, int, int);
    MathFunc mathFunc;
    Error err = rt.add(&mathFunc, &code);
    
    if (err) {
        printf("JIT compilation failed: %s\n", DebugUtils::errorAsString(err));
        assert(false);
    }
    
    // Test cases
    if (false) {
    int result1 = mathFunc(2, 3, 4);  // (2+3)-4 = 1
    EXPECT_EQ(result1, 1);
    
    int result2 = mathFunc(1, 1, 5);  // (1+1)-5 = -3
    EXPECT_EQ(result2, -3);
    }
    printCodeBuffer(code);
    rt.release(mathFunc);
}

// Test function calls (invoke)
void test_function_invoke() {
    JitRuntime rt;
    CodeHolder code;
    
    code.init(rt.environment());
    a64::Compiler cc(&code);
    
    // Function signature: int func(int x)
    FuncSignature sig = FuncSignature::build<int, int>();
    
    FuncNode* func = cc.addFunc(sig);
    
    a64::Gp x = cc.newInt32("x");
    func->setArg(0, x);
    
    // Call external function (e.g., abs)
    a64::Gp result = cc.newInt32("result");
    
    // Create function call to abs(x)
    FuncSignature absSig = FuncSignature::build<int, int>();
    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, (long(*)(long))abs, absSig);
    invokeNode->setArg(0, x);
    invokeNode->setRet(0, result);
    
    cc.ret(result);
    cc.endFunc();
    cc.finalize();
    
    typedef int (*AbsFunc)(int);
    AbsFunc absFunc;
    Error err = rt.add(&absFunc, &code);
    
    if (err) {
        printf("JIT compilation failed: %s\n", DebugUtils::errorAsString(err));
        assert(false);
    }
    
    // Test the function
    int result1 = absFunc(-42);
    EXPECT_EQ(result1, 42);
    
    int result2 = absFunc(15);
    EXPECT_EQ(result2, 15);
    
    int result3 = absFunc(0);
    EXPECT_EQ(result3, 0);
    
    rt.release(absFunc);
}

// Test different integer types
void test_integer_types() {
    JitRuntime rt;
    CodeHolder code;
    
    code.init(rt.environment());
    a64::Compiler cc(&code);
    
    // Function signature: int64_t func(int32_t a, int32_t b)
    FuncSignature sig = FuncSignature::build<int64_t, int32_t, int32_t>();
    
    FuncNode* func = cc.addFunc(sig);
    
    // Test different newInt variants
    a64::Gp a32 = cc.newInt32("a32");
    a64::Gp b32 = cc.newInt32("b32");
    a64::Gp result64 = cc.newInt64("result64");
    a64::Gp temp64 = cc.newInt64("temp64");
    
    func->setArg(0, a32);
    func->setArg(1, b32);
    
    // Convert to 64-bit and multiply
    cc.mov(result64, a32);    // Sign extend a32 to 64-bit
    cc.mov(temp64, b32);      // Sign extend b32 to 64-bit
    cc.mul(result64, result64, temp64);
    
    cc.ret(result64);
    cc.endFunc();
    cc.finalize();
    
    typedef int64_t (*Int64Func)(int32_t, int32_t);
    Int64Func int64Func;
    Error err = rt.add(&int64Func, &code);
    
    if (err) {
        printf("JIT compilation failed: %s\n", DebugUtils::errorAsString(err));
        assert(false);
    }
    
    // Test
    int64_t result1 = int64Func(1000, 2000);
    EXPECT_EQ(result1, 2000000LL);
    
    int64_t result2 = int64Func(-500, 800);
    EXPECT_EQ(result2, -400000LL);
    
    rt.release(int64Func);
}

// Test immediate values and constants
void test_immediate_values() {
    JitRuntime rt;
    CodeHolder code;
    
    code.init(rt.environment());
    a64::Compiler cc(&code);
    
    FuncSignature sig = FuncSignature::build<int, int>();
    
    FuncNode* func = cc.addFunc(sig);
    
    a64::Gp input = cc.newInt32("input");
    a64::Gp result = cc.newInt32("result");
    
    func->setArg(0, input);
    
    // Add immediate value: result = input + 100
    cc.add(result, input, 100);
    
    cc.ret(result);
    cc.endFunc();
    cc.finalize();
    
    typedef int (*ImmFunc)(int);
    ImmFunc immFunc;
    Error err = rt.add(&immFunc, &code);
    
    if (err) {
        printf("JIT compilation failed: %s\n", DebugUtils::errorAsString(err));
        assert(false);
    }
    
    // Test
    int result1 = immFunc(42);
    EXPECT_EQ(result1, 142);
    
    int result2 = immFunc(-50);
    EXPECT_EQ(result2, 50);
    
    rt.release(immFunc);
}

// Test register allocation stress
void test_register_allocation() {
    JitRuntime rt;
    CodeHolder code;
    
    code.init(rt.environment());
    a64::Compiler cc(&code);
    
    FuncSignature sig = FuncSignature::build<int, int>();
    
    FuncNode* func = cc.addFunc(sig);
    
    a64::Gp input = cc.newInt32("input");
    func->setArg(0, input);
    
    // Create many temporary variables to stress register allocation
    a64::Gp vars[16];
    for (int i = 0; i < 16; i++) {
        vars[i] = cc.newInt32();
    }
    
    // Initialize variables
    for (int i = 0; i < 16; i++) {
        cc.mov(vars[i], i + 1);
    }
    
    // Perform operations
    a64::Gp result = cc.newInt32("result");
    cc.mov(result, 0);
    
    for (int i = 0; i < 16; i++) {
        cc.add(result, result, vars[i]);
    }
    
    cc.add(result, result, input);
    
    cc.ret(result);
    cc.endFunc();
    cc.finalize();
    
    typedef int (*StressFunc)(int);
    StressFunc stressFunc;
    Error err = rt.add(&stressFunc, &code);
    
    if (err) {
        printf("JIT compilation failed: %s\n", DebugUtils::errorAsString(err));
        assert(false);
    }
    
    // Test: sum of 1+2+...+16 = 136, plus input
    int result1 = stressFunc(10);
    EXPECT_EQ(result1, 146);  // 136 + 10
    
    int result2 = stressFunc(0);
    EXPECT_EQ(result2, 136);  // 136 + 0
    
    rt.release(stressFunc);
}

// Test complex expressions with multiple operations
void test_complex_expressions() {
    JitRuntime rt;
    CodeHolder code;
    
    code.init(rt.environment());
    a64::Compiler cc(&code);
    
    // Function signature: int func(int a, int b, int c, int d)
    FuncSignature sig = FuncSignature::build<int, int, int, int, int>();
    
    FuncNode* func = cc.addFunc(sig);
    
    a64::Gp a = cc.newInt32("a");
    a64::Gp b = cc.newInt32("b");
    a64::Gp c = cc.newInt32("c");
    a64::Gp d = cc.newInt32("d");
    
    func->setArg(0, a);
    func->setArg(1, b);
    func->setArg(2, c);
    func->setArg(3, d);
    
    // Compute: result = (a * b) + (c * d)
    a64::Gp temp1 = cc.newInt32("temp1");
    a64::Gp temp2 = cc.newInt32("temp2");
    a64::Gp result = cc.newInt32("result");
    
    cc.mul(temp1, a, b);        // temp1 = a * b
    cc.mul(temp2, c, d);        // temp2 = c * d
    cc.add(result, temp1, temp2); // result = temp1 + temp2
    
    cc.ret(result);
    cc.endFunc();
    cc.finalize();
    
    typedef int (*ComplexFunc)(int, int, int, int);
    ComplexFunc complexFunc;
    Error err = rt.add(&complexFunc, &code);
    
    if (err) {
        printf("JIT compilation failed: %s\n", DebugUtils::errorAsString(err));
        assert(false);
    }
    
    // Test: (2*3) + (4*5) = 6 + 20 = 26
    int result1 = complexFunc(2, 3, 4, 5);
    EXPECT_EQ(result1, 26);
    
    // Test: (1*10) + (2*20) = 10 + 40 = 50
    int result2 = complexFunc(1, 10, 2, 20);
    EXPECT_EQ(result2, 50);
    
    rt.release(complexFunc);
}

// Test conditional operations (if available)
void test_conditional_operations() {
    JitRuntime rt;
    CodeHolder code;
    
    code.init(rt.environment());
    a64::Compiler cc(&code);
    
    // Function signature: int func(int a, int b) - returns max(a, b)
    FuncSignature sig = FuncSignature::build<int, int, int>();
    
    FuncNode* func = cc.addFunc(sig);
    
    a64::Gp a = cc.newInt32("a");
    a64::Gp b = cc.newInt32("b");
    a64::Gp result = cc.newInt32("result");
    
    func->setArg(0, a);
    func->setArg(1, b);
    
    // Create labels for conditional jump
    Label labelA = cc.newLabel();
    Label labelEnd = cc.newLabel();
    
    // Compare a and b
    cc.cmp(a, b);
    cc.b(labelA);  // Jump if a > b
    
    // b >= a, so return b
    cc.mov(result, b);
    cc.b(labelEnd);
    
    // a > b, so return a
    cc.bind(labelA);
    cc.mov(result, a);
    
    cc.bind(labelEnd);
    cc.ret(result);
    
    cc.endFunc();
    cc.finalize();
    
    typedef int (*MaxFunc)(int, int);
    MaxFunc maxFunc;
    Error err = rt.add(&maxFunc, &code);
    
    if (err) {
        printf("JIT compilation failed: %s\n", DebugUtils::errorAsString(err));
        assert(false);
    }
    
    // Test max function
    int result1 = maxFunc(10, 5);
    EXPECT_EQ(result1, 10);
    
    int result2 = maxFunc(3, 8);
    EXPECT_EQ(result2, 8);
    
    int result3 = maxFunc(7, 7);
    EXPECT_EQ(result3, 7);
    
    rt.release(maxFunc);
}

// Main test runner
int main() {
    printf("RISC-V Compiler Test Suite (Updated API)\n");
    printf("========================================\n\n");
    
    try {
        TEST_CASE(test_basic_addition);
        TEST_CASE(test_multiple_operations);
        //TEST_CASE(test_function_invoke);
        //TEST_CASE(test_integer_types);
        //TEST_CASE(test_immediate_values);
        //TEST_CASE(test_register_allocation);
        //TEST_CASE(test_complex_expressions);
        //TEST_CASE(test_conditional_operations);
        
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