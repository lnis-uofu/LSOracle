// Benchmark "s27" written by ABC on Wed Feb 12 15:28:47 2020

module s27 ( 
    clock, a, b, c, d,
    f  );
  input  clock, a, b, c, d;
  output f;
  reg lo0, lo1, lo2;
  wire new_new_new_n15___, new_new_new_n16___, new_new_new_n17___,
    new_new_new_n18___, new_new_new_n19___, n14, n19, n24;
  assign n14 = ~n19 & a;
  assign n19 = ~lo0 & new_new_new_n19___;
  assign new_new_new_n15___ = ~a & lo1;
  assign new_new_new_n16___ = ~d & ~new_new_new_n15___;
  assign new_new_new_n17___ = ~b & ~lo2;
  assign new_new_new_n18___ = ~new_new_new_n15___ & ~new_new_new_n17___;
  assign new_new_new_n19___ = ~new_new_new_n16___ & ~new_new_new_n18___;
  assign n24 = ~c & ~new_new_new_n17___;
  assign f = ~n19;
  always @ (posedge clock) begin
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


