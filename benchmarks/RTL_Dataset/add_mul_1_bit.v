`timescale 1ns / 1ps
module adder(a,b,y);
input a,b;
output y;
reg y;
always@(a or b)
	begin
		y=a+b;
	end

endmodule
module multiplier(a,b,y);
input a,b;
output y;
reg y;
always@(a or b)
	begin
		y=a*b;
	end

endmodule

module add_mul_1_bit(a, b, operation, Result);
      input a, b;
      input operation;
      output Result;
      reg Result;
      wire Result_add;
      wire Result_mul;
adder adder_1 (.a(a), .b(b), .y(Result_add));
multiplier multiplier_1 (.a(a), .b(b), .y(Result_mul));
  //always@(a or b or operation)
       //  begin
         //      case (operation)
           //       1'b0:  Result = Result_add;   // Addition
             //     1'b1:  Result = Result_mul;   // multiply

               //endcase
          // end
          always@(*)
          begin
    Result = operation ? Result_mul : Result_add;
end
endmodule


