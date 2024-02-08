#!/bin/bash
CIRCUIT=riscv_simple
${XLSBIN}/xls/dslx/interpreter_main ./${CIRCUIT}.x
${XLSBIN}/xls/dslx/ir_convert/ir_converter_main --top=run_instruction ./${CIRCUIT}.x > ./${CIRCUIT}.ir
${XLSBIN}/xls/tools/opt_main ./${CIRCUIT}.ir > ./${CIRCUIT}.opt.ir
sed '/assert/d' ${CIRCUIT}.opt.ir > ${CIRCUIT}.opt.del.ir
${XLSBIN}/xls/tools/codegen_main --generator=combinational --delay_model=unit ./${CIRCUIT}.opt.del.ir > ./${CIRCUIT}.v
