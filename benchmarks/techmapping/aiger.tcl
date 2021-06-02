yosys -import
read_rtlil $::env(VERILOG_INPUT).rtl
dffunmap
aigmap
write_aiger $::env(VERILOG_INPUT).aig
