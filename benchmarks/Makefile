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
LIBERTY_LIBS := $(foreach LIB,${LIBERTY_FILES},$(shell basename ${LIB} .lib))
TECHMAPPING_SCRIPTS := $(patsubst %.lib,%.unoptimized.tcl,${LIBERTY_FILES})

RUNS := abc lsoracle migscript aigscript mixed_nodes mixed_depth mixed_ndp mixed_resynth timing_direct timing_direct_internal direct_ndp unoptimized

################################################################
######################## Targets ###############################
################################################################

SYNTH_OUTPUTS := $(patsubst %.v,%.unoptimized.rtl,${ORIGINAL_FILES})

PARTITION_OUTPUTS := $(patsubst %.v,%.direct.parts,${ORIGINAL_FILES})

OPTIMIZE_OUTPUTS := $(foreach RUN,${RUNS},$(patsubst %.v,%.${RUN}.rtl,${ORIGINAL_FILES}))

TECHMAP_OUTPUTS := $(foreach TECH,${LIBERTY_FILES},$(patsubst %.rtl,%.$(shell basename ${TECH} .lib).mapped.v,${OPTIMIZE_OUTPUTS}))

TIMING_OUTPUTS := $(patsubst %.mapped.v,%.sta.log,${TECHMAP_OUTPUTS})

JSON_REPORTS := $(patsubst %.mapped.v,%.report.json,${TECHMAP_OUTPUTS})

EQUIV_OUTPUTS := $(patsubst %.formality.log,%.mapped.v,${TECHMAP_OUTPUTS})

################################################################
######################## Phony targets #########################
################################################################

all: synth optimize techmap timing reports equivalence skywater

clean: clean_synth clean_synth clean_optimize clean_techmap clean_timing clean_reports clean_equivalence

picorv: $(foreach LIB,${LIBERTY_LIBS},$(foreach RUN,${RUNS},picorv32/picorv32.${RUN}.${LIB}.report.json))

.PHONY: synth clean_synth optimize clean_optimize techmap clean_techmap timing clean_timing reports clean_reports clean all picorv skywater

.PRECIOUS: %.rtl %.v %.log %.json %.blif %.sdc %.parts

.SECONDARY:

synth: $(SYNTH_OUTPUTS)

clean_synth:
	rm -f **/*.unoptimized.rtl **/*.unoptimized.v **/*.unoptimized.log **/*.unoptimized.blif

partition: $(PARTITION_OUTPUTS)

clean_partition:
	rm -f **/*.parts

optimize: $(OPTIMIZE_OUTPUTS)

clean_optimize:
	rm -f **/*.stats.json $(foreach RUN,${RUNS},**/*.${RUN}.rtl **/*.${RUN}.blif) **/*.optimize.log **/*.optimize_rtl.log

techmap: $(TECHMAP_OUTPUTS)

clean_techmap:
	rm -f **/*.mapped.v **/*.tech.log **/*.flatten.v **/*.tech.json  **/*.gates **/*.gates.json

timing: ${TIMING_OUTPUTS}

clean_timing:
	rm -f **/*.sta.log **/*.timing.json

reports: ${JSON_REPORTS}

clean_reports:
	rm -f **/*.report.json

equivalence: ${EQUIV_OUTPUTS}

clean_equivalence:
	rm -f **/*.formality.log

skywater: sky130_fd_sc_hd__tt_025C_1v80.pdf

# gf12: sc9mcpp84_12lp_base_rvt_c14_tt_nominal_max_0p70v_25c.pdf

# tigfet: sclib_tigfet10_hpall_tt_0p70v_25c.pdf

# tigfet1: sclib_tigfet10_hpnw1_tt_0p70v_25c.pdf

# tigfet4: sclib_tigfet10_hpnw4_tt_0p70v_25c.pdf

# tigfet8: sclib_tigfet10_hpnw8_tt_0p70v_25c.pdf

# tigfet12: sclib_tigfet10_hpnw12_tt_0p70v_25c.pdf

################################################################
######################### SYNTHESIS ############################
################################################################

%.unoptimized.rtl %.unoptimized.v %.unoptimized.log %.unoptimized.blif &: %.v
	${YOSYS} -Q -p 'read_verilog -sv $<; synth -auto-top -flatten -noabc; rename -top top; async2sync; dffunmap; dfflegalize -cell $$_DFF_P_ x -cell $$_DFF_N_ x; write_rtlil $*.unoptimized.rtl; write_verilog -norename $*.unoptimized.v; write_blif $*.unoptimized.blif; stat' | ${TEE} $*.unoptimized.log

# This works in limited circumstances due to flip-flops
%.aig %.aig.log &: %.rtl
	${YOSYS} -Q -p "read_rtlil $<; dffunmap; aigmap; write_aiger $*.aig" | ${TEE} $*.aig.log


################################################################
######################### PARTITION ############################
################################################################

%.direct.parts : %.unoptimized.blif
	$(eval TEMP=$(shell mktemp))
	@ echo "read_blif -a $<" > ${TEMP}
	@ echo "kahypar --size ${PART_SIZE} --epsilon 0.05" >> ${TEMP}
	@ echo "write_metis $@" >> ${TEMP}
	${LSORACLE} -e -f ${TEMP} | ${TEE} $*.partition.log
	rm ${TEMP}

################################################################
####################### OPTIMIZATION ###########################
################################################################

# TODO add a single internal-to-yosys partition file
%.mixed_ndp.stats.json %.mixed_ndp.rtl %.mixed_ndp.optimize.log &: %.unoptimized.rtl
	$(eval TEMP=$*.mixed_ndp.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "kahypar --size ${PART_SIZE} --epsilon 0.05" >> ${TEMP}
	@ echo "optimize --ndp --abc_exec ${ABC}" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_stats -g --name $(shell basename $*) --optimization mixed_ndp $*.mixed_ndp.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.mixed_ndp.rtl" | ${TEE} $*.mixed_ndp.optimize.log

%.mixed_depth.stats.json %.mixed_depth.rtl %.mixed_depth.optimize.log &: %.unoptimized.rtl
	$(eval TEMP=$*.mixed_depth.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "kahypar --size ${PART_SIZE} --epsilon 0.05" >> ${TEMP}
	@ echo "optimize --depth --abc_exec ${ABC}" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_blif -g $*.mixed_depth.internal.blif"
	@ echo "write_stats --name $(shell basename $*) --optimization mixed_depth -g $*.mixed_depth.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -nonames -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.mixed_depth.rtl" | ${TEE} $*.mixed_depth.optimize.log

%.mixed_nodes.stats.json %.mixed_nodes.rtl %.mixed_nodes.optimize.log &: %.unoptimized.rtl
	$(eval TEMP=$*.mixed_nodes.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "kahypar --size ${PART_SIZE} --epsilon 0.05" >> ${TEMP}
	@ echo "optimize --nodes --abc_exec ${ABC}" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization mixed_nodes -g $*.mixed_nodes.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.mixed_nodes.rtl" | ${TEE} $*.mixed_nodes.optimize.log

%.mixed_resynth.stats.json %.mixed_resynth.rtl %.mixed_resynth.optimize.log &: %.unoptimized.rtl
	$(eval TEMP=$*.mixed_resynth.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "kahypar --size ${PART_SIZE} --epsilon 0.05" >> ${TEMP}
	@ echo "optimize --resynth --abc_exec ${ABC}" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization mixed_resynth -g $*.mixed_resynth.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.mixed_resynth.rtl" | ${TEE} $*.mixed_resynth.optimize.log

%.lsoracle_old.stats.json %.lsoracle_old.rtl %.lsoracle_old.optimize.log &: %.unoptimized.rtl
	$(eval TEMP=$*.lsoracle_old.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "partitioning" >> ${TEMP}
	@ echo "optimization" >> ${TEMP}
	@ echo "ps -m" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization lsoracle -m $*.lsoracle_old.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.lsoracle_old.rtl" | ${TEE} $*.lsoracle_old.optimize.log

%.xmgscript.stats.json %.xmgscript.rtl %.xmgscript.optimize.log &: %.unoptimized.rtl
	$(eval TEMP=$*.xmgscript.script)
	@ echo "ps -a" > ${TEMP}
	@ echo "xmg_convert -a" >> ${TEMP}
	@ echo "xmgscript" >> ${TEMP}
	@ echo "function_stats -g" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "xmgscript" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "function_stats -g" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization xmgscript -g $*.xmgscript.stats.json" >> ${TEMP}
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.xmgscript.rtl" | ${TEE} $*.xmgscript.optimize.log

%.abc.stats.json: %.abc.blif
	${LSORACLE} -e -c "read_blif -a $<; write_stats --name $(shell basename $*) --optimization abc -a $@"

%.unoptimized.stats.json: %.unoptimized.blif
	${LSORACLE} -e -c "read_blif -a $<; write_stats --name $(shell basename $*) --optimization unoptimized -a $@"

%.aigscript.rtl %.aigscript.blif %.aigscript.optimize.log &: %.unoptimized.rtl
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -lso_exe ${LSORACLE} -aig -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.aigscript.rtl; write_blif $*.aigscript.blif" | ${TEE} $*.aigscript.optimize.log

%.aigscript.stats.json: %.aigscript.blif
	${LSORACLE} -e -c "read_blif -a $<; write_stats --name $(shell basename $*) --optimization aigscript -a $@"

%.migscript.rtl %.migscript.blif %.migscript.optimize.log &: %.unoptimized.rtl
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -lso_exe ${LSORACLE} -mig; techmap; opt -purge; stat; write_rtlil $*.migscript.rtl; write_blif $*.migscript.blif" | ${TEE} $*.migscript.optimize.log

%.migscript.stats.json: %.migscript.blif
	${LSORACLE} -e -c "read_blif -m $<; write_stats --name $(shell basename $*) --optimization migscript -m $@"

%.mixed_ndp_direct.stats.json %.direct_ndp.blif %.direct_ndp.optimize.log &: %.unoptimized.blif %.direct.parts
	$(eval TEMP=$*.direct_ndp.script)
	@ echo "read_blif -a $<" > ${TEMP}
	@ echo "ps -a" >> ${TEMP}
	@ echo "external_partition $*.direct.parts" >> ${TEMP}
	@ echo "optimize --ndp --abc_exec ${ABC}" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_blif -g --filename $*.direct_ndp.blif" >> ${TEMP}
	@ echo "write_stats -g --name $(shell basename $*) --optimization direct_ndp $*.direct_ndp.stats.json" >> ${TEMP}
	${LSORACLE} -e -f ${TEMP} | ${TEE} $*.direct_ndp.optimize.log

%.direct_ndp.rtl &: %.direct_ndp.blif
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_blif $*.direct_ndp.blif; stat; techmap; stat; opt -purge; stat; write_rtlil $*.direct_ndp.rtl" | ${TEE} $*.direct_ndp.optimize_rtl.log

define TIMING_SYNTH =
# Yosys name mangling is causing conflicts, not currently working.
# %.timing_yosys_internal.${2}.v %.timing_yosys.optimize.log %.timing_yosys.stats.json %.timing_yosys.rtl &: %.unoptimized.rtl %.timing_yosys.${2}.sdc %.direct.parts
# 	$(eval TEMP=$*.timing_yosys.script)
# 	@ echo "ps -a" > ${TEMP}
# 	@ echo "external_partition $*.direct.parts" >> ${TEMP}
# 	@ echo "optimize_timing --clock clk --sdc $*.timing_yosys.${2}.sdc --output $*.timing_yosys_internal.${2}.v --abc_exec ${ABC} --liberty ${1}" >> ${TEMP}
# 	@ echo "ps -g" >> ${TEMP}
# 	@ echo "write_blif -g --filename $*.timing_yosys.blif" >> ${TEMP}
# 	@ echo "write_stats --name $(shell basename $*) --optimization timing -g $*.timing_yosys.stats.json" >> ${TEMP}
# 	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_rtlil $<; lsoracle -xmg_out -lso_exe ${LSORACLE} -script ${TEMP}; techmap; opt -purge; stat; write_rtlil $*.timing_yosys.rtl" | ${TEE} $*.timing_yosys.optimize.log

%.timing_direct_internal.${2}.v %.timing_direct.optimize.log %.timing_direct.stats.json %.timing_direct_internal.stats.json %.timing_direct.blif &: %.unoptimized.blif %.timing_direct.${2}.sdc %.direct.parts
#	$(eval TEMP=$(shell mktemp))
	$(eval TEMP=$*.timing_direct.script)
	@ echo "read_blif -a $<" > ${TEMP}
	@ echo "ps -a" >> ${TEMP}
	@ echo "external_partition $*.direct.parts" >> ${TEMP}
	@ echo "optimize_timing --clock clk --sdc $*.timing_direct.${2}.sdc --output $*.timing_direct_internal.${2}.v --abc_exec ${ABC} --liberty ${1}" >> ${TEMP}
	@ echo "ps -g" >> ${TEMP}
	@ echo "write_blif -g --filename $*.timing_direct.blif" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization timing_direct -g $*.timing_direct.stats.json" >> ${TEMP}
	@ echo "write_stats --name $(shell basename $*) --optimization timing_direct_internal -g $*.timing_direct_internal.stats.json" >> ${TEMP}
	${LSORACLE} -e -f ${TEMP} | ${TEE} $*.timing_direct.optimize.log
endef
$(foreach SCRIPT,${TECHMAPPING_SCRIPTS},$(eval $(call TIMING_SYNTH,$(SCRIPT),$(shell basename $(SCRIPT) .synth.tcl))))

%.timing_direct.rtl &: %.timing_direct.blif
	${YOSYS} -m ${YOSYS_PLUGIN} -Q -p "read_blif $*.timing_direct.blif; stat; techmap; stat; opt -purge; stat; write_rtlil $*.timing_direct.rtl" | ${TEE} $*.timing_direct.optimize_rtl.log

%.timing_direct_internal.rtl: %.timing_direct.rtl
	cp $< $@

%.abc.rtl %.abc.blif %.abc.optimize.log &: %.unoptimized.rtl
	${YOSYS} -Q -p "read_rtlil $<; abc -script techmapping/resyn2rs.abc; techmap; stat; write_rtlil $*.abc.rtl; write_blif $*.abc.blif" | ${TEE} $*.abc.optimize.log

################################################################
####################### TECHMAPPING ############################
################################################################

define TECHMAPPING =
$(2).test:
	echo foo
%.$(2).mapped.v %.$(2).tech.log &: %.rtl
	CIRCUIT_INPUT=$$< OUTPUT_FILE=$$*.$(2).mapped.v $${YOSYS} -Q -c $(1) | ${TEE} $$*.$(2).tech.log
endef
$(foreach SCRIPT,${TECHMAPPING_SCRIPTS},$(eval $(call TECHMAPPING,$(SCRIPT),$(shell basename $(SCRIPT) .synth.tcl))))

################################################################
#################### EQUIVALENCE CHECK #########################
################################################################

%.flatten.v: %.v
	${YOSYS} -p "read_verilog $<; flatten; hierarchy -auto-top; rename -top top; write_verilog  -noattr $@"

#TODO add per run copies of %.unoptimized.v, all targets
%.sky130_fd_sc_hd__tt_025C_1v80.formality.log: %.sky130_fd_sc_hd__tt_025C_1v80.flatten.v %.unoptimized.v
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

# %.sky130_fd_sc_hd__tt_025C_1v80.tech_equiv.log: %.sky130_fd_sc_hd__tt_025C_1v80.mapped.v %.unoptimized.mapped.v
# 	${YOSYS} -p "read_verilog ${word 2,$^}; rename -top gold; prep -top gold; splitnets -ports; design -stash gold;\
# # 		read_verilog $<; rename -top gate; prep -top gate design -stash gate; \
# # 		design -copy-from gold -as gold gold; design -copy-from gate -as gate gate; \
# # 		equiv_make -inames gold gate equiv; hierarchy -top equiv; equiv_simple; equiv_status -assert" | ${TEE} $@

# %.lsoracle_old.equiv.log: %.lsoracle_old.rtl %.unoptimized.rtl
# 	${YOSYS} -p "read_rtlil ${word 2,$^}; rename -top gold; prep -top gold; splitnets -ports; design -stash gold;\
# # 		read_rtlil $<; rename -top gate; prep -top gate; design -stash gate; \
# # 		design -copy-from gold -as gold gold; design -copy-from gate -as gate gate; \
# # 		equiv_make -inames gold gate equiv; hierarchy -top equiv; equiv_simple; equiv_status -assert" | ${TEE} $@

################################################################
########################### TIMING #############################
################################################################
define SDC_COPY =
%.$(RUN).$(LIB).sdc: %.sdc
	cp $$< $$@
endef
$(foreach LIB,${LIBERTY_LIBS},$(foreach RUN,${RUNS},$(eval $(SDC_COPY))))

define TIMING =
%.$(2).sta.log: %.$(2).mapped.v %.$(2).sdc
	VERILOG_INPUT=$$< VERILOG_TOP=top SDC_FILE=$$*.$(2).sdc LIBERTY_FILE=$(1) ${STA} -exit techmapping/sta.tcl | ${TEE} $$@
endef
$(foreach LIBERTY_FILE,${LIBERTY_FILES},$(eval $(call TIMING,$(LIBERTY_FILE),$(shell basename ${LIBERTY_FILE} .lib))))

################################################################
######################### REPORTS ##############################
################################################################
define TIMING_JSON =
%.$(2).stats.json: %.stats.json
	cp $$< $$@

%.$(2).tech.json: %.$(2).mapped.v
	@ echo generating report $$@
	@ echo '{ "liberty_file": "$(2)" }' > $$@

%.$(2).gates: %.$(2).mapped.v
	${YOSYS} -Q -p "read_verilog $$<; stat -liberty $(1)" > $$@
endef
$(foreach LIBERTY_FILE,${LIBERTY_FILES},$(eval $(call TIMING_JSON,$(LIBERTY_FILE),$(shell basename ${LIBERTY_FILE} .lib))))

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

everything.tsv: ${JSON_REPORTS}
	@ echo generating report $@
	jq -r '[.liberty_file, .design, .optimization.name, .area_estimate.total, .size, .depth, .timing["data_arrival"], .power.total.total] | @tsv' < $^ > $@


define GRAPHS =
${LIB}.png ${LIB}.pdf: ${JSON_REPORTS}
	python3 plot_bars.py $@ **/*.${LIB}.report.json
endef
$(foreach LIB,$(LIBERTY_LIBS),$(eval ${GRAPHS}))
