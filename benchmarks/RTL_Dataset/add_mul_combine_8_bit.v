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

module add_mul_combine_8_bit(a, b, Result_mul,Result_add);
      parameter WIDTH=8;
	input [0:WIDTH-1]a, b;
      
      output reg [0:WIDTH-1]Result_add;
      output reg [0:(WIDTH*2)-1]Result_mul;
adder adder_1 (.a(a), .b(b), .y(Result_add));
multiplier multiplier_1 (.a(a), .b(b), .y(Result_mul));
         
endmodule


