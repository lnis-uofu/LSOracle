`timescale 1ns / 1ps
module adder(a,b,y);
parameter  WIDTH=8;
input [0:WIDTH-1] a,b;
output reg  [0:WIDTH-1] y;
always@(a or b)
	begin
y = a+b;

	end

endmodule
module multiplier(a,b,y);
parameter WIDTH=8;
input [0:WIDTH-1]a,b;
output [0:(WIDTH*2)-1]y;
reg [0:(WIDTH*2)-1]y;
always@(a or b)
	begin
		y=a*b;
	end

endmodule

module add_mul_mix_8_bit(a, b, c,d,Result);
      parameter WIDTH=8;
	input [0:WIDTH-1]a, b,c,d;
    
      wire [0:WIDTH-1]Result_add_2;
      wire [0:WIDTH-1]Result_add;
      output reg [0:(WIDTH*2)-1]Result;
adder adder_1 (.a(a), .b(b), .y(Result_add));

adder adder_2 (.a(c), .b(d), .y(Result_add_2));
multiplier multiplier_1 (.a(Result_add), .b(Result_add_2), .y(Result));
         
endmodule


