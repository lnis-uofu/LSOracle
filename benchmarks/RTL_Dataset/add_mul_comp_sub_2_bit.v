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
module subtractor(a,b,y);
parameter  WIDTH=2;
input [0:WIDTH-1] a,b;
//output [0:WIDTH-1] y;
output reg  [0:(WIDTH*2)-1] y;
wire[0:WIDTH-1]g;
assign g=0;
always@(a or b)
	begin
y = a-b;

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
module comparator(A,B,A_greater_B,equal);  
 parameter WIDTH=2;
 input [0:WIDTH-1] A,B;
  output reg A_greater_B;
  output reg equal;
 // A greater than B output   
 always @(A or B)
 begin
 if (A>B)
 A_greater_B<= 1;
 else
 A_greater_B<= 0;
  if (A==B)
 equal<= 1;
 else
 equal<= 0;
 //assign A_greater_B = tmp6 | tmp7 | tmp8;  
 end
 endmodule 

module add_mul_comp_sub_2_bit(a, b,Result);
      parameter WIDTH=2;
	input [0:WIDTH-1]a, b;
wire [0:1]operation;

      output reg [0:(WIDTH*2)-1]Result;
       wire [0:(WIDTH*2)-1]Result_sub1;
      wire [0:(WIDTH*2)-1]Result_sub2;
  
      wire [0:(WIDTH*2)-1]Result_add;
      wire [0:(WIDTH*2)-1]Result_mul;
subtractor subtractor_1 (.a(a), .b(b), .y(Result_sub1));
comparator comparator_1(a,b, operation[0],operation[1]);  
subtractor subtractor_2 (.a(b), .b(a), .y(Result_sub2));
adder adder_1 (.a(a), .b(b), .y(Result_add));
multiplier multiplier_1 (.a(a), .b(b), .y(Result_mul));

          always@(*)

          begin
        case(operation)
        2'b00: // Addition
           Result=Result_add;

        2'b01: // sub1
           Result=Result_sub1;
        2'b10: // sub2
           Result=Result_sub2;

        2'b11: // mul
           Result=Result_mul;
endcase
   end
endmodule


