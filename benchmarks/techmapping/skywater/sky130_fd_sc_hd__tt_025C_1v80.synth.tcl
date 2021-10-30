puts $::env(CIRCUIT_INPUT)

yosys -import
echo
read_rtlil $::env(CIRCUIT_INPUT)
read_liberty -lib techmapping/skywater/sky130_fd_sc_hd__tt_025C_1v80.lib
# read_verilog -defer techmapping/skywater/cells_clkgate_hd.v
techmap -map techmapping/skywater/cells_latch_hd.v
dfflibmap -liberty techmapping/skywater/sky130_fd_sc_hd__tt_025C_1v80.lib
opt
abc -D 10 \
    -script techmapping/skywater/abc.script \
    -liberty techmapping/skywater/sky130_fd_sc_hd__tt_025C_1v80.lib \
    -constr techmapping/skywater/abc.constr
setundef -zero
splitnets
opt_clean -purge
hilomap -singleton \
        -hicell sky130_fd_sc_hd__conb_1 HI \
        -locell sky130_fd_sc_hd__buf_4 A X
insbuf -buf sky130_fd_sc_hd__buf_4 A X
check
flatten
write_verilog -noattr -noexpr -nohex -nodec $::env(OUTPUT_FILE)
