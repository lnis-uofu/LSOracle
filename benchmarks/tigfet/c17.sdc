create_clock -name clk -period 10 {}
set_input_delay -clock clk 0 {
N7
N6
N3
N2
N1
}
set_output_delay -clock clk 0 {
N23
N22
}
