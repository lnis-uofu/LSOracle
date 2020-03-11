// Benchmark "s27" written by ABC on Fri Feb  7 15:53:45 2020

module s27 ( 
    clock, a, b, c, d,
    f  );
  input  clock, a, b, c, d;
  output f;
  reg lo0, lo1, lo2;
  wire new_n16_, new_n17_, new_n18_, new_n19_1_, new_n20_, n14, n19, n24;
  assign new_n16_ = ~a & lo1;
  assign new_n17_ = ~d & ~new_n16_;
  assign new_n18_ = ~b & ~lo2;
  assign new_n19_1_ = ~new_n16_ & ~new_n18_;
  assign new_n20_ = ~new_n17_ & ~new_n19_1_;
  assign n19 = ~lo0 & new_n20_;
  assign n14 = a & ~n19;
  assign n24 = ~c & ~new_n18_;
  assign f = ~n19;
  always @ (negedge clock) begin
    lo0 <= n14;
    lo1 <= n19;
    lo2 <= n24;
  end
  initial begin
    lo0 <= 1'b1;
    lo1 <= 1'b1;
    lo2 <= 1'b1;
  end
endmodule


