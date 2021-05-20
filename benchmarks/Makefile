CLK_PERIOD := 10
STA := sta
LSORACLE := lsoracle
ABC := yosys-abc
YOSYS := yosys

#BENCHMARKS = 74181 74182 74283 74L85 c1335 c17 c1908a c1908 c2670a c2670 c3540a c3540 c432 c499 c5288 c5315a c5315 c7552 c880g c880 picorv32 s1196 s1238 s13207 s1423 s1488 s15850 s208 s27 s298 s344 s349 s35932 s382 s38417 s38584 s386 s400 s420 s444 s510 s526 s5378 s641 s713 s820 s832 s838 s9234 s938 s953
BENCHMARKS = picorv32 Circuit74182 ibex
################################################################
######################## Targets ###############################
################################################################

ORIGINAL_FILES := $(foreach BENCH,${BENCHMARKS},${BENCH}.v)
SDC_SCRIPTS := $(foreach BENCH,${BENCHMARKS},${BENCH}.sdc)
YOSYS_SCRIPTS := $(patsubst %.v,%.ys,${ORIGINAL_FILES})
LSORACLE_OUTPUTS := $(patsubst %.v,%_lsoracle.v,${ORIGINAL_FILES})
ABC_OUTPUTS := $(patsubst %.v,%_abc.v,${ORIGINAL_FILES})
UNOPT_OUTPUTS := $(patsubst %.v,%_unoptimized.v,${ORIGINAL_FILES})
VERILOG_OUTPUTS := ${LSORACLE_OUTPUTS} ${UNOPT_OUTPUTS} ${ABC_OUTPUTS}
AIG_FILES := $(patsubst %.v,%.aig,${VERILOG_OUTPUTS})

REPORTS := $(patsubst %.v,%.report,${VERILOG_OUTPUTS})
TECHMAPPED := $(patsubst %.v,%.mapped.v,${VERILOG_OUTPUTS})
TIMING := $(patsubst %.mapped.v,%.sta.out,${TECHMAPPED})
TIMING_REPORTS := $(patsubst %.sta.out,%.timing,${TIMING})
EVERYTHING_REPORTS := $(patsubst %.v,%.everything,${ORIGINAL_FILES})

################################################################
######################## Phony targets #########################
################################################################
.PHONY: extract synth reports techmap reports timing everything_reports clean

.PRECIOUS: ${AIG_FILES} ${SDC_SCRIPTS} ${LSORACLE_SCRIPTS} ${VERILOG_OUTPUTS} ${REPORTS} ${TECHMAPPED} ${TIMING} ${TIMING_REPORTS} ${EVERYTHING_REPORTS} $(foreach BENCH,${BENCHMARKS},${BENCH}.aig)

extract: ${AIG_FILES}

synth: $(VERILOG_OUTPUTS)

techmap: $(TECHMAPPED)

reports: ${REPORTS}

timing: ${SDC_SCRIPTS} ${TIMING}

clean:
	rm -rf ${AIG_FILES} ${SDC_SCRIPTS} ${LSORACLE_SCRIPTS} ${VERILOG_OUTPUTS} ${REPORTS} ${TECHMAPPED} ${TIMING} ${TIMING_REPORTS} ${EVERYTHING_REPORTS} *.out

################################################################
######################### SYNTHESIS ############################
################################################################

%.rtl: %.v
	${YOSYS} -Q -p "read_verilog $<; synth -auto-top -flatten -noabc; rename -top top; write_rtlil $@" | tee $*.synth.out

# This works in limited circumstances due to flip-flops
%.aig: %.rtl
	${YOSYS} -Q -p "read_rtlil $<; dffunmap; aigmap; write_aiger $@" | tee $*.aig.out

%_lsoracle.rtl: %.rtl
	${YOSYS} -m oracle -Q -p "read_rtlil $<; lsoracle -c kahypar.ini; techmap; opt -purge; stat; write_rtlil $@" | tee $*_lsoracle.synth.out

%_unoptimized.rtl: %.rtl
	cp $< $@

%_abc.rtl: %.rtl
	${YOSYS} -Q -p "read_rtlil $<; abc -script techmapping/resyn2rs.abc; techmap; stat; write_rtlil $@" | tee $*_abc.synth.out

%.equiv.out: %_lsoracle.rtl %.rtl
	${YOSYS} -p "read_rtil ${word 2,$^}; rename -top gold; design -stash gold;\
		read_rtlil $<; rename -top gate; design -stash gate; \
		design -copy-from gold -as gold gold; design -copy-from gate -as gate gate; \
		equiv_make -inames gold gate equiv; hierarchy -top equiv; equiv_simple; equiv_status -assert;" | tee $@

%.mapped.v: %.rtl
	CIRCUIT_INPUT=$* ${YOSYS} -Q -c techmapping/synth.tcl | tee $*.yosys.out

################################################################
########################### TIMING #############################
################################################################

# this is there to help generate these, but they need to be tweaked to get clocks.
%.sdc_skel: %.rtl
	echo "create_clock -name clk -period ${CLK_PERIOD} {}" > $@
	echo "set_input_delay -clock clk 0 {" >> $@
	${YOSYS} -Q -p  'read_rtlil picorv32.rtl; select i:* -list' | sed -e '/^top/!d' -e 's/top\///' >> $@
	echo "}" >> $@
	echo "set_output_delay -clock clk 0 {" >> $@
	${YOSYS} -Q -p  'read_rtlil picorv32.rtl; select o:* -list' | sed -e '/^top/!d' -e 's/top\///' >> $@
	echo "}" >> $@

%_lsoracle.sdc %_abc.sdc %_unoptimized.sdc: %.sdc
	cp $< $@

%.sta.out: %.mapped.v %.sdc
	VERILOG_INPUT=$< VERILOG_TOP=top SDC_FILE=$*.sdc ${STA} -exit techmapping/sta.tcl | tee $@

################################################################
######################### REPORTS ##############################
################################################################
# Dummy columns if previous doesn't exist.
previous:
	mkdir -p previous

previous/%_lsoracle.report: | previous
	echo "\t\t\t\t\t\t" > $@

previous/%_lsoracle.timing: | previous
	echo "\t\t\t\t\t" > $@

%.timing: %.sta.out
	grep -E "(-.*data arrival time|^Total)" $< | sed -e 's/Total\s*\([-.e0-9]*\)\s*\([-.e0-9]*\)\s*\([-.e0-9]*\)\s*\([-.e0-9]*\).*/\1\t\2\t\3\t\4/' -e 's/\s*-\([0-9.]*\)\s *data arrival time/\1/' | tr '\n' '\t' | sed 's/\t\t/\t/' > $@
	sed -e "/Area/!d" -e "s/.*Area = *\([0-9.]*\) .*/\1/" $*.yosys.out >> $@

#TODO broken
%_lsoracle.report: %.mapped.v %.aig
	$(eval CIRCUIT_SIZE := $(shell ${LSORACLE} -c 'read_aig $*.aig; ps -a' | grep '^nodes' | cut -f 2 -d ' '))
	sed -n -e 's/.*partitioning \([0-9]*\).*/\1/;s/\(.*\) AIGs and \(.*\) MIGs/\1\t\2/;s/.*size = \([0-9]*\) and depth = \([0-9]*\)/\1\t\2/;s/.*Product = \([0-9]\)/\1/;s/.*Optimization: \([0-9]*\)ms/\1/;1p;4p;5p;7p;8p' $*.synth.out | tr "\n" "\t" > $@;
	echo "${CIRCUIT_SIZE}"  >> $@

#TODO broken
%_unoptimized.report: %_unoptimized.rtl
	sed -e '/stored/d' -e 's/\(.*\): *\([0-9]*\)/\1\t\2/' $*_unoptimized.synth.out | cut -f 2 | tr '\n' '\t' | sed 's/\t$$//' > $@
	echo >> $@

#TODO broken
%_abc.report: %_abc.rtl
	sed -e '/stored/d' -e 's/\(.*\): *\([0-9]*\)/\1\t\2/' %_abc.synth.out | cut -f 2 | tr '\n' '\t' | sed 's/\t$$//' > $@
	echo >> $@

%.everything: %_lsoracle.report %_lsoracle.timing %_abc.report %_abc.timing %_unoptimized.report %_unoptimized.timing previous/%_lsoracle.report previous/%_lsoracle.timing
	echo -n "$*\t" > $@
	cat $^ | tr '\n' '\t' >> $@
	echo >> $@

everything.tsv: ${EVERYTHING_REPORTS}
	echo -n "circuit\tpartitions\taigs\tmigs\tnodes\tdepth\tnodes\truntime\toriginal\t" > $@
	echo -n "internal_power\tswitching_power\tleakage_power\ttotal_power\tarrival\tarea\t" >> $@
	echo -n "abc_nodes\tabc_inputs\tabc_latches\tabc_outputs\tabc_nodes\tabc_depth\t" >> $@
	echo -n "abc_internal_power\tabc_switching_power\tabc_leakage_power\tabc_total_power\tabc_arrival\tabc_area\t" >> $@
	echo -n "unopt_nodes\tunopt_inputs\tunopt_latches\tunopt_outputs\tunopt_nodes\tunopt_depth\t" >> $@
	echo -n "unopt_internal_power\tunopt_switching_power\tunopt_leakage_power\tunopt_total_power\tunopt_arrival\tunopt_area\t" >> $@
	echo -n "prev_partitions\tprev_aigs\tprev_migs\tprev_nodes\tprev_depth\tprev_nodes\tprev_runtime\tprev_original\t" >> $@
	echo "prev_internal_power\tprev_switching_power\tprev_leakage_power\tprev_total_power\tprev_arrival\tprev_area" >> $@
	cat $^ >> $@

everything.png: everything.tsv
	python3 plot_bars.py $< $@
