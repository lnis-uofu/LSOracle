yosys -import
read_verilog $::env(VERILOG_INPUT).v
hierarchy -check -top $::env(VERILOG_INPUT)
procs
memory
flatten
fsm
techmap
dffunmap
abc -g AND
write_aiger $::env(VERILOG_INPUT).aig
