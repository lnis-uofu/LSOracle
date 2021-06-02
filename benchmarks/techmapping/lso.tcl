yosys -import
read_rtlil $::env(VERILOG_INPUT).rtl
lsoracle
opt -purge
write_rtlil $::env(VERILOG_INPUT)_lsoracle.rtl
