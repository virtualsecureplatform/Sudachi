module wrapper (input wire [511:0] in_img,
  output wire [123:0] out,
);
wire [127:0] temp;
__sobel_filter__sobel_filter_4 dut(
	.in_img(in_img),
	.out(temp)
);
assign out = {temp[126:96],temp[94:64],temp[62:32],temp[30:0]};
endmodule
