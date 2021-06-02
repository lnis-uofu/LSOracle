create_clock -name clk -period 10 {clk}
set_input_delay -clock clk 0 {
rst_n
pcx_fpio_data_px2_79_72
pcx_fpio_data_px2_67_0
pcx_fpio_data_px2_123_118
pcx_fpio_data_px2_116_112
l15_fp_rdy
}
set_output_delay -clock clk 0 {
fpu_arb_grant
fpu_arb_data_rdy
fpu_arb_data
}
