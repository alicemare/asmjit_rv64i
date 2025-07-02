// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>
#include <cstdint>
#if !defined(ASMJIT_NO_COMPILER) && !defined(ASMJIT_NO_RISCV)

#include <asmjit/riscv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./asmjit_test_compiler.h"

using namespace asmjit;

// riscv::Compiler - RISCVTestCase
// ===============================

class RISCVTestCase : public TestCase {
public:
  RISCVTestCase(const char* name = nullptr)
    : TestCase(name, Arch::kRISCV64) {}

  virtual void compile(BaseCompiler& cc) override {
    compile(static_cast<riscv::Compiler&>(cc));
  }

  virtual void compile(riscv::Compiler& cc) = 0;
};

// riscv::Compiler - RISCVTest_GpArgs
// ==============================

class RISCVTest_GpArgs : public RISCVTestCase {
public:
  uint32_t _argCount;
  bool _preserveFP;

  RISCVTest_GpArgs(uint32_t argCount, bool preserveFP)
    : _argCount(argCount),
      _preserveFP(preserveFP) {
    _name.assignFormat("GpArgs {NumArgs=%u PreserveFP=%c}", argCount, preserveFP ? 'Y' : 'N');
  }

  static void add(TestApp& app) {
    for (uint32_t i = 0; i <= 16; i++) {
     // app.add(new RISCVTest_GpArgs(i, true));
      app.add(new RISCVTest_GpArgs(i, false));
    }
  }

  virtual void compile(riscv::Compiler& cc) {
    uint32_t i;
    uint32_t argCount = _argCount;

    FuncSignature signature;
    signature.setRetT<int>();
    for (i = 0; i < argCount; i++)
      signature.addArgT<int>();

    FuncNode* funcNode = cc.addFunc(signature);
    if (_preserveFP)
      funcNode->frame().setPreservedFP();

    riscv::Gp sum;

    if (argCount) {
      for (i = 0; i < argCount; i++) {
        riscv::Gp iReg = cc.newInt32("i%u", i);
        funcNode->setArg(i, iReg);

        if (i == 0)
          sum = iReg;
        else
          cc.add(sum, sum, iReg);
      }
    }
    else {
      sum = cc.newInt32("i");
      cc.mov(sum, 0);
    }

    cc.ret(sum);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    using U = unsigned int;

    using Func0 = U (*)();
    using Func1 = U (*)(U);
    using Func2 = U (*)(U, U);
    using Func3 = U (*)(U, U, U);
    using Func4 = U (*)(U, U, U, U);
    using Func5 = U (*)(U, U, U, U, U);
    using Func6 = U (*)(U, U, U, U, U, U);
    using Func7 = U (*)(U, U, U, U, U, U, U);
    using Func8 = U (*)(U, U, U, U, U, U, U, U);
    using Func9 = U (*)(U, U, U, U, U, U, U, U, U);
    using Func10 = U (*)(U, U, U, U, U, U, U, U, U, U);
    using Func11 = U (*)(U, U, U, U, U, U, U, U, U, U, U);
    using Func12 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U);
    using Func13 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U, U);
    using Func14 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U, U, U);
    using Func15 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U, U, U, U);
    using Func16 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U);

    unsigned int resultRet = 0;
    unsigned int expectRet = 0;

    switch (_argCount) {
      case 0:
        resultRet = ptr_as_func<Func0>(_func)();
        expectRet = 0;
        break;
      case 1:
        resultRet = ptr_as_func<Func1>(_func)(1);
        expectRet = 1;
        break;
      case 2:
        resultRet = ptr_as_func<Func2>(_func)(1, 2);
        expectRet = 1 + 2;
        break;
      case 3:
        resultRet = ptr_as_func<Func3>(_func)(1, 2, 3);
        expectRet = 1 + 2 + 3;
        break;
      case 4:
        resultRet = ptr_as_func<Func4>(_func)(1, 2, 3, 4);
        expectRet = 1 + 2 + 3 + 4;
        break;
      case 5:
        resultRet = ptr_as_func<Func5>(_func)(1, 2, 3, 4, 5);
        expectRet = 1 + 2 + 3 + 4 + 5;
        break;
      case 6:
        resultRet = ptr_as_func<Func6>(_func)(1, 2, 3, 4, 5, 6);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6;
        break;
      case 7:
        resultRet = ptr_as_func<Func7>(_func)(1, 2, 3, 4, 5, 6, 7);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7;
        break;
      case 8:
        resultRet = ptr_as_func<Func8>(_func)(1, 2, 3, 4, 5, 6, 7, 8);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8;
        break;
      case 9:
        resultRet = ptr_as_func<Func9>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;
        break;
      case 10:
        resultRet = ptr_as_func<Func10>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10;
        break;
      case 11:
        resultRet = ptr_as_func<Func11>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
        break;
      case 12:
        resultRet = ptr_as_func<Func12>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12;
        break;
      case 13:
        resultRet = ptr_as_func<Func13>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13;
        break;
      case 14:
        resultRet = ptr_as_func<Func14>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14;
        break;
      case 15:
        resultRet = ptr_as_func<Func15>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15;
        break;
      case 16:
        resultRet = ptr_as_func<Func16>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15 + 16;
        break;
    }

    result.assignFormat("ret={%u, %u}", resultRet >> 28, resultRet & 0x0FFFFFFFu);
    expect.assignFormat("ret={%u, %u}", expectRet >> 28, expectRet & 0x0FFFFFFFu);

    return result == expect;
  }
};

// riscv::Compiler - RISCVTest_Add
// ================================
class RISCVTest_Add : public RISCVTestCase {
public:
  RISCVTest_Add()
    : RISCVTestCase("Add") {}

  static void add(TestApp& app) {
    app.add(new RISCVTest_Add());
  }

  virtual void compile(riscv::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, void*, const void*, const void*>());

    riscv::Gp dst = cc.newUIntPtr("dst");
    riscv::Gp src1 = cc.newUIntPtr("src1");
    riscv::Gp src2 = cc.newUIntPtr("src2");

    funcNode->setArg(0, dst);
    funcNode->setArg(1, src1);
    funcNode->setArg(2, src2);

    riscv::Gp v1 = cc.newUInt32("val1");
    riscv::Gp v2 = cc.newUInt32("val2");
    riscv::Gp v3 = cc.newUInt32("val3");

    cc.lw(v1, riscv::Mem::ptr(src1));
    cc.lw(v2, riscv::Mem::ptr(src2));
    cc.add(v3, v1, v2);
    cc.sw(v3, riscv::Mem::ptr(dst));

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    using Func = void (*)(void*, const void*, const void*);

    uint32_t dst;
    uint32_t aSrc = 1;
    uint32_t bSrc = 99;

    uint32_t ref = 100;

    ptr_as_func<Func>(_func)(&dst, &aSrc, &bSrc);

    result.assignFormat("ret={%u}", dst);
    expect.assignFormat("ret={%u}", ref);

    return result == expect;
  }
};

// riscv::Compiler - RISCVTest_ManyRegs
// ================================

class RISCVTest_ManyRegs : public RISCVTestCase {
public:
  uint32_t _regCount;

  RISCVTest_ManyRegs(uint32_t n)
    : RISCVTestCase(),
      _regCount(n) {
    _name.assignFormat("GpRegs {NumRegs=%u}", n);
  }

  static void add(TestApp& app) {
    for (uint32_t i = 2; i < 64; i++)
      app.add(new RISCVTest_ManyRegs(i));
  }

  virtual void compile(riscv::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    riscv::Gp* regs = static_cast<riscv::Gp*>(malloc(_regCount * sizeof(riscv::Gp)));

    for (uint32_t i = 0; i < _regCount; i++) {
      regs[i] = cc.newUInt32("reg%u", i);
      cc.mov(regs[i], i + 1);
    }

    riscv::Gp sum = cc.newUInt32("sum");
    cc.mov(sum, 0);

    for (uint32_t i = 0; i < _regCount; i++) {
      cc.add(sum, sum, regs[i]);
    }

    cc.ret(sum);
    cc.endFunc();

    free(regs);
  }

  virtual bool run(void* _func, String& result, String& expect) {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    result.assignFormat("ret={%d}", func());
    expect.assignFormat("ret={%d}", calcSum());

    return result == expect;
  }

  uint32_t calcSum() const {
    return (_regCount | 1) * ((_regCount + 1) / 2);
  }
};

// riscv::Compiler - RISCVTest_Adr
// ===========================

class RISCVTest_Adr : public RISCVTestCase {
public:
  RISCVTest_Adr()
    : RISCVTestCase("Adr") {}

  static void add(TestApp& app) {
    app.add(new RISCVTest_Adr());
  }

  virtual void compile(riscv::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    riscv::Gp addr = cc.newIntPtr("addr");
    riscv::Gp val = cc.newIntPtr("val");

    Label L_Table = cc.newLabel();

    cc.adr(addr, L_Table);
    cc.lw(val, riscv::Mem::ptr(addr, 8));
    cc.ret(val);
    cc.endFunc();

    cc.bind(L_Table);
    cc.embedInt32(1);
    cc.embedInt32(2);
    cc.embedInt32(3);
    cc.embedInt32(4);
    cc.embedInt32(5);
  }

  virtual bool run(void* _func, String& result, String& expect) {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    result.assignFormat("ret={%d}", func());
    expect.assignFormat("ret={%d}", 3);

    return result == expect;
  }
};

// riscv::Compiler - RISCVTest_Branch1
// ===============================

class RISCVTest_Branch1 : public RISCVTestCase {
public:
  RISCVTest_Branch1()
    : RISCVTestCase("Branch1") {}

  static void add(TestApp& app) {
    app.add(new RISCVTest_Branch1());
  }

  virtual void compile(riscv::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, void*, size_t>());

    riscv::Gp p = cc.newIntPtr("p");
    riscv::Gp count = cc.newIntPtr("count");
    riscv::Gp i = cc.newIntPtr("i");
    Label L = cc.newLabel();

    funcNode->setArg(0, p);
    funcNode->setArg(1, count);

    cc.mov(i, 0);

    cc.bind(L);
    cc.sb(i, riscv::Mem::ptr(p, 0));
    cc.addi(i, i, 1);
    cc.bne(i, count, L);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    using Func = void (*)(void* p, size_t n);
    Func func = ptr_as_func<Func>(_func);

    uint8_t array[16];
    func(array, 16);

    expect.assign("ret={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}");

    result.assign("ret={");
    for (size_t i = 0; i < 16; i++) {
      if (i)
        result.append(", ");
      result.appendFormat("%d", int(array[i]));
    }
    result.append("}");

    return result == expect;
  }
};

// riscv::Compiler - RISCVTest_Invoke1
// ===============================

class RISCVTest_Invoke1 : public RISCVTestCase {
public:
  RISCVTest_Invoke1()
    : RISCVTestCase("Invoke1") {}

  static void add(TestApp& app) {
    app.add(new RISCVTest_Invoke1());
  }

  virtual void compile(riscv::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<uint32_t, uint32_t, uint32_t>());

    riscv::Gp x = cc.newUInt32("x");
    riscv::Gp y = cc.newUInt32("y");
    riscv::Gp r = cc.newUInt32("r");
    riscv::Gp fn = cc.newUIntPtr("fn");

    funcNode->setArg(0, x);
    funcNode->setArg(1, y);

    // cc.mov(fn, (uint64_t)calledFunc);
    // 不要直接mov立即数，使用常量池
    riscv::Mem constAddr = cc.newConst(ConstPoolScope::kLocal, (void*)calledFunc, sizeof(void*));
    cc.ld(fn, constAddr);

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, fn, FuncSignature::build<uint32_t, uint32_t, uint32_t>());
    invokeNode->setArg(0, x);
    invokeNode->setArg(1, y);
    invokeNode->setRet(0, r);

    cc.ret(r);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    using Func = uint32_t (*)(uint32_t, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t x = 49;
    uint32_t y = 7;

    result.assignFormat("ret={%u}", func(x, y));
    expect.assignFormat("ret={%u}", x - y);

    return result == expect;
  }

  static uint32_t calledFunc(uint32_t x, uint32_t y) {
    return x - y;
  }
};

// riscv::Compiler - RISCVTest_Invoke2
// ===============================

class RISCVTest_Invoke2 : public RISCVTestCase {
public:
  RISCVTest_Invoke2()
    : RISCVTestCase("Invoke2") {}

  static void add(TestApp& app) {
    app.add(new RISCVTest_Invoke2());
  }

  virtual void compile(riscv::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int64_t, int64_t, int64_t>());

    riscv::Gp x = cc.newInt64("x");
    riscv::Gp y = cc.newInt64("y");
    riscv::Gp r = cc.newInt64("r");
    riscv::Gp fn = cc.newUIntPtr("fn");

    funcNode->setArg(0, x);
    funcNode->setArg(1, y);
    printf("calledFunc address: %p\n", (void*)calledFunc);
    riscv::Mem funcPtr = cc.newConst(ConstPoolScope::kLocal, (void*)calledFunc, 8);
    cc.ld(fn, funcPtr);
    //cc.mov(fn, (uint64_t)calledFunc);

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, fn, FuncSignature::build<int64_t, int64_t, int64_t>());
    invokeNode->setArg(0, x);
    invokeNode->setArg(1, y);
    invokeNode->setRet(0, r);

    cc.ret(r);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    using Func = int64_t (*)(int64_t, int64_t);
    Func func = ptr_as_func<Func>(_func);

    int64_t x = 49;
    int64_t y = 7;

    result.assignFormat("ret={%ld}", func(x, y));
    expect.assignFormat("ret={%ld}", calledFunc(x, y));

    return result == expect;
  }

  static int64_t calledFunc(int64_t x, int64_t y) {
    return x - y;
  }
};

// riscv::Compiler - RISCVTest_Invoke3
// ===============================

class RISCVTest_Invoke3 : public RISCVTestCase {
public:
  RISCVTest_Invoke3()
    : RISCVTestCase("Invoke3") {}

  static void add(TestApp& app) {
    app.add(new RISCVTest_Invoke3());
  }

  virtual void compile(riscv::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int64_t, int64_t, int64_t>());

    riscv::Gp x = cc.newInt64("x");
    riscv::Gp y = cc.newInt64("y");
    riscv::Gp r = cc.newInt64("r");
    riscv::Gp fn = cc.newUIntPtr("fn");

    funcNode->setArg(0, x);
    funcNode->setArg(1, y);
    cc.mov(fn, (uint64_t)calledFunc);

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, fn, FuncSignature::build<int64_t, int64_t, int64_t>());
    invokeNode->setArg(0, y);
    invokeNode->setArg(1, x);
    invokeNode->setRet(0, r);

    cc.ret(r);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    using Func = int64_t (*)(int64_t, int64_t);
    Func func = ptr_as_func<Func>(_func);

    int64_t x = 49;
    int64_t y = 7;

    result.assignFormat("ret={%ld}", func(x, y));
    expect.assignFormat("ret={%ld}", calledFunc(y, x));

    return result == expect;
  }

  static int64_t calledFunc(int64_t x, int64_t y) {
    return x - y;
  }
};

// riscv::Compiler - RISCVTest_JumpTable
// =================================
/*
class RISCVTest_JumpTable : public RISCVTestCase {
public:
  bool _annotated;

  RISCVTest_JumpTable(bool annotated)
    : RISCVTestCase("RISCVTest_JumpTable"),
      _annotated(annotated) {
    _name.assignFormat("JumpTable {%s}", annotated ? "Annotated" : "Unknown Target");
  }

  enum Operator {
    kOperatorAdd = 0,
    kOperatorSub = 1,
    kOperatorMul = 2,
    kOperatorDiv = 3
  };

  static void add(TestApp& app) {
    app.add(new RISCVTest_JumpTable(false));
    app.add(new RISCVTest_JumpTable(true));
  }

  virtual void compile(riscv::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<float, float, float, uint32_t>());

    riscv::Gp a = cc.newInt64("a");
    riscv::Gp b = cc.newInt64("b");
    riscv::Gp op = cc.newUInt32("op");

    riscv::Gp target = cc.newIntPtr("target");
    riscv::Gp offset = cc.newIntPtr("offset");

    Label L_End = cc.newLabel();

    Label L_Table = cc.newLabel();
    Label L_Add = cc.newLabel();
    Label L_Sub = cc.newLabel();
    Label L_Mul = cc.newLabel();
    Label L_Div = cc.newLabel();

    funcNode->setArg(0, a);
    funcNode->setArg(1, b);
    funcNode->setArg(2, op);

    cc.adr(target, L_Table);
    cc.ldrsw(offset, riscv::ptr(target, op, riscv::sxtw(2)));
    cc.add(target, target, offset);

    // JumpAnnotation allows to annotate all possible jump targets of
    // instructions where it cannot be deduced from operands.
    if (_annotated) {
      JumpAnnotation* annotation = cc.newJumpAnnotation();
      annotation->addLabel(L_Add);
      annotation->addLabel(L_Sub);
      annotation->addLabel(L_Mul);
      annotation->addLabel(L_Div);
      cc.br(target, annotation);
    }
    else {
      cc.br(target);
    }

    cc.bind(L_Add);
    cc.fadd(a, a, b);
    cc.b(L_End);

    cc.bind(L_Sub);
    cc.fsub(a, a, b);
    cc.b(L_End);

    cc.bind(L_Mul);
    cc.fmul(a, a, b);
    cc.b(L_End);

    cc.bind(L_Div);
    cc.fdiv(a, a, b);

    cc.bind(L_End);
    cc.ret(a);
    cc.endFunc();

    cc.bind(L_Table);
    cc.embedLabelDelta(L_Add, L_Table, 4);
    cc.embedLabelDelta(L_Sub, L_Table, 4);
    cc.embedLabelDelta(L_Mul, L_Table, 4);
    cc.embedLabelDelta(L_Div, L_Table, 4);
  }

  virtual bool run(void* _func, String& result, String& expect) {
    using Func = float (*)(float, float, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    float dst[4];
    float ref[4];

    dst[0] = func(33.0f, 14.0f, kOperatorAdd);
    dst[1] = func(33.0f, 14.0f, kOperatorSub);
    dst[2] = func(10.0f, 6.0f, kOperatorMul);
    dst[3] = func(80.0f, 8.0f, kOperatorDiv);

    ref[0] = 47.0f;
    ref[1] = 19.0f;
    ref[2] = 60.0f;
    ref[3] = 10.0f;

    result.assignFormat("ret={%f, %f, %f, %f}", dst[0], dst[1], dst[2], dst[3]);
    expect.assignFormat("ret={%f, %f, %f, %f}", ref[0], ref[1], ref[2], ref[3]);

    return result == expect;
  }
};
*/
// riscv::Compiler - Export
// ======================

void compiler_add_riscv_tests(TestApp& app) {
  app.addT<RISCVTest_GpArgs>();
  app.addT<RISCVTest_ManyRegs>();
  app.addT<RISCVTest_Add>();
  app.addT<RISCVTest_Adr>();
  app.addT<RISCVTest_Branch1>();
  app.addT<RISCVTest_Invoke1>();
  app.addT<RISCVTest_Invoke2>();
  app.addT<RISCVTest_Invoke3>();
  //app.addT<RISCVTest_JumpTable>();
}

#endif // !ASMJIT_NO_COMPILER && !ASMJIT_NO_AARCH64
