puts $::env(CIRCUIT_INPUT)

yosys -import
echo
read_rtlil $::env(CIRCUIT_INPUT)
read_liberty -lib techmapping/gf12/sc9mcpp84_12lp_base_rvt_c14_tt_nominal_max_0p70v_25c.lib
dffunmap
dfflegalize -cell \$_DFF_P_ x -cell \$_DFF_N_ x
stat
dfflibmap -liberty techmapping/gf12/sc9mcpp84_12lp_base_rvt_c14_tt_nominal_max_0p70v_25c.lib
opt
abc -D 10 \
    -script techmapping/gf12/abc.script \
    -liberty techmapping/gf12/sc9mcpp84_12lp_base_rvt_c14_tt_nominal_max_0p70v_25c.lib \
    -constr techmapping/gf12/abc.constr
setundef -zero
splitnets
opt_clean -purge
check
flatten
write_verilog -noattr -noexpr -nohex -nodec $::env(OUTPUT_FILE)
stat -liberty techmapping/gf12/sc9mcpp84_12lp_base_rvt_c14_tt_nominal_max_0p70v_25c.lib
