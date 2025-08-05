// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

"use strict";

const core = require("./tablegen.js");
const commons = require("./generator-commons.js");
const hasOwn = Object.prototype.hasOwnProperty;

const asmdb = core.asmdb;
const kIndent = commons.kIndent;
const IndexedArray = commons.IndexedArray;
const StringUtils = commons.StringUtils;

const FATAL = commons.FATAL;

// ============================================================================
// [RiscVDB]
// ============================================================================

// Create RISC-V ISA.
const isa = new asmdb.riscv.ISA();

// ============================================================================
// [tablegen.riscv.RiscVTableGen]
// ============================================================================

class RiscVTableGen extends core.TableGen {
  constructor() {
    super("RiscV");
  }

  parse() {
    const json = JSON.parse(this.dataOfFile("db/isa_riscv.json"));
    isa.addFromJSON(json);

    for (const inst of isa.instructionList) {
      this.addInstruction(inst);
    }
  }

  onBeforeRun() {
    this.load(["db/isa_riscv.json"]);
    this.parse();
  }

  onAfterRun() {
    this.merge();
    this.save();
    this.dumpTableSizes();
  }
}

// ============================================================================
// [tablegen.riscv.IdEnum]
// ============================================================================

class IdEnum extends core.IdEnum {
  constructor() {
    super("IdEnum");
  }

  comment(inst) {
    let name = inst.name;
    let ext = [];

    // Handle RISC-V instruction extensions
    if (name.endsWith("w")) {
      ext.push("RV64I");
    } else if (name.includes("fence")) {
      ext.push("Zifencei");
    } else if (name === "ecall" || name === "ebreak") {
      ext.push("Zicsr");
    }

    let exts = "";
    if (ext.length)
      exts = " {" + ext.join("&") + "}";

    return `Instruction '${name}'${exts}.`;
  }
}

// ============================================================================
// [tablegen.riscv.NameTable]
// ============================================================================

class NameTable extends core.NameTable {
  constructor() {
    super("NameTable");
  }
}

// ============================================================================
// [tablegen.riscv.EncodingTable]
// ============================================================================

class EncodingTable extends core.Task {
  constructor() {
    super("EncodingTable");
  }

  run() {
    const insts = this.ctx.insts;
    const map = {};

    for (var i = 0; i < insts.length; i++) {
      const inst = insts[i];

      const encoding = inst.encoding;
      const opcodeData = inst.opcodeData.replace(/\(/g, "{ ").replace(/\)/g, " }");

      if (!hasOwn.call(map, encoding))
        map[encoding] = [];

      if (inst.opcodeData === "(_)") {
        inst.opcodeDataIndex = 0;
        continue;
      }

      const opcodeTable = map[encoding];
      const opcodeDataIndex = opcodeTable.length;

      opcodeTable.push({ name: inst.name, data: opcodeData });
      inst.opcodeDataIndex = opcodeDataIndex;
    }

    const keys = Object.keys(map);
    keys.sort();

    var tableSource = "";
    var tableHeader = "";
    var encodingIds = "";

    encodingIds += "enum EncodingId : uint32_t {\n"
    encodingIds += "  kEncodingNone = 0";

    keys.forEach((dataClass) => {
      const dataName = dataClass[0].toLowerCase() + dataClass.substr(1);
      const opcodeTable = map[dataClass];
      const count = opcodeTable.length;

      if (dataClass !== "None") {
        encodingIds += ",\n"
        encodingIds += "  kEncoding" + dataClass;
      }

      if (count) {
        tableHeader += `extern const ${dataClass}Type ${dataName}[${count}];\n`;

        if (tableSource)
          tableSource += "\n";

        tableSource += `const ${dataClass}Type ${dataName}[${count}] = {\n`;
        for (var i = 0; i < count; i++) {
          tableSource += `  ${opcodeTable[i].data}` + (i == count - 1 ? " " : ",") + " // " + opcodeTable[i].name + "\n";
        }
        tableSource += `};\n`;
      }
    });

    encodingIds += "\n};\n";

    return this.ctx.inject("EncodingId"         , StringUtils.disclaimer(encodingIds), 0) +
           this.ctx.inject("EncodingDataForward", StringUtils.disclaimer(tableHeader), 0) +
           this.ctx.inject("EncodingData"       , StringUtils.disclaimer(tableSource), 0);
  }
}

// ============================================================================
// [tablegen.riscv.CommonTable]
// ============================================================================

class CommonTable extends core.Task {
  constructor() {
    super("CommonTable", [
      "IdEnum",
      "NameTable"
    ]);
  }

  run() {
    // Common table implementation for RISC-V
    // This can be extended to include instruction metadata
    return 0;
  }
}

// ============================================================================
// [Main]
// ============================================================================

new RiscVTableGen()
  .addTask(new IdEnum())
  .addTask(new NameTable())
  .addTask(new EncodingTable())
  .addTask(new CommonTable())
  .run();