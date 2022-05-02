create_clock -name clk -period 0
set_input_delay -clock clk -max 0 [all_inputs]
set_output_delay -clock clk -max 0 [all_outputs]
