puts $::env(CIRCUIT_INPUT)

yosys -import
echo
read_rtlil $::env(CIRCUIT_INPUT)
read_liberty -lib techmapping/tigfet/sclib_tigfet10_hpnw1_tt_0p70v_25c.lib
# read_verilog -defer techmapping/cells_clkgate_hd.v
# techmap -map techmapping/tigfet/cells_latch_hd.v
dffunmap
dfflegalize -cell \$_DFF_P_ x -cell \$_DFF_N_ x
stat
dfflibmap -liberty techmapping/tigfet/sclib_tigfet10_hpnw1_tt_0p70v_25c.lib
opt
abc -D 10 \
    -script techmapping/tigfet/abc.script \
    -liberty techmapping/tigfet/sclib_tigfet10_hpnw1_tt_0p70v_25c.lib \
    -constr techmapping/tigfet/abc.constr
setundef -zero
splitnets
opt_clean -purge
check
flatten
write_verilog -noattr -noexpr -nohex -nodec $::env(OUTPUT_FILE)
stat -liberty techmapping/tigfet/sclib_tigfet10_hpnw1_tt_0p70v_25c.lib
