`timescale 1ns / 1ps
module adder(a,b,y);
parameter  WIDTH=16;
input [0:WIDTH-1] a,b;
//output [0:WIDTH-1] y;
output reg  [0:(WIDTH*2)-1] y;
wire[0:WIDTH-1]g;
assign g=0;
always@(a or b)
	begin
y = {g, (a + b)};

	end

endmodule
module multiplier(a,b,y);
parameter WIDTH=16;
input [0:WIDTH-1]a,b;
output [0:(WIDTH*2)-1]y;
reg [0:(WIDTH*2)-1]y;
always@(a or b)
	begin
		y=a*b;
	end

endmodule

module add_mul_16_bit(a, b, operation, Result);
      parameter WIDTH=16;
	input [0:WIDTH-1]a, b;
      input operation;
      output reg [0:(WIDTH*2)-1]Result;
      wire [0:(WIDTH*2)-1]Result_add;
      wire [0:(WIDTH*2)-1]Result_mul;
adder adder_1 (.a(a), .b(b), .y(Result_add));
multiplier multiplier_1 (.a(a), .b(b), .y(Result_mul));
          always@(*)
          begin
    Result = operation ? Result_mul : Result_add;
end
endmodule


