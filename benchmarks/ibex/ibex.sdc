create_clock -name clk -period 10 {clk_i}
set_input_delay -clock clk 0 {
test_en_i
rst_ni
irq_timer_i
irq_software_i
irq_nm_i
irq_fast_i
irq_external_i
instr_rvalid_i
instr_rdata_i
instr_gnt_i
instr_err_i
hart_id_i
fetch_enable_i
debug_req_i
data_rvalid_i
data_rdata_i
data_gnt_i
data_err_i
boot_addr_i
}
set_output_delay -clock clk 0 {
instr_req_o
instr_addr_o
data_we_o
data_wdata_o
data_req_o
data_be_o
data_addr_o
core_sleep_o
alert_minor_o
alert_major_o
}
