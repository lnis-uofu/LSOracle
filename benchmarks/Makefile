CLK_PERIOD := 10
STA := sta
LSORACLE := lsoracle
ABC := yosys-abc
YOSYS := yosys

#BENCHMARKS = 74181 74182 74283 74L85 c1335 c17 c1908a c1908 c2670a c2670 c3540a c3540 c432 c499 c5288 c5315a c5315 c7552 c880g c880 picorv32 s1196 s1238 s13207 s1423 s1488 s15850 s208 s27 s298 s344 s349 s35932 s382 s38417 s38584 s386 s400 s420 s444 s510 s526 s5378 s641 s713 s820 s832 s838 s9234 s938 s953
BENCHMARKS = picorv32
################################################################
######################## Targets ###############################
################################################################

ORIGINAL_FILES := $(foreach BENCH,${BENCHMARKS},${BENCH}.v)
SDC_SCRIPTS := $(foreach BENCH,${BENCHMARKS},${BENCH}.sdc)
LSORACLE_SCRIPTS := $(patsubst %.v,%.lsoracle,${ORIGINAL_FILES})
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

.PRECIOUS: ${AIG_FILES} ${SDC_SCRIPTS} ${LSORACLE_SCRIPTS} ${VERILOG_OUTPUTS} ${REPORTS} ${TECHMAPPED} ${TIMING} ${TIMING_REPORTS} ${EVERYTHING_REPORTS}

extract: ${AIG_FILES}

synth: $(VERILOG_OUTPUTS)

techmap: $(TECHMAPPED)

reports: ${REPORTS}

timing: ${SDC_SCRIPTS} ${TIMING}

clean:
	rm -rf ${AIG_FILES} ${SDC_SCRIPTS} ${LSORACLE_SCRIPTS} ${VERILOG_OUTPUTS} ${REPORTS} ${TECHMAPPED} ${TIMING} ${TIMING_REPORTS} ${EVERYTHING_REPORTS} *.out

################################################################
########################### RULES ##############################
################################################################

%_lsoracle.aig %_abc.aig %_unoptimized.aig: %.aig
	cp $< $@

%_lsoracle.sdc %_abc.sdc %_unoptimized.sdc: %.sdc
	cp $< $@

%.aig: %.v
	VERILOG_INPUT=$* ${YOSYS} -Q -c techmapping/aig.tcl | tee $*.aig.out

%.lsoracle: %.aig
	$(eval CIRCUIT_SIZE := $(shell ${LSORACLE} -c 'read_aig $<; ps -a' | grep '^nodes' | cut -f 2 -d ' '))
	$(eval PARTITIONS := $(shell python3 -c 'import math; print(math.floor(${CIRCUIT_SIZE}/2048))'))
	echo "read_aig $<; partitioning ${PARTITIONS} -c kahypar.ini; optimization; write_verilog -m $*_lsoracle.v"  > $@

%_lsoracle.v: %.lsoracle
	${LSORACLE} -e -f $< | tee $*_lsoracle.synth.out
	sed -i -e "s/^module .*(/module top (/" $@

%.sdc: %.aig
	echo "create_clock -name clk -period ${CLK_PERIOD} {}" > $@
	echo "set_input_delay -clock clk 0 {" >> $@
	${LSORACLE} -c 'read_aig $<; show_ntk' | grep 'PI' | cut -f4 -d ' ' >> $@
	echo "}" >> $@
	echo "set_output_delay -clock clk 0 {" >> $@
	${LSORACLE} -c 'read_aig $<; show_ntk' | grep 'PO' | cut -f4 -d ' ' >> $@
	echo "}" >> $@

%.timing: %.sta.out
	grep -E "(-.*data arrival time|^Total)" $< | sed -e 's/Total\s*\([-.e0-9]*\)\s*\([-.e0-9]*\)\s*\([-.e0-9]*\)\s*\([-.e0-9]*\).*/\1\t\2\t\3\t\4/' -e 's/\s*-\([0-9.]*\)\s *data arrival time/\1/' | tr '\n' '\t' | sed 's/\t\t/\t/' > $@
	sed -e "/Area/!d" -e "s/.*Area = *\([0-9.]*\) .*/\1/" $*.yosys.out >> $@

%.mapped.v: %.v
	VERILOG_INPUT=$* VERILOG_TOP=top ${YOSYS} -Q -c techmapping/synth.tcl | tee $*.yosys.out

previous:
	mkdir -p previous

previous/%_lsoracle.report: | previous
	echo "\t\t\t\t\t\t" > $@

previous/%_lsoracle.timing: | previous
	echo "\t\t\t\t\t" > $@

%.everything: %_lsoracle.report %_lsoracle.timing %_abc.report %_abc.timing %_unoptimized.report %_unoptimized.timing previous/%_lsoracle.report previous/%_lsoracle.timing
	echo -n "$*\t" > $@
	cat $^ | tr '\n' '\t' >> $@
	echo >> $@

%.sta.out: %.mapped.v %.sdc
	VERILOG_INPUT=$< VERILOG_TOP=top SDC_FILE=$*.sdc ${STA} -exit techmapping/sta.tcl | tee $@

%.report: %.mapped.v %.aig
	$(eval CIRCUIT_SIZE := $(shell ${LSORACLE} -c 'read_aig $*.aig; ps -a' | grep '^nodes' | cut -f 2 -d ' '))
	sed -n -e 's/.*partitioning \([0-9]*\).*/\1/;s/\(.*\) AIGs and \(.*\) MIGs/\1\t\2/;s/.*size = \([0-9]*\) and depth = \([0-9]*\)/\1\t\2/;s/.*Product = \([0-9]\)/\1/;s/.*Optimization: \([0-9]*\)ms/\1/;1p;4p;5p;7p;8p' $*.synth.out | tr "\n" "\t" > $@;
	echo "${CIRCUIT_SIZE}"  >> $@

%_unoptimized.v: %.aig
	${LSORACLE} -c 'read_aig $<; ps -a; write_verilog $@' | tee $*_unoptimized.synth.out
	sed -i -e "s/^module .*(/module top (/" $@

%_unoptimized.report: %_unoptimized.v
	sed -e '/stored/d' -e 's/\(.*\): *\([0-9]*\)/\1\t\2/' $*_unoptimized.synth.out | cut -f 2 | tr '\n' '\t' | sed 's/\t$$//' > $@
	echo >> $@

%_abc.v: %.aig
	${ABC} -c "read_aiger $<; resyn2rs; write_verilog $@; write_aiger %_abc_opt.aig"
	${LSORACLE} -c 'read_aig %_abc_opt.aig; ps -a' | tee %_abc.synth.out
	sed -i -e "s/^module .* (/module top (/" -e "s/\([pl][io]\)0*\([0-9]\)/\1\2/g" $@

%_abc.report: %_abc.v
	sed -e '/stored/d' -e 's/\(.*\): *\([0-9]*\)/\1\t\2/' %_abc.synth.out | cut -f 2 | tr '\n' '\t' | sed 's/\t$$//' > $@
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
