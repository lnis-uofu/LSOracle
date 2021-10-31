create_clock -name clk -period 10 {}
set_input_delay -clock clk 0 {
trailingones[1]
trailingones[0]
totalcoeffs[4]
totalcoeffs[3]
totalcoeffs[2]
totalcoeffs[1]
totalcoeffs[0]
ctable[2]
ctable[1]
ctable[0]
}
set_output_delay -clock clk 0 {
ctoken_len[4]
ctoken_len[3]
ctoken_len[2]
ctoken_len[1]
ctoken_len[0]
coeff_token[5]
coeff_token[4]
coeff_token[3]
coeff_token[2]
coeff_token[1]
coeff_token[0]
}
