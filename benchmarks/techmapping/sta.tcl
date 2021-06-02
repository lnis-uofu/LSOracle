puts $::env(VERILOG_INPUT)
puts $::env(SDC_FILE)
read_liberty techmapping/sky130_fd_sc_hd__tt_025C_1v80.lib
read_verilog $::env(VERILOG_INPUT)
link_design $::env(VERILOG_TOP)
read_sdc $::env(SDC_FILE)
report_power
report_checks
report_worst_slack
