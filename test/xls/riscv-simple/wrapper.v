//remove R0 which is fixed to 0
module wrapper(in,out);
input wire [1215:0] in;
output wire [1151:0] out;
wire [1183:0] temp;

__riscv_simple__run_instruction dut(
.pc(in[31:0]),
.ins(in[63:32]),
.regs(in[1087:64]),
.dmem(in[1214:1088]),
.out(temp)
);
assign out = {temp[1183:160],temp[127:0]};
endmodule