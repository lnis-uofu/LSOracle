create_clock -name clk -period 10 {}
set_input_delay -clock clk 0 {
opcode[4]
opcode[3]
opcode[2]
opcode[1]
opcode[0]
op_ext[1]
op_ext[0]
}
set_output_delay -clock clk 0 {
sign
sel_wb
sel_reg_dst[1]
sel_reg_dst[0]
sel_pc_opB
sel_pc_opA
sel_alu_opB[1]
sel_alu_opB[0]
reg_write
mem_write
jump
invB
invA
halt
bnez
bltz
bgez
beqz
alu_op_ext[3]
alu_op_ext[2]
alu_op_ext[1]
alu_op_ext[0]
alu_op[2]
alu_op[1]
alu_op[0]
Cin
}
