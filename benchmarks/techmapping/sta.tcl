puts $::env(VERILOG_INPUT)
puts $::env(SDC_FILE)
read_liberty $::env(LIBERTY_FILE)
read_verilog $::env(VERILOG_INPUT)
link_design $::env(VERILOG_TOP)
read_sdc $::env(SDC_FILE)
report_power
report_checks
report_worst_slack
