create_clock -name clk -period 10 {chip_clk intcnct_clk}
set_input_delay -clock clk 0 {
rst_n
network_out_3
network_out_2
network_out_1
intcnct_data_in
intcnct_credit_back_out
intcnct_channel_in
data_out_val_3
data_out_val_2
data_out_val_1
data_in_rdy_3
data_in_rdy_2
data_in_rdy_1
async_mux
}
set_output_delay -clock clk 0 {
network_in_3
network_in_2
network_in_1
intcnct_data_out
intcnct_credit_back_in
intcnct_channel_out
data_out_rdy_3
data_out_rdy_2
data_out_rdy_1
data_in_val_3
data_in_val_2
data_in_val_1
}
