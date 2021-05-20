puts $::env(VERILOG_INPUT)

yosys -import
read_verilog -defer -sv $::env(VERILOG_INPUT).v
read_liberty -lib techmapping/sky130_fd_sc_hd__tt_025C_1v80.lib
read_verilog -defer techmapping/cells_clkgate_hd.v
hierarchy -check -top $::env(VERILOG_TOP)
procs
flatten
fsm
techmap
#synth -top c17 -flatten
#opt -purge
techmap -map techmapping/cells_latch_hd.v
dfflibmap -liberty techmapping/sky130_fd_sc_hd__tt_025C_1v80.lib
opt
abc -D 10 \
    -script techmapping/abc.script \
    -liberty techmapping/sky130_fd_sc_hd__tt_025C_1v80.lib \
    -constr techmapping/abc.constr
setundef -zero
splitnets
opt_clean -purge
hilomap -singleton \
        -hicell sky130_fd_sc_hd__conb_1 HI \
        -locell sky130_fd_sc_hd__buf_4 A X
insbuf -buf sky130_fd_sc_hd__buf_4 A X
check
write_verilog -noattr -noexpr -nohex -nodec $::env(VERILOG_INPUT).mapped.v
