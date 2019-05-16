/****************************************************************************
 *                                                                          *
 *  VERILOG HIGH-LEVEL DESCRIPTION OF THE ISCAS-85 BENCHMARK CIRCUIT c499   *
 *                                                                          *
 *  Function: Single-Error-Correcting Circuit                               *
 *                                                                          *
 *  Written by: Mark C. Hansen                                              *
 *                                                                          *
 *  Last modified: Jan 12, 1998                                             *
 *                                                                          *
 ****************************************************************************/

module Circuit499 (in1, in5, in9, in13, in17, in21, in25, in29, 
             in33, in37, in41, in45, in49, in53, in57, in61,
             in65, in69, in73, in77, in81, in85, in89, in93,
             in97, in101, in105, in109, in113, in117, in121, in125,
             in129, in130, in131, in132, in133, in134, in135, in136,
             in137,
             out724, out725, out726, out727, out728, out729, out730, out731,
             out732, out733, out734, out735, out736, out737, out738, out739, 
             out740, out741, out742, out743, out744, out745, out746, out747, 
             out748, out749, out750, out751, out752, out753, out754, out755);

  input      in1, in5, in9, in13, in17, in21, in25, in29, 
             in33, in37, in41, in45, in49, in53, in57, in61,
             in65, in69, in73, in77, in81, in85, in89, in93,
             in97, in101, in105, in109, in113, in117, in121, in125,
             in129, in130, in131, in132, in133, in134, in135, in136,
             in137;
  output     out724, out725, out726, out727, out728, out729, out730, out731,
             out732, out733, out734, out735, out736, out737, out738, out739, 
             out740, out741, out742, out743, out744, out745, out746, out747, 
             out748, out749, out750, out751, out752, out753, out754, out755;


  wire [0:31]   ID, OD;
  wire [0:7]    IC;
  wire          R;

  assign
      ID[0:31] = { in1, in5, in9, in13, in17, in21, in25, in29, 
             in33, in37, in41, in45, in49, in53, in57, in61,
             in65, in69, in73, in77, in81, in85, in89, in93,
             in97, in101, in105, in109, in113, in117, in121, in125},
      IC[0:7] = { in129, in130, in131, in132, in133, in134, in135, in136},
      R = in137,
      OD[0:31] = { 
             out724, out725, out726, out727, out728, out729, out730, out731,
             out732, out733, out734, out735, out736, out737, out738, out739, 
             out740, out741, out742, out743, out744, out745, out746, out747, 
             out748, out749, out750, out751, out752, out753, out754, out755};
	
  TopLevel499 Ckt499 (ID, IC, R, OD);

endmodule /* Circuit499 */

/*************************************************************************/

module  TopLevel499 (ID, IC, R, OD);

  input[0:31]   ID;
  input[0:7]    IC;
  input          R;
  output[0:31]  OD;

  wire[0:7]      S;

  Syndrome M1(S, R, IC, ID);
  Correction M2(OD, S, ID);

endmodule /* TopLevel499 */

/*************************************************************************/

module Syndrome (S, R, IC, ID);

  input[0:31]   ID;
  input[0:7]    IC;
  input         R;
  output[0:7]   S;
  
  wire[0:15]    XA;
  wire[0:7]     XB, XC, XD, XE, F, G, H;

xor XA0(XA[0], ID[0], ID[1]);
xor XA1(XA[1], ID[2], ID[3]);
xor XA2(XA[2], ID[4], ID[5]);
xor XA3(XA[3], ID[6], ID[7]);
xor XA4(XA[4], ID[8], ID[9]);
xor XA5(XA[5], ID[10], ID[11]);
xor XA6(XA[6], ID[12], ID[13]);
xor XA7(XA[7], ID[14], ID[15]);
xor XA8(XA[8], ID[16], ID[17]);
xor XA9(XA[9], ID[18], ID[19]);
xor XA10(XA[10], ID[20], ID[21]);
xor XA11(XA[11], ID[22], ID[23]);
xor XA12(XA[12], ID[24], ID[25]);
xor XA13(XA[13], ID[26], ID[27]);
xor XA14(XA[14], ID[28], ID[29]);
xor XA15(XA[15], ID[30], ID[31]);

xor F0(F[0], XA[0], XA[1]);
xor F1(F[1], XA[2], XA[3]);
xor F2(F[2], XA[4], XA[5]);
xor F3(F[3], XA[6], XA[7]);
xor F4(F[4], XA[8], XA[9]);
xor F5(F[5], XA[10], XA[11]);
xor F6(F[6], XA[12], XA[13]);
xor F7(F[7], XA[14], XA[15]);

and H0(H[0], IC[0], R);
and H1(H[1], IC[1], R);
and H2(H[2], IC[2], R);
and H3(H[3], IC[3], R);
and H4(H[4], IC[4], R);
and H5(H[5], IC[5], R);
and H6(H[6], IC[6], R);
and H7(H[7], IC[7], R);

xor XB0(XB[0], ID[0], ID[4]);
xor XB1(XB[1], ID[1], ID[5]);
xor XB2(XB[2], ID[2], ID[6]);
xor XB3(XB[3], ID[3], ID[7]);
xor XB4(XB[4], ID[16], ID[20]);
xor XB5(XB[5], ID[17], ID[21]);
xor XB6(XB[6], ID[18], ID[22]);
xor XB7(XB[7], ID[19], ID[23]);

xor XC0(XC[0], ID[8], ID[12]);
xor XC1(XC[1], ID[9], ID[13]);
xor XC2(XC[2], ID[10], ID[14]);
xor XC3(XC[3], ID[11], ID[15]);
xor XC4(XC[4], ID[24], ID[28]);
xor XC5(XC[5], ID[25], ID[29]);
xor XC6(XC[6], ID[26], ID[30]);
xor XC7(XC[7], ID[27], ID[31]);

xor XE0(XE[0], XB[0], XC[0]);
xor XE1(XE[1], XB[1], XC[1]);
xor XE2(XE[2], XB[2], XC[2]);
xor XE3(XE[3], XB[3], XC[3]);
xor XE4(XE[4], XB[4], XC[4]);
xor XE5(XE[5], XB[5], XC[5]);
xor XE6(XE[6], XB[6], XC[6]);
xor XE7(XE[7], XB[7], XC[7]);

xor G0(G[0], F[0], F[1]);
xor G1(G[1], F[2], F[3]);
xor G2(G[2], F[0], F[2]);
xor G3(G[3], F[1], F[3]);
xor G4(G[4], F[4], F[5]);
xor G5(G[5], F[6], F[7]);
xor G6(G[6], F[4], F[6]);
xor G7(G[7], F[5], F[7]);

xor XD0(XD[0], G[4], H[0]);
xor XD1(XD[1], G[5], H[1]);
xor XD2(XD[2], G[6], H[2]);
xor XD3(XD[3], G[7], H[3]);
xor XD4(XD[4], G[0], H[4]);
xor XD5(XD[5], G[1], H[5]);
xor XD6(XD[6], G[2], H[6]);
xor XD7(XD[7], G[3], H[7]);

xor S0(S[0], XD[0], XE[0]);
xor S1(S[1], XD[1], XE[1]);
xor S2(S[2], XD[2], XE[2]);
xor S3(S[3], XD[3], XE[3]);
xor S4(S[4], XD[4], XE[4]);
xor S5(S[5], XD[5], XE[5]);
xor S6(S[6], XD[6], XE[6]);
xor S7(S[7], XD[7], XE[7]);

endmodule /* Syndrome */

/*************************************************************************/

module Correction (OD, S, ID);

  input[0:31]   ID;
  input[0:7]    S;
  output[0:31]  OD;

  wire[0:31]    E;
  wire[0:15]    XA;
  wire[0:7]     XB, XC, XD, XE, F, G, H, T, W;
  wire[0:4]     S0B, S1B, S2B, S3B, S4B, S5B, S6B, S7B;
  wire[0:1]     U;

not S0B0(S0B[0], S[0]);
not S0B1(S0B[1], S[0]);
not S0B2(S0B[2], S[0]);
not S0B3(S0B[3], S[0]);
not S0B4(S0B[4], S[0]);
not S1B0(S1B[0], S[1]);
not S1B1(S1B[1], S[1]);
not S1B2(S1B[2], S[1]);
not S1B3(S1B[3], S[1]);
not S1B4(S1B[4], S[1]);
not S2B0(S2B[0], S[2]);
not S2B1(S2B[1], S[2]);
not S2B2(S2B[2], S[2]);
not S2B3(S2B[3], S[2]);
not S2B4(S2B[4], S[2]);
not S3B0(S3B[0], S[3]);
not S3B1(S3B[1], S[3]);
not S3B2(S3B[2], S[3]);
not S3B3(S3B[3], S[3]);
not S3B4(S3B[4], S[3]);
not S4B0(S4B[0], S[4]);
not S4B1(S4B[1], S[4]);
not S4B2(S4B[2], S[4]);
not S4B3(S4B[3], S[4]);
not S4B4(S4B[4], S[4]);
not S5B0(S5B[0], S[5]);
not S5B1(S5B[1], S[5]);
not S5B2(S5B[2], S[5]);
not S5B3(S5B[3], S[5]);
not S5B4(S5B[4], S[5]);
not S6B0(S6B[0], S[6]);
not S6B1(S6B[1], S[6]);
not S6B2(S6B[2], S[6]);
not S6B3(S6B[3], S[6]);
not S6B4(S6B[4], S[6]);
not S7B0(S7B[0], S[7]);
not S7B1(S7B[1], S[7]);
not S7B2(S7B[2], S[7]);
not S7B3(S7B[3], S[7]);
not S7B4(S7B[4], S[7]);

and T0(T[0], S0B[0], S1B[0], S2B[0], S[3]);
and T1(T[1], S0B[1], S1B[1], S[2], S3B[0]);
and T2(T[2], S0B[2], S[1],   S2B[1], S3B[1]);
and T3(T[3], S[0],   S1B[2], S2B[2], S3B[2]);
and T4(T[4], S4B[0], S5B[0], S6B[0], S[7]);
and T5(T[5], S4B[1], S5B[1], S[6], S7B[0]);
and T6(T[6], S4B[2], S[5],   S6B[1], S7B[1]);
and T7(T[7], S[4],   S5B[2], S6B[2], S7B[2]);

or U0(U[0], T[0], T[1], T[2], T[3]);
or U1(U[1], T[4], T[5], T[6], T[7]);

and W0(W[0], S[4], S5B[3], S[6], S7B[3], U[0]);
and W1(W[1], S[4], S5B[4], S6B[3], S[7], U[0]);
and W2(W[2], S4B[3], S[5], S[6], S7B[4], U[0]);
and W3(W[3], S4B[4], S[5], S6B[4], S[7], U[0]);
and W4(W[4], S[0], S1B[3], S[2], S3B[3], U[1]);
and W5(W[5], S[0], S1B[4], S2B[3], S[3], U[1]);
and W6(W[6], S0B[3], S[1], S[2], S3B[4], U[1]);
and W7(W[7], S0B[4], S[1], S2B[4], S[3], U[1]);

and E0(E[0], W[0], S[0]);
and E1(E[1], W[0], S[1]);
and E2(E[2], W[0], S[2]);
and E3(E[3], W[0], S[3]);
and E4(E[4], W[1], S[0]);
and E5(E[5], W[1], S[1]);
and E6(E[6], W[1], S[2]);
and E7(E[7], W[1], S[3]);
and E8(E[8], W[2], S[0]);
and E9(E[9], W[2], S[1]);
and E10(E[10], W[2], S[2]);
and E11(E[11], W[2], S[3]);
and E12(E[12], W[3], S[0]);
and E13(E[13], W[3], S[1]);
and E14(E[14], W[3], S[2]);
and E15(E[15], W[3], S[3]);
and E16(E[16], W[4], S[4]);
and E17(E[17], W[4], S[5]);
and E18(E[18], W[4], S[6]);
and E19(E[19], W[4], S[7]);
and E20(E[20], W[5], S[4]);
and E21(E[21], W[5], S[5]);
and E22(E[22], W[5], S[6]);
and E23(E[23], W[5], S[7]);
and E24(E[24], W[6], S[4]);
and E25(E[25], W[6], S[5]);
and E26(E[26], W[6], S[6]);
and E27(E[27], W[6], S[7]);
and E28(E[28], W[7], S[4]);
and E29(E[29], W[7], S[5]);
and E30(E[30], W[7], S[6]);
and E31(E[31], W[7], S[7]);

xor OD0(OD[0], ID[0], E[0]);
xor OD1(OD[1], ID[1], E[1]);
xor OD2(OD[2], ID[2], E[2]);
xor OD3(OD[3], ID[3], E[3]);
xor OD4(OD[4], ID[4], E[4]);
xor OD5(OD[5], ID[5], E[5]);
xor OD6(OD[6], ID[6], E[6]);
xor OD7(OD[7], ID[7], E[7]);
xor OD8(OD[8], ID[8], E[8]);
xor OD9(OD[9], ID[9], E[9]);
xor OD10(OD[10], ID[10], E[10]);
xor OD11(OD[11], ID[11], E[11]);
xor OD12(OD[12], ID[12], E[12]);
xor OD13(OD[13], ID[13], E[13]);
xor OD14(OD[14], ID[14], E[14]);
xor OD15(OD[15], ID[15], E[15]);
xor OD16(OD[16], ID[16], E[16]);
xor OD17(OD[17], ID[17], E[17]);
xor OD18(OD[18], ID[18], E[18]);
xor OD19(OD[19], ID[19], E[19]);
xor OD20(OD[20], ID[20], E[20]);
xor OD21(OD[21], ID[21], E[21]);
xor OD22(OD[22], ID[22], E[22]);
xor OD23(OD[23], ID[23], E[23]);
xor OD24(OD[24], ID[24], E[24]);
xor OD25(OD[25], ID[25], E[25]);
xor OD26(OD[26], ID[26], E[26]);
xor OD27(OD[27], ID[27], E[27]);
xor OD28(OD[28], ID[28], E[28]);
xor OD29(OD[29], ID[29], E[29]);
xor OD30(OD[30], ID[30], E[30]);
xor OD31(OD[31], ID[31], E[31]);

endmodule /* Correction */

