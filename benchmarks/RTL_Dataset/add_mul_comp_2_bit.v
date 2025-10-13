`timescale 1ns / 1ps
module adder(a,b,y);
parameter  WIDTH=2;
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
parameter WIDTH=2;
input [0:WIDTH-1]a,b;
output [0:(WIDTH*2)-1]y;
reg [0:(WIDTH*2)-1]y;
always@(a or b)
	begin
		y=a*b;
	end

endmodule
module comparator(A,B,A_greater_B);  
 parameter WIDTH=2;
 input [0:WIDTH-1] A,B;
  output reg A_greater_B;

 // A greater than B output   
 always @(A or B)
 begin
 if (A>B)
 A_greater_B<= 1;
 else
 A_greater_B<= 0;
 //assign A_greater_B = tmp6 | tmp7 | tmp8;  
 end
 endmodule 

module add_mul_comp_2_bit(a, b, Result);
      parameter WIDTH=2;
	input [0:WIDTH-1]a, b;

      output reg [0:(WIDTH*2)-1]Result;
      wire A_greater_B;
      wire [0:(WIDTH*2)-1]Result_add;
      wire [0:(WIDTH*2)-1]Result_mul;
adder adder_1 (.a(a), .b(b), .y(Result_add));
multiplier multiplier_1 (.a(a), .b(b), .y(Result_mul));
comparator comparator_1(a,b, A_greater_B);  
          always@(*)
          begin
    Result = A_greater_B ? Result_mul : Result_add;
    
end
endmodule


