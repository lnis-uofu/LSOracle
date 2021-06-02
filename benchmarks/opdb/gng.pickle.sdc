create_clock -name clk -period 10 {clk}
set_input_delay -clock clk 0 {
rstn
ce
}
set_output_delay -clock clk 0 {
valid_out
data_out
}
