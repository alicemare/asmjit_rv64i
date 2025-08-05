// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

(function($scope, $as) {
"use strict";

function FAIL(msg) { throw new Error("[RISCV] " + msg); }

// Import
// ======

const base = $scope.base ? $scope.base : require("./base.js");
const exp = $scope.exp ? $scope.exp : require("./exp.js")

const hasOwn = Object.prototype.hasOwnProperty;
const dict = base.dict;
const NONE = base.NONE;
const Parsing = base.Parsing;
const MapUtils = base.MapUtils;

// Export
// ======

const riscv = $scope[$as] = dict();

// Database
// ========

riscv.dbName = "isa_riscv.json";

// asmdb.riscv.FieldInfo
// =====================

const FieldInfo = {
  "Xd"       : { "bits": 5, "read": false, "write": true  },
  "Xs1"      : { "bits": 5, "read": true , "write": false },
  "Xs2"      : { "bits": 5, "read": true , "write": false },
  "imm_i"    : { "bits": 12 },
  "imm_s_hi" : { "bits": 7 },
  "imm_s_lo" : { "bits": 5 },
  "imm_b_hi" : { "bits": 7 },
  "imm_b_lo" : { "bits": 5 },
  "imm_u"    : { "bits": 20 },
  "imm_j"    : { "bits": 20 },
  "shamt"    : { "bits": 5 }
};
riscv.FieldInfo = FieldInfo;

// asmdb.riscv.Utils
// =================

class Utils {
  static splitInstructionSignature(s) {
    const names = s.match(/^[\\w\\|]+/)[0];
    s = s.substring(names.length);

    const opOffset = s.indexOf(" ")
    const suffix = s.substring(0, opOffset).trim();
    const operands = opOffset === -1 ? "" : s.substring(opOffset + 1).trim();

    return {
      names: names.split("|").map((base)=>{ return base + suffix}),
      operands: operands
    }
  }
}
riscv.Utils = Utils;

// asmdb.riscv.Operand
// ===================

class Operand extends base.Operand {
  constructor(def) {
    super();
    this.data = def;
  }

  get name() {
    switch (this.type) {
      case "reg": return this.reg;
      case "mem": return this.mem;
      case "imm": return this.imm;
      case "rel": return this.rel;
      default   : return "";
    }
  }
}
riscv.Operand = Operand;

// asmdb.riscv.Instruction
// =======================

class Instruction extends base.Instruction {
  constructor(db, data) {
    super(db, data);

    this.name = data.inst.split(" ")[0];
    this._assignOperands(data.inst.substring(this.name.length).trim());
    this._assignOpcode(data.op);

    for (let k in data) {
      if (k === "inst" || k === "op")
        continue;
      this._assignAttribute(k, data[k]);
    }

    this._postProcess();
  }

  _assignOpcode(s) {
    if (!s) return;

    const parts = s.split("|");
    for (const part of parts) {
      if (/^[01]+$/.test(part)) {
        this.opcode.push(new base.OpcodeBit(part));
      }
      else {
        if (!hasOwn.call(riscv.FieldInfo, part))
          FAIL(`Unknown field '\${part}' in opcode '\${s}'`);
        this.opcode.push(new base.OpcodeField(part, riscv.FieldInfo[part].bits));
      }
    }
  }
}
riscv.Instruction = Instruction;

// asmdb.riscv.ISA
// ===============

class ISA extends base.ISA {
  constructor() {
    super("riscv", riscv.Instruction, riscv.Operand);
  }

  addFromJSON(json) {
    const instructions = json.instructions;
    if (!instructions)
      return;

    for (const group of instructions) {
      const groupData = group.data;
      for (const instData of groupData) {
        this.addInstruction(instData.inst, instData.op, instData.io, instData.ext);
      }
    }
  }
}
riscv.ISA = ISA;

}).apply(this, typeof module === "object" && module.exports ? [module, "exports"] : [window, "asmdb.riscv"]);