# LSOracle: A learning based Oracle for Logic Synthesis
# MIT License
# Copyright 2019 Laboratory for Nano Integrated Systems (LNIS)
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

CLK_PERIOD := 10
STA := sta
LSORACLE := lsoracle
ABC := yosys-abc
YOSYS := yosys
YOSYS_PLUGIN := oracle
TEE := cat - >
FM := fm_shell

PART_SIZE := 2048

ISCAS85 := iscas85/c432.v iscas85/c880a.v iscas85/c3540a.v iscas85/c5288.v iscas85/c2670a.v iscas85/c5315a.v iscas85/c3540.v iscas85/c1908a.v iscas85/c7552.v iscas85/c499.v iscas85/c2670.v iscas85/c1908.v iscas85/c5315.v iscas85/c1335.v
ISCAS89 := iscas89/s1196.v iscas89/s1238.v iscas89/s15850.v iscas89/s298.v
OPDB := opdb/chip_bridge.pickle.v opdb/fpu.pickle.v opdb/gng.pickle.v # opdb/sparc_core.pickle.v
EPFL := EPFL/adder.v EPFL/arbiter.v EPFL/bar.v EPFL/i2c.v EPFL/log2.v EPFL/max.v EPFL/mem_ctrl.v EPFL/multiplier.v #EPFL/sin.v EPFL/sqrt.v EPFL/square.v EPFL/voter.v

ORIGINAL_FILES := picorv32/picorv32.v ${EPFL} ibex/ibex.v # ${OPDB} # ${ISCAS85} ${ISCAS89}
LIBERTY_FILES := techmapping/skywater/sky130_fd_sc_hd__tt_025C_1v80.lib #\
#	techmapping/tigfet/sclib_tigfet10_hpall_tt_0p70v_25c.lib \
#	techmapping/tigfet/sclib_tigfet10_hpnw1_tt_0p70v_25c.lib \
#	techmapping/tigfet/sclib_tigfet10_hpnw4_tt_0p70v_25c.lib \
#	techmapping/tigfet/sclib_tigfet10_hpnw8_tt_0p70v_25c.lib \
#	techmapping/tigfet/sclib_tigfet10_hpnw12_tt_0p70v_25c.lib \
#	techmapping/gf12/sc9mcpp84_12lp_base_rvt_c14_tt_nominal_max_0p70v_25c.lib
TECHMAPPING_SCRIPTS := $(patsubst %.lib,%.synth.tcl,${LIBERTY_FILES})

################################################################
######################## Targets ###############################
################################################################

AIG_FILES := $(patsubst %.v,%.aig,${ORIGINAL_FILES})
AIG_LOGS := $(patsubst %.v,%.aig.log,${ORIGINAL_FILES})

SDC_SCRIPTS := $(patsubst %.v,%.lsoracle.sdc,${ORIGINAL_FILES}) $(patsubst %.v,%.abc.sdc,${ORIGINAL_FILES}) $(patsubst %.v,%.unoptimized.sdc,${ORIGINAL_FILES}) $(patsubst %.v,%.aigscript.sdc,${ORIGINAL_FILES}) $(patsubst %.v,%.migscript.sdc,${ORIGINAL_FILES})

SYNTH_BASE := $(patsubst %.v,%.rtl,${ORIGINAL_FILES})
PART_FILES := $(patsubst %.v,%.part,${ORIGINAL_FILES})
SYNTH_BASE_LOGS := $(patsubst %.rtl,%.synth.log,${SYNTH_BASE})

LSORACLE_OUTPUTS := $(patsubst %.rtl,%.lsoracle.rtl,${SYNTH_BASE})
LSORACLE_SYNTH_LOG := $(patsubst %.rtl,%.lsoracle.synth.log,${SYNTH_BASE})
LSORACLE_TECHMAP_OUTPUTS := $(patsubst %.rtl,%.lsoracle.mapped.v,${SYNTH_BASE})
LSORACLE_SYNTH_REPORTS := $(patsubst %.rtl,%.lsoracle.synth_report,${SYNTH_BASE})
LSORACLE_TECHMAP_LOGS := $(patsubst %.rtl,%.lsoracle.tech.log,${SYNTH_BASE})
LSORACLE_TIMING_REPORT := $(patsubst %.rtl,%.lsoracle.timing,${SYNTH_BASE})
LSORACLE_TIMING_LOG := $(patsubst %.rtl,%.lsoracle.sta.log,${SYNTH_BASE})

AIGSCRIPT_OUTPUTS := $(patsubst %.rtl,%.aigscript.rtl,${SYNTH_BASE})
MIGSCRIPT_OUTPUTS := $(patsubst %.rtl,%.migscript.rtl,${SYNTH_BASE})
TIMING_SYNTH_OUTPUTS := $(patsubst %.rtl,%.timing.rtl,${SYNTH_BASE})

ABC_OUTPUTS := $(patsubst %.rtl,%.abc.rtl,${SYNTH_BASE})
UNOPT_OUTPUTS := $(patsubst %.rtl,%.unoptimized.rtl,${SYNTH_BASE})
SYNTH_OUTPUTS := ${LSORACLE_OUTPUTS} ${UNOPT_OUTPUTS} ${ABC_OUTPUTS} ${AIGSCRIPT_OUTPUTS} ${MIGSCRIPT_OUTPUTS} ${TIMING_SYNTH_OUTPUTS}
SYNTH_LOGS := $(patsubst %.rtl,%.synth.log,${SYNTH_OUTPUTS})

BLIF_FILES := $(patsubst %.rtl,%.blif,${SYNTH_BASE})
TIMING_SYNTH_INTERNAL := $(patsubst %.rtl,%.timing.internal.v,${SYNTH_BASE})

SYNTH_REPORTS := $(patsubst %.synth.log,%.synth_report,${SYNTH_LOGS})
TECHMAP_OUTPUTS := $(patsubst %.rtl,%.mapped.v,${SYNTH_OUTPUTS})
TECHMAP_LOGS := $(patsubst %.rtl,%.tech.log,${SYNTH_OUTPUTS})
TIMING := $(patsubst %.mapped.v,%.sta.log,${TECHMAP_OUTPUTS})
TIMING_REPORTS := $(patsubst %.sta.log,%.timing,${TIMING})
#EVERYTHING_REPORTS := $(patsubst %.v,%.everything,${ORIGINAL_FILES})
REPORTS := $(patsubst %.v,%.report,${ORIGINAL_FILES})
RUNS := abc lsoracle redux_nodes redux_depth redux_ndp redux_resynth timing_direct_indep direct_ndp unoptimized
JSON_REPORTS := $(foreach RUN,${RUNS},$(foreach TECH,${LIBERTY_FILES},$(patsubst %.v,%.${RUN}.$(shell basename ${TECH} .lib).report.json,${ORIGINAL_FILES}))) $(foreach TECH,${LIBERTY_FILES},$(patsubst %.v,%.timing_direct_indep.$(shell basename ${TECH} .lib).internal.report.json,${ORIGINAL_FILES}))

################################################################
######################## Phony targets #########################
################################################################

test:
	@ echo ${JSON_REPORTS}

all: ${JSON_REPORTS}

.PHONY: extract synth reports techmap reports timing everything_reports clean all reset test lsoracle

.PRECIOUS: %.rtl %.v %.log %.json %.blif %.sdc %.parts

.SECONDARY:

sdc_skels: $(patsubst %.v,%.sdc_skel,${ORIGINAL_FILES})

clean_synth: clean_techmap
	rm -f tigfet/*_lsoracle.rtl tigfet/*_lsoracle.stats.json

clean_techmap: clean_timing
	rm -f tigfet/*.mapped.v tigfet/*.tech.log

clean_timing: clean_reports
	rm -f tigfet/*.sta.log tigfet/*.gates

clean_reports:
	rm -f tigfet/*.report.json tigfet/*.tech.json tigfet/*.timing.json tigfet/*.gates.json

extract: ${SYNTH_BASE}

lsoracle: $(LSORACLE_OUTPUTS)

synth: $(SYNTH_OUTPUTS)

techmap: $(TECHMAP_OUTPUTS)

reports: ${REPORTS}

timing: ${TIMING}

sdc_skel: $(patsubst %.v,%.sdc,$(wildcard iscas89/*.v))

# reset:
# 	rm -f ${LSORACLE_OUTPUTS} ${LSORACLE_SYNTH_LOG} ${LSORACLE_TECHMAP_OUTPUTS} ${LSORACLE_SYNTH_REPORTS} ${LSORACLE_TECHMAP_LOGS} ${LSORACLE_TIMING_REPORT} ${LSORACLE_TIMING_LOG} ${REPORTS}

clean: clean_reports
	rm -f ${SDC_SCRIPTS} ${BLIF_FILES} ${AIG_FILES} ${AIG_LOGS} ${SYNTH_BASE} ${SYNTH_BASE_LOGS} ${SYNTH_OUTPUTS} ${SYNTH_LOGS} ${TECHMAP_OUTPUTS} ${TIMING} ${SDC_SCRIPTS} ${TECHMAP_LOGS} ${PART_FILES} ${TIMING_SYNTH_BLIF} ${TIMING_SYNTH_INTERNAL}

# clean_reports:
# 	rm -f ${SYNTH_REPORTS} ${TIMING_REPORTS} ${REPORTS} ${EVERYTHING_REPORTS}  everything.tsv everything.png
################################################################
######################### SYNTHESIS ############################
################################################################

%.rtl %.synth.v %.synth.log &: %.v
	${YOSYS} -Q -p 'read_verilog -sv $<; synth -auto-top -flatten -noabc; rename -top top; async2sync; dffunmap; dfflegalize -cell $$_DFF_P_ x -cell $$_DFF_N_ x; write_rtlil $*.rtl; write_verilog -norename $*.synth.v; stat' | ${TEE} $*.synth.log

# This works in limited circumstances due to flip-flops
%.aig %.aig.log &: %.rtl
	${YOSYS} -Q -p "read_rtlil $<; dffunmap; aigmap; write_aiger $*.aig" | ${TEE} $*.aig.log

%.redux_ndp.stats.json %.redux_ndp.rtl %.redux_ndp.synth.log &: %.rtl %.parts
	$(eval TEMP=$*.redux_ndp.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "kahypar --size ${PART_SIZE} --epsilon 0.05" >> ${TEMP}
	@ echo "optimization_redux --ndp --abc_exec ${ABC}" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_stats -g --name $(shell basename $*) --optimization redux_ndp $*.redux_ndp.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.redux_ndp.rtl; write_blif $*.redux_ndp.yosys.blif" | ${TEE} $*.redux_ndp.synth.log

%.redux_depth.stats.json %.redux_depth.rtl %.redux_depth.synth.log &: %.rtl %.parts
	$(eval TEMP=$*.redux_depth.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "kahypar --size ${PART_SIZE} --epsilon 0.05" >> ${TEMP}
	@ echo "optimization_redux --depth --abc_exec ${ABC}" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_blif -g $*.redux_depth.internal.blif"
	@ echo "write_stats --name $(shell basename $*) --optimization redux_depth -g $*.redux_depth.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -nonames -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.redux_depth.rtl; write_blif $*.redux_depth.yosys.blif" | ${TEE} $*.redux_depth.synth.log

%.redux_nodes.stats.json %.redux_nodes.rtl %.redux_nodes.synth.log &: %.rtl %.parts
	$(eval TEMP=$*.redux_nodes.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "kahypar --size ${PART_SIZE} --epsilon 0.05" >> ${TEMP}
	@ echo "optimization_redux --nodes --abc_exec ${ABC}" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization redux_nodes -g $*.redux_nodes.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.redux_nodes.rtl; write_blif $*.redux_nodes.yosys.blif" | ${TEE} $*.redux_nodes.synth.log

%.redux_resynth.stats.json %.redux_resynth.rtl %.redux_resynth.synth.log &: %.rtl %.parts
	$(eval TEMP=$*.redux_resynth.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "kahypar --size ${PART_SIZE} --epsilon 0.05" >> ${TEMP}
	@ echo "budget_simple --abc_exec ${ABC}" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization redux_resynth -g $*.redux_resynth.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.redux_resynth.rtl; write_blif $*.redux_resynth.yosys.blif" | ${TEE} $*.redux_resynth.synth.log

%.lsoracle.stats.json %.lsoracle.rtl %.lsoracle.synth.log &: %.rtl
	$(eval TEMP=$*.lsoracle.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "partitioning" >> ${TEMP}
	@ echo "optimization" >> ${TEMP}
	@ echo "ps -m" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization lsoracle -m $*.lsoracle.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.lsoracle.rtl; write_blif $*.lsoracle.yosys.blif" | ${TEE} $*.lsoracle.synth.log

%.xmgscript.stats.json %.xmgscript.rtl %.xmgscript.synth.log &: %.rtl
	$(eval TEMP=$*.xmgscript.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "xmg_convert -a" >> ${TEMP}
	@ echo "xmgscript2" >> ${TEMP}
	@ echo "function_stats -g" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "xmgscript" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "function_stats -g" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization xmgscript -g $*.xmgscript.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.xmgscript.rtl; write_blif $*.xmgscript.yosys.blif" | ${TEE} $*.xmgscript.synth.log

%.xmg3.stats.json %.xmg3.rtl %.xmg3.synth.log &: %.timing_yosys_post.rtl
	$(eval TEMP=$*.xmg3.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "xmg_convert -a" >> ${TEMP}
	@ echo "function_stats -g" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "xmgscript2" >> ${TEMP}
	@ echo "function_stats -g" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "xmgscript" >> ${TEMP}
	@ echo "function_stats -g" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization xmg2 -g $*.xmg3.stats.json" >> ${TEMP}
	@ echo "write_blif -g $*.xmg3.inner.blif" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.xmg3.rtl; write_blif $*.xmg3.yosys.blif" | ${TEE} $*.xmg3.synth.log

%.xmg2.stats.json %.xmg2.rtl %.xmg2.synth.log &: %.timing.rtl
	$(eval TEMP=$*.xmg2.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "xmg_convert -a" >> ${TEMP}
	@ echo "function_stats -g" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "xmgscript2" >> ${TEMP}
	@ echo "function_stats -g" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "xmgscript" >> ${TEMP}
	@ echo "function_stats -g" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization xmg2 -g $*.xmg2.stats.json" >> ${TEMP}
	@ echo "write_blif -g $*.xmg2.inner.blif" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.xmg2.rtl; write_blif $*.xmg2.yosys.blif" | ${TEE} $*.xmg2.synth.log

%.abc.stats.json: %.abc.blif
	${LSORACLE} -e -c "read_blif -a $<; write_stats --name $(shell basename $*) --optimization abc -a $@"

%.unoptimized.stats.json: %.blif
	${LSORACLE} -e -c "read_blif -a $<; write_stats --name $(shell basename $*) --optimization unoptimized -a $@"

%.aigscript.rtl %.aigscript.synth.log &: %.rtl
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -lso_exe ${LSORACLE} -aig -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.aigscript.rtl" | ${TEE} $*.aigscript.synth.log

%.migscript.rtl %.migscript.synth.log &: %.rtl
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -lso_exe ${LSORACLE} -mig; techmap; opt -purge; stat; write_rtlil $*.migscript.rtl" | ${TEE} $*.migscript.synth.log

%.blif: %.rtl
	${YOSYS} -Q -p "read_rtlil $<; write_blif $@"

%.parts : %.blif
	$(eval TEMP=$(shell mktemp))
	@ echo "read_blif -a $<" > ${TEMP}
	@ echo "kahypar --size ${PART_SIZE} --epsilon 0.05" >> ${TEMP}
	@ echo "write_metis $@" >> ${TEMP}
	${LSORACLE} -e -f ${TEMP} | ${TEE} $*.partition.log
	rm ${TEMP}

%.direct_ndp.stats.json %.direct_ndp.blif %.direct_ndp.synth.log &: %.blif %.parts
	$(eval TEMP=$*.direct_ndp.script)
	@ echo "read_blif -a $<" > ${TEMP}
	@ echo "ps -a" >> ${TEMP}
	@ echo "external_partition $*.parts" >> ${TEMP}
	@ echo "optimization_redux --ndp --abc_exec ${ABC}" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_blif -g --filename $*.direct_ndp.blif" >> ${TEMP}
	@ echo "write_stats -g --name $(shell basename $*) --optimization direct_ndp $*.direct_ndp.stats.json" >> ${TEMP}
	${LSORACLE} -e -f ${TEMP} | ${TEE} $*.direct_ndp.synth.log

%.direct_ndp.rtl &: %.direct_ndp.blif
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_blif $*.direct_ndp.blif; stat; techmap; stat; opt -purge; stat; write_rtlil $*.direct_ndp.rtl" | ${TEE} $*.direct_ndp.synth_tech.log


TECHLIB := sky130_fd_sc_hd__tt_025C_1v80
# TODO clock signal name, library
%.timing_direct_indep.${TECHLIB}.internal.v %.timing_direct_indep.synth.log %.timing_direct_indep.stats.json %.timing_direct_indep.internal.stats.json %.timing_direct_indep.blif &: %.blif %.timing_direct_indep.sdc %.parts
#	$(eval TEMP=$(shell mktemp))
	$(eval TEMP=$*.timing_direct_indep.script)
	@ echo "read_blif -a $<" > ${TEMP}
	@ echo "ps -a" >> ${TEMP}
	@ echo "external_partition $*.parts" >> ${TEMP}
	@ echo "budget_script --tech-independent --clock clk --sdc $*.timing_direct_indep.sdc --output $*.timing_direct_indep.${TECHLIB}.internal.v --abc_exec ${ABC} --liberty techmapping/skywater/${TECHLIB}.lib" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_blif -g --filename $*.timing_direct_indep.blif" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization timing_direct_indep -g $*.timing_direct_indep.stats.json" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization timing_direct_indep_internal -g $*.timing_direct_indep.internal.stats.json" >> ${TEMP}
	${LSORACLE} -e -f ${TEMP} | ${TEE} $*.timing_direct_indep.synth.log

%.timing_direct_indep.rtl &: %.timing_direct_indep.blif
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_blif $*.timing_direct_indep.blif; stat; techmap; stat; opt -purge; stat; write_rtlil $*.timing_direct_indep.rtl" | ${TEE} $*.timing_direct_indep.synth_tech.log

# TODO clock signal name, library
%.timing_direct_depen.${TECHLIB}.internal.v %.timing_direct_depen.synth.log %.timing_direct_depen.stats.json %.timing_direct_depen.rtl &: %.blif %.timing_direct_depen.sdc %.parts
	$(eval TEMP=$*.timing_direct_depen.script)
	@ echo "read_blif -a $<" > ${TEMP}
	@ echo "ps -a" >> ${TEMP}
	@ echo "external_partition $*.parts" >> ${TEMP}
	@ echo "budget_script --tech-dependent --clock clk --sdc $*.timing_direct_depen.sdc --output $*.timing_direct_depen.${TECHLIB}.internal.v --abc_exec ${ABC} --liberty techmapping/skywater/${TECHLIB}.lib" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_blif -g --filename $*.timing_direct_depen.blif" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization timing -g $*.timing_direct_depen.stats.json" >> ${TEMP}
	${LSORACLE} -e -f ${TEMP} | ${TEE} $*.timing_direct_depen.synth.log
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_blif $*.timing_direct_depen.blif; techmap; opt -purge; stat; write_rtlil $*.timing_direct_depen.rtl"

# TODO clock signal name, library
%.timing_yosys_indep.${TECHLIB}.internal.v %.timing_yosys_indep.synth.log %.timing_yosys_indep.stats.json %.timing_yosys_indep.rtl &: %.rtl %.timing_yosys_indep.sdc %.parts
	$(eval TEMP=$*.timing_yosys_indep.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "external_partition $*.parts" >> ${TEMP}
	@ echo "budget_script --tech-independent --clock clk --sdc $*.timing_yosys_indep.sdc --output $*.timing_yosys_indep.${TECHLIB}.internal.v --abc_exec ${ABC} --liberty techmapping/skywater/${TECHLIB}.lib" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_blif -g --filename $*.timing_yosys_indep.blif" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization timing -g $*.timing_yosys_indep.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.timing_yosys_indep.rtl; write_blif $*.timing_yosys_indep.yosys.blif" | ${TEE} $*.timing_yosys_indep.synth.log

# TODO clock signal name, library
%.timing_yosys_depend.${TECHLIB}.internal.v %.timing_yosys_depend.synth.log %.timing_yosys_depend.stats.json %.timing_yosys_depend.rtl &: %.rtl %.timing_yosys_depend.sdc
	$(eval TEMP=$*.timing_yosys_depend.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "external_partition $*.parts" >> ${TEMP}
	@ echo "budget_script --tech-dependent --clock clk --sdc $*.timing_yosys_depend.sdc --output $*.timing_yosys_depend.${TECHLIB}.internal.v --abc_exec ${ABC} --liberty techmapping/skywater/${TECHLIB}.lib" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_blif -g --filename $*.timing_yosys_depend.blif" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization timing -g $*.timing_yosys_depend.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.timing_yosys_depend.rtl; write_blif $*.timing_yosys_depend.yosys.blif" | ${TEE} $*.timing_yosys_depend.synth.log

%.unoptimized.rtl: %.rtl
	cp $< $@

%.abc.rtl %.abc.synth.log &: %.rtl
	${YOSYS} -Q -p "read_rtlil $<; abc -script techmapping/resyn2rs.abc; techmap; stat; write_rtlil $*.abc.rtl" | ${TEE} $*.abc.synth.log

define TECHMAPPING =
%.$(shell basename ${TECHMAPPING_SCRIPT} .synth.tcl).mapped.v %.$(shell basename ${TECHMAPPING_SCRIPT} .synth.tcl).tech.log &: %.rtl
	CIRCUIT_INPUT=$$< OUTPUT_FILE=$$*.$(shell basename ${TECHMAPPING_SCRIPT} .synth.tcl).mapped.v $${YOSYS} -Q -c ${TECHMAPPING_SCRIPT} | ${TEE} $$*.$(shell basename ${TECHMAPPING_SCRIPT} .synth.tcl).tech.log
endef
$(foreach TECHMAPPING_SCRIPT,${TECHMAPPING_SCRIPTS},$(eval $(TECHMAPPING)))

# %.mapped.v %.tech.log &: %.rtl
# 	CIRCUIT_INPUT=$* ${YOSYS} -Q -c ${TECHMAPPING_SCRIPT} | ${TEE} $*.tech.log

################################################################
#################### EQUIVALENCE CHECK #########################
################################################################

%.flatten.v: %.v
	${YOSYS} -p "read_verilog $<; flatten; hierarchy -auto-top; rename -top top; write_verilog  -noattr $@"

%.timing.formality.log: %.timing.flatten.v %.synth.v
	$(eval TEMP=$(shell mktemp))
	@ echo "read_verilog -r ${word 2,$^}" > ${TEMP}
	@ echo "set_top top" >> ${TEMP}
	@ echo "read_db -i techmapping/skywater/sky130_fd_sc_hd__tt_025C_1v80.db" >> ${TEMP}
	@ echo "read_verilog -i $<" >> ${TEMP}
	@ echo "set_top top" >> ${TEMP}
	@ echo "match" >> ${TEMP}
	@ echo "verify" >> ${TEMP}
	cat ${TEMP}
	${FM} -work_path formality_workdir -file ${TEMP} -overwrite | ${TEE} $@
	rm ${TEMP}

%.direct_ndp.sky130_fd_sc_hd__tt_025C_1v80.formality.log: %.direct_ndp.sky130_fd_sc_hd__tt_025C_1v80.mapped.v %.synth.v
	$(eval TEMP=$(shell mktemp))
	@ echo "read_verilog -r ${word 2,$^}" > ${TEMP}
	@ echo "set_top top" >> ${TEMP}
	@ echo "read_db -i techmapping/skywater/sky130_fd_sc_hd__tt_025C_1v80.db" >> ${TEMP}
	@ echo "read_verilog -i $<" >> ${TEMP}
	@ echo "set_top top" >> ${TEMP}
	@ echo "match" >> ${TEMP}
	@ echo "verify" >> ${TEMP}
	@ echo "analyze_points -failing" >> ${TEMP}
	cat ${TEMP}
	${FM} -work_path formality_workdir -file ${TEMP} -overwrite | ${TEE} $@
	rm ${TEMP}

%.timing_direct_indep.sky130_fd_sc_hd__tt_025C_1v80.internal_flat.v: %.timing_direct_indep.sky130_fd_sc_hd__tt_025C_1v80.internal.v
	${YOSYS} -Q -p "read_verilog $<; flatten; hierarchy -top top; write_verilog $@" | ${TEE} $*.timing_direct_indep.sky130_fd_sc_hd__tt_025C_1v80.internal_flat.log

%.timing_direct_indep.sky130_fd_sc_hd__tt_025C_1v80.internal.formality.log: %.timing_direct_indep.sky130_fd_sc_hd__tt_025C_1v80.internal_flat.v %.synth.v
	$(eval TEMP=$*.timing_direct_indep.sky130_fd_sc_hd__tt_025C_1v80.formality.script)
#	$(eval TEMP=$(shell mktemp))
	@ echo "read_verilog -r ${word 2,$^}" > ${TEMP}
	@ echo "set_top top" >> ${TEMP}
	@ echo "read_db -i techmapping/skywater/sky130_fd_sc_hd__tt_025C_1v80.db" >> ${TEMP}
	@ echo "read_verilog -i $<" >> ${TEMP}
	@ echo "set_top top" >> ${TEMP}
	@ echo "match" >> ${TEMP}
	@ echo "verify" >> ${TEMP}
	cat ${TEMP}
	${FM} -work_path formality_workdir -file ${TEMP} -overwrite | ${TEE} $@
#	rm ${TEMP}

%.lsoracle.tech_equiv.log: %.lsoracle.mapped.v %.unoptimized.mapped.v
	${YOSYS} -p "read_verilog ${word 2,$^}; rename -top gold; prep -top gold; splitnets -ports; design -stash gold;\
		read_verilog $<; rename -top gate; prep -top gate design -stash gate; \
		design -copy-from gold -as gold gold; design -copy-from gate -as gate gate; \
		equiv_make -inames gold gate equiv; hierarchy -top equiv; equiv_simple; equiv_status -assert" | ${TEE} $@

%.timing.tech_equiv.log: %.timing.flatten.v %.unoptimized.mapped.v
	${YOSYS} -p "read_verilog ${word 2,$^}; rename -top gold; design -stash gold;\
		read_verilog $<; rename -top gate; design -stash gate; \
		design -copy-from gold -as gold gold; design -copy-from gate -as gate gate; \
		equiv_make -inames gold gate equiv; hierarchy -top equiv; equiv_simple; equiv_status -assert" | ${TEE} $@

%.lsoracle.equiv.log: %.lsoracle.rtl %.rtl
	${YOSYS} -p "read_rtlil ${word 2,$^}; rename -top gold; prep -top gold; splitnets -ports; design -stash gold;\
		read_rtlil $<; rename -top gate; prep -top gate; design -stash gate; \
		design -copy-from gold -as gold gold; design -copy-from gate -as gate gate; \
		equiv_make -inames gold gate equiv; hierarchy -top equiv; equiv_simple; equiv_status -assert" | ${TEE} $@

################################################################
########################### TIMING #############################
################################################################

# this is there to help generate these, but they need to be tweaked to get clocks.
%.sdc_skel: %.rtl
	echo "create_clock -name clk -period ${CLK_PERIOD} {clk_i}" > $@
	echo "set_input_delay -clock clk 0 {" >> $@
	${YOSYS} -Q -p  'read_rtlil $<; select i:* -list' | sed -e '/^top/!d' -e 's/top\///' >> $@
	echo "}" >> $@
	echo "set_output_delay -clock clk 0 {" >> $@
	${YOSYS} -Q -p  'read_rtlil $<; select o:* -list' | sed -e '/^top/!d' -e 's/top\///' >> $@
	echo "}" >> $@

# %.sdc: dummy.sdc
# 	cp $< $@

%.lsoracle.sdc: %.sdc
	cp $< $@
%.unoptimized.sdc: %.sdc
	cp $< $@
%.migscript.sdc: %.sdc
	cp $< $@
%.aigscript.sdc: %.sdc
	cp $< $@
%.abc.sdc: %.sdc
	cp $< $@
%.xmg2.sdc: %.sdc
	cp $< $@
%.xmgscript.sdc: %.sdc
	cp $< $@
%.redux.sdc: %.sdc
	cp $< $@
%.timing_yosys_post.sdc: %.sdc
	cp $< $@
%.redux_depth.sdc: %.sdc
	cp $< $@
%.redux_ndp.sdc: %.sdc
	cp $< $@
%.redux_nodes.sdc: %.sdc
	cp $< $@
%.timing_yosys.sdc: %.sdc
	cp $< $@
%.direct_ndp.sdc: %.sdc
	cp $< $@
%.timing_direct_indep.sdc: %.sdc
	cp $< $@
%.timing_direct_depen.sdc: %.sdc
	cp $< $@
%.xmg3.sdc: %.sdc
	cp $< $@
%.redux_resynth.sdc: %.sdc
	cp $< $@

# %.lsoracle.sdc %.abc.sdc %.migscript.sdc %.aigscript.sdc %.unoptimized.sdc: %.sdc
# 	cp $< $@

define TIMING =
%.$(shell basename ${LIBERTY_FILE} .lib).sta.log: %.$(shell basename ${LIBERTY_FILE} .lib).mapped.v %.sdc
	VERILOG_INPUT=$$< VERILOG_TOP=top SDC_FILE=$$*.sdc LIBERTY_FILE=${LIBERTY_FILE} ${STA} -exit techmapping/sta.tcl | ${TEE} $$@

%.$(shell basename ${LIBERTY_FILE} .lib).internal.sta.log: %.$(shell basename ${LIBERTY_FILE} .lib).internal.v %.sdc
	VERILOG_INPUT=$$< VERILOG_TOP=top SDC_FILE=$$*.sdc LIBERTY_FILE=${LIBERTY_FILE} ${STA} -exit techmapping/sta.tcl | ${TEE} $$@
endef
$(foreach LIBERTY_FILE,${LIBERTY_FILES},$(eval $(TIMING)))

# %.sta.log: %.mapped.v %.sdc
# 	VERILOG_INPUT=$< VERILOG_TOP=top SDC_FILE=$*.sdc LIBERTY_FILE=${LIBERTY_FILE} ${STA} -exit techmapping/sta.tcl | ${TEE} $@


################################################################
######################### REPORTS ##############################
################################################################
# Dummy columns if previous doesn't exist.
previous:
	mkdir -p previous

previous/%.lsoracle.everything: | previous
	@ echo -n "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" > $@

%.timing: %.sta.log %.tech.log
	$(eval TYPE=$(shell echo $* | sed 's/^.*_\([a-z]*\)$$/\1/'))
	@ echo generating report $@
	@ sed -n 's/^Combinational\s*\([-.e0-9]*\)\s*\([-.e0-9]*\)\s*\([-.e0-9]*\)\s*\([-.e0-9]*\)\s*\([.0-9]*\)%.*/${TYPE}_internal_power\t\1\n${TYPE}_switching_power\t\2\n${TYPE}_leakage_power\t\3\n${TYPE}_total_power\t\4\n${TYPE}_percent_power\t\5/p' $< > $@
	@ sed -n 's/\s*-\([0-9.]*\)\s *data arrival time/\1/p' $< | python3 -c 'import sys; print("${TYPE}_arrival\t", max([float(i) for i in sys.stdin.readlines()]))' >> $@
	@ sed -n 's/worst slack \([0-9.]*\)/${TYPE}_worst_slack\t\1/p' $< >> $@
	@ sed -e "/Area/!d" -e "s/.*Area = *\([0-9.]*\) .*/${TYPE}_area\t\1/" $*.tech.log >> $@

define TIMING_JSON =
%.$(shell basename ${LIBERTY_FILE} .lib).stats.json: %.stats.json
	cp $$< $$@

%.$(shell basename ${LIBERTY_FILE} .lib).internal.stats.json: %.internal.stats.json
	cp $$< $$@

%.$(shell basename ${LIBERTY_FILE} .lib).tech.json: %.$(shell basename ${LIBERTY_FILE} .lib).mapped.v
	@ echo generating report $$@
	@ echo '{ "liberty_file": "$(shell basename ${LIBERTY_FILE} .lib)" }' > $$@

%.$(shell basename ${LIBERTY_FILE} .lib).gates: %.$(shell basename ${LIBERTY_FILE} .lib).mapped.v
	${YOSYS} -Q -p "read_verilog $$<; stat -liberty ${LIBERTY_FILE}" > $$@

%.$(shell basename ${LIBERTY_FILE} .lib).internal.tech.json: %.$(shell basename ${LIBERTY_FILE} .lib).internal.v
	@ echo generating report $$@
	@ echo '{ "liberty_file": "$(shell basename ${LIBERTY_FILE} .lib)" }' > $$@

%.$(shell basename ${LIBERTY_FILE} .lib).internal.gates: %.$(shell basename ${LIBERTY_FILE} .lib).internal.v
	${YOSYS} -Q -p "read_verilog $$<; flatten; hierarchy -auto-top; stat -liberty ${LIBERTY_FILE}" > $$@

endef
$(foreach LIBERTY_FILE,${LIBERTY_FILES},$(eval $(TIMING_JSON)))

POWER_REGEX := \s*\([-+.e0-9]*\)\s*\([-+.e0-9]*\)\s*\([-+.e0-9]*\)\s*\([-+.e0-9]*\)\s*\([.0-9]*\)%.*
POWER_JSON := {"internal": \1, "switching": \2, "leakage": \3, "total": \4, "percent": \5}
%.timing.json: %.sta.log
	@ echo generating report $@
	@ echo '{"power": {' > $@
	@ sed -n 's/^Combinational${POWER_REGEX}/"combinational": ${POWER_JSON},/p' $< >> $@
	@ sed -n 's/^Sequential${POWER_REGEX}/"sequential": ${POWER_JSON},/p' $< >> $@
	@ sed -n 's/^Pad${POWER_REGEX}/"pad": ${POWER_JSON},/p' $< >> $@
	@ sed -n 's/^Macro${POWER_REGEX}/"macro": ${POWER_JSON},/p' $< >> $@
	@ sed -n 's/^Total${POWER_REGEX}/"total": ${POWER_JSON}/p' $< >> $@
	@ echo '}, "timing": {' >> $@
	@ sed -n 's/\s*-\([0-9.]*\)\s *data arrival time/\1/p' $< | \
		python3 -c 'import sys; print("\"data_arrival\": ", max([float(i) for i in sys.stdin.readlines()]), ",")' >> $@
	@ sed -n 's/worst slack \([0-9.]*\)/"worst_slack": \1/p' $< >> $@
	@ echo '}}' >> $@

%.gates.json: %.gates
	@ echo generating report $@
	@  echo '{ "gates": {' > $@
	@ sed -e '1,/Number of cells/d' -e '/^$$/,$$d' $< \
	 | sed 's/\s*\(\S*\)\s*\([0-9]*\)/"\1": \2/' \
	 | python3 -c 'import sys; print(",".join(sys.stdin.readlines()))'  >> $@
	@ echo '},' >> $@
	@ sed -n 's/\s*Number of wire bits:\s*\([0-9]*\)/"wires": \1,/p' $< >> $@
	@ echo '"area_estimate": {' >> $@
	@ sed -n 's/\s*Chip area for module .*: \([.0-9]*\)/"total": \1/p' $< >> $@
	@ echo '}}' >> $@

%.report.json: %.timing.json %.stats.json %.tech.json %.gates.json
	jq -s '.[0] * .[1] * .[2] * .[3]' $^ > $@

%.lsoracle.synth_report: %.lsoracle.synth.log
	@ echo generating report $@
	@ sed -n -e "s/LSOracle: inputs: \([0-9]*\)/inputs\t\1/p" \
	-e "s/LSOracle: outputs: \([0-9]*\)/outputs\t\1/p" \
	-e "s/LSOracle: AIG level: \([0-9]*\)/unoptimized_level\t\1/p" \
	-e "s/LSOracle: AIG nodes: \([0-9]*\)/unoptimized_nodes\t\1/p" \
	-e "s/LSOracle.* Using \([0-9]*\) partitions/lsoracle_partitions\t\1/p" \
	-e "s/LSOracle: Final ntk size = \([0-9]*\) and depth = \([0-9]*\)/lsoracle_nodes\t\1\nlsoracle_level\t\2/p" \
	-e "s/LSOracle: \([0-9]*\) AIGs and \([0-9]*\) MIGs/lsoracle_aigs\t\1\\nlsoracle_migs\t\2/p" \
	-e "s/LSOracle: Full Optimization: \([0-9]*\)ms/lsoracle_runtime\t\1/p" \
	-e "s/LSOracle: MAJ nodes internally = \([0-9]*\)/lsoracle_maj_nodes_internal\t\1/p" \
	-e "s/LSOracle: AND nodes internally = \([0-9]*\)/lsoracle_and_nodes_internal\t\1/p" \
	-e "s/LSOracle: AND nodes on critical path = \([0-9]*\)/lsoracle_critical_and\t\1/p" \
	-e "s/LSOracle: MAJ nodes on critical path = \([0-9]*\)/lsoracle_critical_maj\t\1/p" \
	-e "s/LSOracle: INPUTS nodes on critical path = \([0-9]*\)/lsoracle_critical_inputs\t\1/p" < $< > $@

%.migscript.synth_report: %.migscript.synth.log
	@ echo generating report $@
	@ sed -n -e "s/LSOracle: Final ntk size = \([0-9]*\) and depth = \([0-9]*\)/migscript_nodes\t\1\\nmigscript_level\t\2/p" \
	-e "s/LSOracle: Full Optimization: \([0-9]*\)ms/migscript_runtime\t\1/p" \
	-e "s/LSOracle: MAJ nodes internally = \([0-9]*\)/migscript_maj_nodes_internal\t\1/p" \
	-e "s/LSOracle: AND nodes internally = \([0-9]*\)/migscript_and_nodes_internal\t\1/p" \
	-e "s/LSOracle: AND nodes on critical path = \([0-9]*\)/migscript_critical_and\t\1/p" \
	-e "s/LSOracle: MAJ nodes on critical path = \([0-9]*\)/migscript_critical_maj\t\1/p" \
	-e "s/LSOracle: INPUTS nodes on critical path = \([0-9]*\)/migscript_critical_inputs\t\1/p" < $< > $@

%.aigscript.synth_report: %.aigscript.synth.log
	@ sed -n -e "s/LSOracle: Final ntk size = \([0-9]*\) and depth = \([0-9]*\)/aigscript_nodes\t\1\\naigscript_level\t\2/p" \
	-e "s/LSOracle: Full Optimization: \([0-9]*\)ms/aigscript_runtime\t\1/p" < $< > $@

%.abc.synth_report: %.abc.synth.log
	@ echo generating report $@
	@ sed -n 's/ABC:.*and = *\([0-9]*\) *lev = *\([0-9*]\)/abc_nodes\t\1\nabc_level\t\2/p' < $< > $@

%.report: %.lsoracle.synth_report %.lsoracle.timing %.abc.synth_report %.abc.timing %.unoptimized.timing %.migscript.synth_report %.migscript.timing %.aigscript.synth_report %.aigscript.timing
	@ echo "circuit\t$*" > $@
	cat $^ >> $@

#%.everything: %.report
#	@ echo generating report $@
#	@ cut -f1 < $< | tr '\n' '\t' | sed 's/\t$$//' > $@
#	@ echo >> $@
#	@ cut -f2 < $< | tr '\n' '\t' | sed 's/\t$$//' >> $@
#	@ echo >> $@
everything.tsv: ${REPORTS}
	@ echo generating report $@
	python3 combine_report.py $^

#everything.tsv: ${EVERYTHING_REPORTS}
#	@ echo generating report $@
#	python3 combine_report.py $< $^

everything.png everything.pdf: everything.tsv
	python3 plot_bars.py everything.tsv previous/everything.tsv $@
