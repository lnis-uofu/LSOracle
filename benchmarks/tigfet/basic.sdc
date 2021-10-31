create_clock -name clk -period 10 {}
set_input_delay -clock clk 0 {*}
set_output_delay -clock clk {*}
