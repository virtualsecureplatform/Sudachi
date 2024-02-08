module __crc32__main(
  input wire [7:0] message,
  output wire [31:0] out
);
  wire [7:0] U32_MAX__3;
  wire [31:0] mask;
  wire [31:0] xor_297;
  wire [31:0] mask__1;
  wire [31:0] xor_305;
  wire [31:0] mask__2;
  wire [31:0] xor_313;
  wire [31:0] mask__3;
  wire [31:0] xor_321;
  wire [31:0] mask__4;
  wire [31:0] xor_329;
  wire [31:0] mask__5;
  wire [31:0] xor_337;
  wire [31:0] mask__6;
  wire [31:0] xor_345;
  wire [31:0] mask__7;
  assign U32_MAX__3 = ~message;
  assign mask = {32{U32_MAX__3[0]}};
  assign xor_297 = {25'h0ff_ffff, U32_MAX__3[7:1]} ^ mask & 32'hedb8_8320;
  assign mask__1 = {32{xor_297[0]}};
  assign xor_305 = {1'h0, xor_297[31:1]} ^ mask__1 & 32'hedb8_8320;
  assign mask__2 = {32{xor_305[0]}};
  assign xor_313 = {1'h0, xor_305[31:1]} ^ mask__2 & 32'hedb8_8320;
  assign mask__3 = {32{xor_313[0]}};
  assign xor_321 = {1'h0, xor_313[31:1]} ^ mask__3 & 32'hedb8_8320;
  assign mask__4 = {32{xor_321[0]}};
  assign xor_329 = {1'h0, xor_321[31:1]} ^ mask__4 & 32'hedb8_8320;
  assign mask__5 = {32{xor_329[0]}};
  assign xor_337 = {1'h0, xor_329[31:1]} ^ mask__5 & 32'hedb8_8320;
  assign mask__6 = {32{xor_337[0]}};
  assign xor_345 = {1'h0, xor_337[31:1]} ^ mask__6 & 32'hedb8_8320;
  assign mask__7 = {32{xor_345[0]}};
  assign out = ~({1'h0, xor_345[31:1]} ^ mask__7 & 32'hedb8_8320);
endmodule
