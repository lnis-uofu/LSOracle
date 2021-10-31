create_clock -name clk -period 10 {}
set_input_delay -clock clk 0 {
B[9]
B[8]
B[7]
B[6]
B[5]
B[4]
B[3]
B[2]
B[1]
B[10]
B[0]
}
set_output_delay -clock clk 0 {
M[3]
M[2]
M[1]
M[0]
E[2]
E[1]
E[0]
}
