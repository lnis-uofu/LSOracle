create_clock -name clk -period 10 {CK}
set_input_delay -clock clk 0 {
G2
G1
G0
}
set_output_delay -clock clk 0 {
G67
G66
G133
G132
G118
G117
}
