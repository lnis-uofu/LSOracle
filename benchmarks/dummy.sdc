create_clock -period 0 -name virtual_io_clock
set_input_delay -clock virtual_io_clock -max 0 [all_inputs]
set_output_delay -clock virtual_io_clock -max 0 [all_outputs]
