create_clock -name clk -period 10 {wb_clk_i}
set_input_delay -clock clk 0 {
wbm_dat_i
wbm_ack_i
wb_rst_i
pcpi_wr
pcpi_wait
pcpi_ready
pcpi_rd
irq
}
set_output_delay -clock clk 0 {
wbm_we_o
wbm_stb_o
wbm_sel_o
wbm_dat_o
wbm_cyc_o
wbm_adr_o
trap
trace_valid
trace_data
pcpi_valid
pcpi_rs2
pcpi_rs1
pcpi_insn
mem_instr
eoi
}
