/****************************************************************************
 *                                                                          *
 *  VERILOG HIGH-LEVEL DESCRIPTION OF THE ISCAS-85 BENCHMARK CIRCUIT c1355  *
 *                                                                          *
 *  Function: Single-Error-Correcting Circuit                               *
 *                                                                          *
 *  Written by: Mark C. Hansen                                              *
 *                                                                          *
 *  Last modified: Jan 12, 1998                                             *
 *                                                                          *
 ****************************************************************************/

module Circuit1355 (in1, in5, in9, in13, in17, in21, in25, in29, 
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
      {out724, out725, out726, out727, out728, out729, out730, out731,
       out732, out733, out734, out735, out736, out737, out738, out739, 
       out740, out741, out742, out743, out744, out745, out746, out747, 
       out748, out749, out750, out751, out752, out753, out754, out755}
	    = OD[0:31];

  TopLevel1355 Ckt1355 (ID, IC, R, OD);

endmodule /* Circuit1355 */

/*************************************************************************/

module  TopLevel1355 (ID, IC, R, OD);

  input[0:31]   ID;
  input[0:7]    IC;
  input          R;
  output[0:31]  OD;

  wire[0:7]      S;

  Syndrome M1(S, R, IC, ID);
  Correction M2(OD, S, ID);

endmodule /* TopLevel1355 */

/*************************************************************************/

module Syndrome (S, R, IC, ID);

  input[0:31]   ID;
  input[0:7]    IC;
  input         R;
  output[0:7]   S;
  
  wire[0:15]    XA;
  wire[0:7]     XB, XC, XD, XE, F, G, H;

/* xor XA0(XA[0], ID[0], ID[1]);*/
nand XA0_n1(XA[0]_n1, ID[0], ID[1]);
nand XA0_n2(XA[0]_n2, ID[0], XA[0]_n1);
nand XA0_n3(XA[0]_n3, XA[0]_n1, ID[1]);
nand XA0_n4(XA[0], XA[0]_n2, XA[0]_n3);

/* xor XA1(XA[1], ID[2], ID[3]);*/
nand XA1_n1(XA[1]_n1, ID[2], ID[3]);
nand XA1_n2(XA[1]_n2, ID[2], XA[1]_n1);
nand XA1_n3(XA[1]_n3, XA[1]_n1, ID[3]);
nand XA1_n4(XA[1], XA[1]_n2, XA[1]_n3);

/* xor XA2(XA[2], ID[4], ID[5]);*/
nand XA2_n1(XA[2]_n1, ID[4], ID[5]);
nand XA2_n2(XA[2]_n2, ID[4], XA[2]_n1);
nand XA2_n3(XA[2]_n3, XA[2]_n1, ID[5]);
nand XA2_n4(XA[2], XA[2]_n2, XA[2]_n3);

/* xor XA3(XA[3], ID[6], ID[7]);*/
nand XA3_n1(XA[3]_n1, ID[6], ID[7]);
nand XA3_n2(XA[3]_n2, ID[6], XA[3]_n1);
nand XA3_n3(XA[3]_n3, XA[3]_n1, ID[7]);
nand XA3_n4(XA[3], XA[3]_n2, XA[3]_n3);

/* xor XA4(XA[4], ID[8], ID[9]);*/
nand XA4_n1(XA[4]_n1, ID[8], ID[9]);
nand XA4_n2(XA[4]_n2, ID[8], XA[4]_n1);
nand XA4_n3(XA[4]_n3, XA[4]_n1, ID[9]);
nand XA4_n4(XA[4], XA[4]_n2, XA[4]_n3);

/* xor XA5(XA[5], ID[10], ID[11]);*/
nand XA5_n1(XA[5]_n1, ID[10], ID[11]);
nand XA5_n2(XA[5]_n2, ID[10], XA[5]_n1);
nand XA5_n3(XA[5]_n3, XA[5]_n1, ID[11]);
nand XA5_n4(XA[5], XA[5]_n2, XA[5]_n3);

/* xor XA6(XA[6], ID[12], ID[13]);*/
nand XA6_n1(XA[6]_n1, ID[12], ID[13]);
nand XA6_n2(XA[6]_n2, ID[12], XA[6]_n1);
nand XA6_n3(XA[6]_n3, XA[6]_n1, ID[13]);
nand XA6_n4(XA[6], XA[6]_n2, XA[6]_n3);

/* xor XA7(XA[7], ID[14], ID[15]);*/
nand XA7_n1(XA[7]_n1, ID[14], ID[15]);
nand XA7_n2(XA[7]_n2, ID[14], XA[7]_n1);
nand XA7_n3(XA[7]_n3, XA[7]_n1, ID[15]);
nand XA7_n4(XA[7], XA[7]_n2, XA[7]_n3);

/* xor XA8(XA[8], ID[16], ID[17]);*/
nand XA8_n1(XA[8]_n1, ID[16], ID[17]);
nand XA8_n2(XA[8]_n2, ID[16], XA[8]_n1);
nand XA8_n3(XA[8]_n3, XA[8]_n1, ID[17]);
nand XA8_n4(XA[8], XA[8]_n2, XA[8]_n3);

/* xor XA9(XA[9], ID[18], ID[19]);*/
nand XA9_n1(XA[9]_n1, ID[18], ID[19]);
nand XA9_n2(XA[9]_n2, ID[18], XA[9]_n1);
nand XA9_n3(XA[9]_n3, XA[9]_n1, ID[19]);
nand XA9_n4(XA[9], XA[9]_n2, XA[9]_n3);

/* xor XA10(XA[10], ID[20], ID[21]);*/
nand XA10_n1(XA[10]_n1, ID[20], ID[21]);
nand XA10_n2(XA[10]_n2, ID[20], XA[10]_n1);
nand XA10_n3(XA[10]_n3, XA[10]_n1, ID[21]);
nand XA10_n4(XA[10], XA[10]_n2, XA[10]_n3);

/* xor XA11(XA[11], ID[22], ID[23]);*/
nand XA11_n1(XA[11]_n1, ID[22], ID[23]);
nand XA11_n2(XA[11]_n2, ID[22], XA[11]_n1);
nand XA11_n3(XA[11]_n3, XA[11]_n1, ID[23]);
nand XA11_n4(XA[11], XA[11]_n2, XA[11]_n3);

/* xor XA12(XA[12], ID[24], ID[25]);*/
nand XA12_n1(XA[12]_n1, ID[24], ID[25]);
nand XA12_n2(XA[12]_n2, ID[24], XA[12]_n1);
nand XA12_n3(XA[12]_n3, XA[12]_n1, ID[25]);
nand XA12_n4(XA[12], XA[12]_n2, XA[12]_n3);

/* xor XA13(XA[13], ID[26], ID[27]);*/
nand XA13_n1(XA[13]_n1, ID[26], ID[27]);
nand XA13_n2(XA[13]_n2, ID[26], XA[13]_n1);
nand XA13_n3(XA[13]_n3, XA[13]_n1, ID[27]);
nand XA13_n4(XA[13], XA[13]_n2, XA[13]_n3);

/* xor XA14(XA[14], ID[28], ID[29]);*/
nand XA14_n1(XA[14]_n1, ID[28], ID[29]);
nand XA14_n2(XA[14]_n2, ID[28], XA[14]_n1);
nand XA14_n3(XA[14]_n3, XA[14]_n1, ID[29]);
nand XA14_n4(XA[14], XA[14]_n2, XA[14]_n3);

/* xor XA15(XA[15], ID[30], ID[31]);*/
nand XA15_n1(XA[15]_n1, ID[30], ID[31]);
nand XA15_n2(XA[15]_n2, ID[30], XA[15]_n1);
nand XA15_n3(XA[15]_n3, XA[15]_n1, ID[31]);
nand XA15_n4(XA[15], XA[15]_n2, XA[15]_n3);


/* xor F0(F[0], XA[0], XA[1]);*/
nand F0_n1(F[0]_n1, XA[0], XA[1]);
nand F0_n2(F[0]_n2, XA[0], F[0]_n1);
nand F0_n3(F[0]_n3, F[0]_n1, XA[1]);
nand F0_n4(F[0], F[0]_n2, F[0]_n3);

/* xor F1(F[1], XA[2], XA[3]);*/
nand F1_n1(F[1]_n1, XA[2], XA[3]);
nand F1_n2(F[1]_n2, XA[2], F[1]_n1);
nand F1_n3(F[1]_n3, F[1]_n1, XA[3]);
nand F1_n4(F[1], F[1]_n2, F[1]_n3);

/* xor F2(F[2], XA[4], XA[5]);*/
nand F2_n1(F[2]_n1, XA[4], XA[5]);
nand F2_n2(F[2]_n2, XA[4], F[2]_n1);
nand F2_n3(F[2]_n3, F[2]_n1, XA[5]);
nand F2_n4(F[2], F[2]_n2, F[2]_n3);

/* xor F3(F[3], XA[6], XA[7]);*/
nand F3_n1(F[3]_n1, XA[6], XA[7]);
nand F3_n2(F[3]_n2, XA[6], F[3]_n1);
nand F3_n3(F[3]_n3, F[3]_n1, XA[7]);
nand F3_n4(F[3], F[3]_n2, F[3]_n3);

/* xor F4(F[4], XA[8], XA[9]);*/
nand F4_n1(F[4]_n1, XA[8], XA[9]);
nand F4_n2(F[4]_n2, XA[8], F[4]_n1);
nand F4_n3(F[4]_n3, F[4]_n1, XA[9]);
nand F4_n4(F[4], F[4]_n2, F[4]_n3);

/* xor F5(F[5], XA[10], XA[11]);*/
nand F5_n1(F[5]_n1, XA[10], XA[11]);
nand F5_n2(F[5]_n2, XA[10], F[5]_n1);
nand F5_n3(F[5]_n3, F[5]_n1, XA[11]);
nand F5_n4(F[5], F[5]_n2, F[5]_n3);

/* xor F6(F[6], XA[12], XA[13]);*/
nand F6_n1(F[6]_n1, XA[12], XA[13]);
nand F6_n2(F[6]_n2, XA[12], F[6]_n1);
nand F6_n3(F[6]_n3, F[6]_n1, XA[13]);
nand F6_n4(F[6], F[6]_n2, F[6]_n3);

/* xor F7(F[7], XA[14], XA[15]);*/
nand F7_n1(F[7]_n1, XA[14], XA[15]);
nand F7_n2(F[7]_n2, XA[14], F[7]_n1);
nand F7_n3(F[7]_n3, F[7]_n1, XA[15]);
nand F7_n4(F[7], F[7]_n2, F[7]_n3);


and H0(H[0], IC[0], R);
and H1(H[1], IC[1], R);
and H2(H[2], IC[2], R);
and H3(H[3], IC[3], R);
and H4(H[4], IC[4], R);
and H5(H[5], IC[5], R);
and H6(H[6], IC[6], R);
and H7(H[7], IC[7], R);

/* xor XB0(XB[0], ID[0], ID[4]);*/
nand XB0_n1(XB[0]_n1, ID[0], ID[4]);
nand XB0_n2(XB[0]_n2, ID[0], XB[0]_n1);
nand XB0_n3(XB[0]_n3, XB[0]_n1, ID[4]);
nand XB0_n4(XB[0], XB[0]_n2, XB[0]_n3);

/* xor XB1(XB[1], ID[1], ID[5]);*/
nand XB1_n1(XB[1]_n1, ID[1], ID[5]);
nand XB1_n2(XB[1]_n2, ID[1], XB[1]_n1);
nand XB1_n3(XB[1]_n3, XB[1]_n1, ID[5]);
nand XB1_n4(XB[1], XB[1]_n2, XB[1]_n3);

/* xor XB2(XB[2], ID[2], ID[6]);*/
nand XB2_n1(XB[2]_n1, ID[2], ID[6]);
nand XB2_n2(XB[2]_n2, ID[2], XB[2]_n1);
nand XB2_n3(XB[2]_n3, XB[2]_n1, ID[6]);
nand XB2_n4(XB[2], XB[2]_n2, XB[2]_n3);

/* xor XB3(XB[3], ID[3], ID[7]);*/
nand XB3_n1(XB[3]_n1, ID[3], ID[7]);
nand XB3_n2(XB[3]_n2, ID[3], XB[3]_n1);
nand XB3_n3(XB[3]_n3, XB[3]_n1, ID[7]);
nand XB3_n4(XB[3], XB[3]_n2, XB[3]_n3);

/* xor XB4(XB[4], ID[16], ID[20]);*/
nand XB4_n1(XB[4]_n1, ID[16], ID[20]);
nand XB4_n2(XB[4]_n2, ID[16], XB[4]_n1);
nand XB4_n3(XB[4]_n3, XB[4]_n1, ID[20]);
nand XB4_n4(XB[4], XB[4]_n2, XB[4]_n3);

/* xor XB5(XB[5], ID[17], ID[21]);*/
nand XB5_n1(XB[5]_n1, ID[17], ID[21]);
nand XB5_n2(XB[5]_n2, ID[17], XB[5]_n1);
nand XB5_n3(XB[5]_n3, XB[5]_n1, ID[21]);
nand XB5_n4(XB[5], XB[5]_n2, XB[5]_n3);

/* xor XB6(XB[6], ID[18], ID[22]);*/
nand XB6_n1(XB[6]_n1, ID[18], ID[22]);
nand XB6_n2(XB[6]_n2, ID[18], XB[6]_n1);
nand XB6_n3(XB[6]_n3, XB[6]_n1, ID[22]);
nand XB6_n4(XB[6], XB[6]_n2, XB[6]_n3);

/* xor XB7(XB[7], ID[19], ID[23]);*/
nand XB7_n1(XB[7]_n1, ID[19], ID[23]);
nand XB7_n2(XB[7]_n2, ID[19], XB[7]_n1);
nand XB7_n3(XB[7]_n3, XB[7]_n1, ID[23]);
nand XB7_n4(XB[7], XB[7]_n2, XB[7]_n3);


/* xor XC0(XC[0], ID[8], ID[12]);*/
nand XC0_n1(XC[0]_n1, ID[8], ID[12]);
nand XC0_n2(XC[0]_n2, ID[8], XC[0]_n1);
nand XC0_n3(XC[0]_n3, XC[0]_n1, ID[12]);
nand XC0_n4(XC[0], XC[0]_n2, XC[0]_n3);

/* xor XC1(XC[1], ID[9], ID[13]);*/
nand XC1_n1(XC[1]_n1, ID[9], ID[13]);
nand XC1_n2(XC[1]_n2, ID[9], XC[1]_n1);
nand XC1_n3(XC[1]_n3, XC[1]_n1, ID[13]);
nand XC1_n4(XC[1], XC[1]_n2, XC[1]_n3);

/* xor XC2(XC[2], ID[10], ID[14]);*/
nand XC2_n1(XC[2]_n1, ID[10], ID[14]);
nand XC2_n2(XC[2]_n2, ID[10], XC[2]_n1);
nand XC2_n3(XC[2]_n3, XC[2]_n1, ID[14]);
nand XC2_n4(XC[2], XC[2]_n2, XC[2]_n3);

/* xor XC3(XC[3], ID[11], ID[15]);*/
nand XC3_n1(XC[3]_n1, ID[11], ID[15]);
nand XC3_n2(XC[3]_n2, ID[11], XC[3]_n1);
nand XC3_n3(XC[3]_n3, XC[3]_n1, ID[15]);
nand XC3_n4(XC[3], XC[3]_n2, XC[3]_n3);

/* xor XC4(XC[4], ID[24], ID[28]);*/
nand XC4_n1(XC[4]_n1, ID[24], ID[28]);
nand XC4_n2(XC[4]_n2, ID[24], XC[4]_n1);
nand XC4_n3(XC[4]_n3, XC[4]_n1, ID[28]);
nand XC4_n4(XC[4], XC[4]_n2, XC[4]_n3);

/* xor XC5(XC[5], ID[25], ID[29]);*/
nand XC5_n1(XC[5]_n1, ID[25], ID[29]);
nand XC5_n2(XC[5]_n2, ID[25], XC[5]_n1);
nand XC5_n3(XC[5]_n3, XC[5]_n1, ID[29]);
nand XC5_n4(XC[5], XC[5]_n2, XC[5]_n3);

/* xor XC6(XC[6], ID[26], ID[30]);*/
nand XC6_n1(XC[6]_n1, ID[26], ID[30]);
nand XC6_n2(XC[6]_n2, ID[26], XC[6]_n1);
nand XC6_n3(XC[6]_n3, XC[6]_n1, ID[30]);
nand XC6_n4(XC[6], XC[6]_n2, XC[6]_n3);

/* xor XC7(XC[7], ID[27], ID[31]);*/
nand XC7_n1(XC[7]_n1, ID[27], ID[31]);
nand XC7_n2(XC[7]_n2, ID[27], XC[7]_n1);
nand XC7_n3(XC[7]_n3, XC[7]_n1, ID[31]);
nand XC7_n4(XC[7], XC[7]_n2, XC[7]_n3);


/* xor XE0(XE[0], XB[0], XC[0]);*/
nand XE0_n1(XE[0]_n1, XB[0], XC[0]);
nand XE0_n2(XE[0]_n2, XB[0], XE[0]_n1);
nand XE0_n3(XE[0]_n3, XE[0]_n1, XC[0]);
nand XE0_n4(XE[0], XE[0]_n2, XE[0]_n3);

/* xor XE1(XE[1], XB[1], XC[1]);*/
nand XE1_n1(XE[1]_n1, XB[1], XC[1]);
nand XE1_n2(XE[1]_n2, XB[1], XE[1]_n1);
nand XE1_n3(XE[1]_n3, XE[1]_n1, XC[1]);
nand XE1_n4(XE[1], XE[1]_n2, XE[1]_n3);

/* xor XE2(XE[2], XB[2], XC[2]);*/
nand XE2_n1(XE[2]_n1, XB[2], XC[2]);
nand XE2_n2(XE[2]_n2, XB[2], XE[2]_n1);
nand XE2_n3(XE[2]_n3, XE[2]_n1, XC[2]);
nand XE2_n4(XE[2], XE[2]_n2, XE[2]_n3);

/* xor XE3(XE[3], XB[3], XC[3]);*/
nand XE3_n1(XE[3]_n1, XB[3], XC[3]);
nand XE3_n2(XE[3]_n2, XB[3], XE[3]_n1);
nand XE3_n3(XE[3]_n3, XE[3]_n1, XC[3]);
nand XE3_n4(XE[3], XE[3]_n2, XE[3]_n3);

/* xor XE4(XE[4], XB[4], XC[4]);*/
nand XE4_n1(XE[4]_n1, XB[4], XC[4]);
nand XE4_n2(XE[4]_n2, XB[4], XE[4]_n1);
nand XE4_n3(XE[4]_n3, XE[4]_n1, XC[4]);
nand XE4_n4(XE[4], XE[4]_n2, XE[4]_n3);

/* xor XE5(XE[5], XB[5], XC[5]);*/
nand XE5_n1(XE[5]_n1, XB[5], XC[5]);
nand XE5_n2(XE[5]_n2, XB[5], XE[5]_n1);
nand XE5_n3(XE[5]_n3, XE[5]_n1, XC[5]);
nand XE5_n4(XE[5], XE[5]_n2, XE[5]_n3);

/* xor XE6(XE[6], XB[6], XC[6]);*/
nand XE6_n1(XE[6]_n1, XB[6], XC[6]);
nand XE6_n2(XE[6]_n2, XB[6], XE[6]_n1);
nand XE6_n3(XE[6]_n3, XE[6]_n1, XC[6]);
nand XE6_n4(XE[6], XE[6]_n2, XE[6]_n3);

/* xor XE7(XE[7], XB[7], XC[7]);*/
nand XE7_n1(XE[7]_n1, XB[7], XC[7]);
nand XE7_n2(XE[7]_n2, XB[7], XE[7]_n1);
nand XE7_n3(XE[7]_n3, XE[7]_n1, XC[7]);
nand XE7_n4(XE[7], XE[7]_n2, XE[7]_n3);


/* xor G0(G[0], F[0], F[1]);*/
nand G0_n1(G[0]_n1, F[0], F[1]);
nand G0_n2(G[0]_n2, F[0], G[0]_n1);
nand G0_n3(G[0]_n3, G[0]_n1, F[1]);
nand G0_n4(G[0], G[0]_n2, G[0]_n3);

/* xor G1(G[1], F[2], F[3]);*/
nand G1_n1(G[1]_n1, F[2], F[3]);
nand G1_n2(G[1]_n2, F[2], G[1]_n1);
nand G1_n3(G[1]_n3, G[1]_n1, F[3]);
nand G1_n4(G[1], G[1]_n2, G[1]_n3);

/* xor G2(G[2], F[0], F[2]);*/
nand G2_n1(G[2]_n1, F[0], F[2]);
nand G2_n2(G[2]_n2, F[0], G[2]_n1);
nand G2_n3(G[2]_n3, G[2]_n1, F[2]);
nand G2_n4(G[2], G[2]_n2, G[2]_n3);

/* xor G3(G[3], F[1], F[3]);*/
nand G3_n1(G[3]_n1, F[1], F[3]);
nand G3_n2(G[3]_n2, F[1], G[3]_n1);
nand G3_n3(G[3]_n3, G[3]_n1, F[3]);
nand G3_n4(G[3], G[3]_n2, G[3]_n3);

/* xor G4(G[4], F[4], F[5]);*/
nand G4_n1(G[4]_n1, F[4], F[5]);
nand G4_n2(G[4]_n2, F[4], G[4]_n1);
nand G4_n3(G[4]_n3, G[4]_n1, F[5]);
nand G4_n4(G[4], G[4]_n2, G[4]_n3);

/* xor G5(G[5], F[6], F[7]);*/
nand G5_n1(G[5]_n1, F[6], F[7]);
nand G5_n2(G[5]_n2, F[6], G[5]_n1);
nand G5_n3(G[5]_n3, G[5]_n1, F[7]);
nand G5_n4(G[5], G[5]_n2, G[5]_n3);

/* xor G6(G[6], F[4], F[6]);*/
nand G6_n1(G[6]_n1, F[4], F[6]);
nand G6_n2(G[6]_n2, F[4], G[6]_n1);
nand G6_n3(G[6]_n3, G[6]_n1, F[6]);
nand G6_n4(G[6], G[6]_n2, G[6]_n3);

/* xor G7(G[7], F[5], F[7]);*/
nand G7_n1(G[7]_n1, F[5], F[7]);
nand G7_n2(G[7]_n2, F[5], G[7]_n1);
nand G7_n3(G[7]_n3, G[7]_n1, F[7]);
nand G7_n4(G[7], G[7]_n2, G[7]_n3);


/* xor XD0(XD[0], G[4], H[0]);*/
nand XD0_n1(XD[0]_n1, G[4], H[0]);
nand XD0_n2(XD[0]_n2, G[4], XD[0]_n1);
nand XD0_n3(XD[0]_n3, XD[0]_n1, H[0]);
nand XD0_n4(XD[0], XD[0]_n2, XD[0]_n3);

/* xor XD1(XD[1], G[5], H[1]);*/
nand XD1_n1(XD[1]_n1, G[5], H[1]);
nand XD1_n2(XD[1]_n2, G[5], XD[1]_n1);
nand XD1_n3(XD[1]_n3, XD[1]_n1, H[1]);
nand XD1_n4(XD[1], XD[1]_n2, XD[1]_n3);

/* xor XD2(XD[2], G[6], H[2]);*/
nand XD2_n1(XD[2]_n1, G[6], H[2]);
nand XD2_n2(XD[2]_n2, G[6], XD[2]_n1);
nand XD2_n3(XD[2]_n3, XD[2]_n1, H[2]);
nand XD2_n4(XD[2], XD[2]_n2, XD[2]_n3);

/* xor XD3(XD[3], G[7], H[3]);*/
nand XD3_n1(XD[3]_n1, G[7], H[3]);
nand XD3_n2(XD[3]_n2, G[7], XD[3]_n1);
nand XD3_n3(XD[3]_n3, XD[3]_n1, H[3]);
nand XD3_n4(XD[3], XD[3]_n2, XD[3]_n3);

/* xor XD4(XD[4], G[0], H[4]);*/
nand XD4_n1(XD[4]_n1, G[0], H[4]);
nand XD4_n2(XD[4]_n2, G[0], XD[4]_n1);
nand XD4_n3(XD[4]_n3, XD[4]_n1, H[4]);
nand XD4_n4(XD[4], XD[4]_n2, XD[4]_n3);

/* xor XD5(XD[5], G[1], H[5]);*/
nand XD5_n1(XD[5]_n1, G[1], H[5]);
nand XD5_n2(XD[5]_n2, G[1], XD[5]_n1);
nand XD5_n3(XD[5]_n3, XD[5]_n1, H[5]);
nand XD5_n4(XD[5], XD[5]_n2, XD[5]_n3);

/* xor XD6(XD[6], G[2], H[6]);*/
nand XD6_n1(XD[6]_n1, G[2], H[6]);
nand XD6_n2(XD[6]_n2, G[2], XD[6]_n1);
nand XD6_n3(XD[6]_n3, XD[6]_n1, H[6]);
nand XD6_n4(XD[6], XD[6]_n2, XD[6]_n3);

/* xor XD7(XD[7], G[3], H[7]);*/
nand XD7_n1(XD[7]_n1, G[3], H[7]);
nand XD7_n2(XD[7]_n2, G[3], XD[7]_n1);
nand XD7_n3(XD[7]_n3, XD[7]_n1, H[7]);
nand XD7_n4(XD[7], XD[7]_n2, XD[7]_n3);


/* xor S0(S[0], XD[0], XE[0]);*/
nand S0_n1(S[0]_n1, XD[0], XE[0]);
nand S0_n2(S[0]_n2, XD[0], S[0]_n1);
nand S0_n3(S[0]_n3, S[0]_n1, XE[0]);
nand S0_n4(S[0], S[0]_n2, S[0]_n3);

/* xor S1(S[1], XD[1], XE[1]);*/
nand S1_n1(S[1]_n1, XD[1], XE[1]);
nand S1_n2(S[1]_n2, XD[1], S[1]_n1);
nand S1_n3(S[1]_n3, S[1]_n1, XE[1]);
nand S1_n4(S[1], S[1]_n2, S[1]_n3);

/* xor S2(S[2], XD[2], XE[2]);*/
nand S2_n1(S[2]_n1, XD[2], XE[2]);
nand S2_n2(S[2]_n2, XD[2], S[2]_n1);
nand S2_n3(S[2]_n3, S[2]_n1, XE[2]);
nand S2_n4(S[2], S[2]_n2, S[2]_n3);

/* xor S3(S[3], XD[3], XE[3]);*/
nand S3_n1(S[3]_n1, XD[3], XE[3]);
nand S3_n2(S[3]_n2, XD[3], S[3]_n1);
nand S3_n3(S[3]_n3, S[3]_n1, XE[3]);
nand S3_n4(S[3], S[3]_n2, S[3]_n3);

/* xor S4(S[4], XD[4], XE[4]);*/
nand S4_n1(S[4]_n1, XD[4], XE[4]);
nand S4_n2(S[4]_n2, XD[4], S[4]_n1);
nand S4_n3(S[4]_n3, S[4]_n1, XE[4]);
nand S4_n4(S[4], S[4]_n2, S[4]_n3);

/* xor S5(S[5], XD[5], XE[5]);*/
nand S5_n1(S[5]_n1, XD[5], XE[5]);
nand S5_n2(S[5]_n2, XD[5], S[5]_n1);
nand S5_n3(S[5]_n3, S[5]_n1, XE[5]);
nand S5_n4(S[5], S[5]_n2, S[5]_n3);

/* xor S6(S[6], XD[6], XE[6]);*/
nand S6_n1(S[6]_n1, XD[6], XE[6]);
nand S6_n2(S[6]_n2, XD[6], S[6]_n1);
nand S6_n3(S[6]_n3, S[6]_n1, XE[6]);
nand S6_n4(S[6], S[6]_n2, S[6]_n3);

/* xor S7(S[7], XD[7], XE[7]);*/
nand S7_n1(S[7]_n1, XD[7], XE[7]);
nand S7_n2(S[7]_n2, XD[7], S[7]_n1);
nand S7_n3(S[7]_n3, S[7]_n1, XE[7]);
nand S7_n4(S[7], S[7]_n2, S[7]_n3);


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

/* xor OD0(OD[0], ID[0], E[0]);*/
nand OD0_n1(OD[0]_n1, ID[0], E[0]);
nand OD0_n2(OD[0]_n2, ID[0], OD[0]_n1);
nand OD0_n3(OD[0]_n3, OD[0]_n1, E[0]);
nand OD0_n4(OD[0], OD[0]_n2, OD[0]_n3);

/* xor OD1(OD[1], ID[1], E[1]);*/
nand OD1_n1(OD[1]_n1, ID[1], E[1]);
nand OD1_n2(OD[1]_n2, ID[1], OD[1]_n1);
nand OD1_n3(OD[1]_n3, OD[1]_n1, E[1]);
nand OD1_n4(OD[1], OD[1]_n2, OD[1]_n3);

/* xor OD2(OD[2], ID[2], E[2]);*/
nand OD2_n1(OD[2]_n1, ID[2], E[2]);
nand OD2_n2(OD[2]_n2, ID[2], OD[2]_n1);
nand OD2_n3(OD[2]_n3, OD[2]_n1, E[2]);
nand OD2_n4(OD[2], OD[2]_n2, OD[2]_n3);

/* xor OD3(OD[3], ID[3], E[3]);*/
nand OD3_n1(OD[3]_n1, ID[3], E[3]);
nand OD3_n2(OD[3]_n2, ID[3], OD[3]_n1);
nand OD3_n3(OD[3]_n3, OD[3]_n1, E[3]);
nand OD3_n4(OD[3], OD[3]_n2, OD[3]_n3);

/* xor OD4(OD[4], ID[4], E[4]);*/
nand OD4_n1(OD[4]_n1, ID[4], E[4]);
nand OD4_n2(OD[4]_n2, ID[4], OD[4]_n1);
nand OD4_n3(OD[4]_n3, OD[4]_n1, E[4]);
nand OD4_n4(OD[4], OD[4]_n2, OD[4]_n3);

/* xor OD5(OD[5], ID[5], E[5]);*/
nand OD5_n1(OD[5]_n1, ID[5], E[5]);
nand OD5_n2(OD[5]_n2, ID[5], OD[5]_n1);
nand OD5_n3(OD[5]_n3, OD[5]_n1, E[5]);
nand OD5_n4(OD[5], OD[5]_n2, OD[5]_n3);

/* xor OD6(OD[6], ID[6], E[6]);*/
nand OD6_n1(OD[6]_n1, ID[6], E[6]);
nand OD6_n2(OD[6]_n2, ID[6], OD[6]_n1);
nand OD6_n3(OD[6]_n3, OD[6]_n1, E[6]);
nand OD6_n4(OD[6], OD[6]_n2, OD[6]_n3);

/* xor OD7(OD[7], ID[7], E[7]);*/
nand OD7_n1(OD[7]_n1, ID[7], E[7]);
nand OD7_n2(OD[7]_n2, ID[7], OD[7]_n1);
nand OD7_n3(OD[7]_n3, OD[7]_n1, E[7]);
nand OD7_n4(OD[7], OD[7]_n2, OD[7]_n3);

/* xor OD8(OD[8], ID[8], E[8]);*/
nand OD8_n1(OD[8]_n1, ID[8], E[8]);
nand OD8_n2(OD[8]_n2, ID[8], OD[8]_n1);
nand OD8_n3(OD[8]_n3, OD[8]_n1, E[8]);
nand OD8_n4(OD[8], OD[8]_n2, OD[8]_n3);

/* xor OD9(OD[9], ID[9], E[9]);*/
nand OD9_n1(OD[9]_n1, ID[9], E[9]);
nand OD9_n2(OD[9]_n2, ID[9], OD[9]_n1);
nand OD9_n3(OD[9]_n3, OD[9]_n1, E[9]);
nand OD9_n4(OD[9], OD[9]_n2, OD[9]_n3);

/* xor OD10(OD[10], ID[10], E[10]);*/
nand OD10_n1(OD[10]_n1, ID[10], E[10]);
nand OD10_n2(OD[10]_n2, ID[10], OD[10]_n1);
nand OD10_n3(OD[10]_n3, OD[10]_n1, E[10]);
nand OD10_n4(OD[10], OD[10]_n2, OD[10]_n3);

/* xor OD11(OD[11], ID[11], E[11]);*/
nand OD11_n1(OD[11]_n1, ID[11], E[11]);
nand OD11_n2(OD[11]_n2, ID[11], OD[11]_n1);
nand OD11_n3(OD[11]_n3, OD[11]_n1, E[11]);
nand OD11_n4(OD[11], OD[11]_n2, OD[11]_n3);

/* xor OD12(OD[12], ID[12], E[12]);*/
nand OD12_n1(OD[12]_n1, ID[12], E[12]);
nand OD12_n2(OD[12]_n2, ID[12], OD[12]_n1);
nand OD12_n3(OD[12]_n3, OD[12]_n1, E[12]);
nand OD12_n4(OD[12], OD[12]_n2, OD[12]_n3);

/* xor OD13(OD[13], ID[13], E[13]);*/
nand OD13_n1(OD[13]_n1, ID[13], E[13]);
nand OD13_n2(OD[13]_n2, ID[13], OD[13]_n1);
nand OD13_n3(OD[13]_n3, OD[13]_n1, E[13]);
nand OD13_n4(OD[13], OD[13]_n2, OD[13]_n3);

/* xor OD14(OD[14], ID[14], E[14]);*/
nand OD14_n1(OD[14]_n1, ID[14], E[14]);
nand OD14_n2(OD[14]_n2, ID[14], OD[14]_n1);
nand OD14_n3(OD[14]_n3, OD[14]_n1, E[14]);
nand OD14_n4(OD[14], OD[14]_n2, OD[14]_n3);

/* xor OD15(OD[15], ID[15], E[15]);*/
nand OD15_n1(OD[15]_n1, ID[15], E[15]);
nand OD15_n2(OD[15]_n2, ID[15], OD[15]_n1);
nand OD15_n3(OD[15]_n3, OD[15]_n1, E[15]);
nand OD15_n4(OD[15], OD[15]_n2, OD[15]_n3);

/* xor OD16(OD[16], ID[16], E[16]);*/
nand OD16_n1(OD[16]_n1, ID[16], E[16]);
nand OD16_n2(OD[16]_n2, ID[16], OD[16]_n1);
nand OD16_n3(OD[16]_n3, OD[16]_n1, E[16]);
nand OD16_n4(OD[16], OD[16]_n2, OD[16]_n3);

/* xor OD17(OD[17], ID[17], E[17]);*/
nand OD17_n1(OD[17]_n1, ID[17], E[17]);
nand OD17_n2(OD[17]_n2, ID[17], OD[17]_n1);
nand OD17_n3(OD[17]_n3, OD[17]_n1, E[17]);
nand OD17_n4(OD[17], OD[17]_n2, OD[17]_n3);

/* xor OD18(OD[18], ID[18], E[18]);*/
nand OD18_n1(OD[18]_n1, ID[18], E[18]);
nand OD18_n2(OD[18]_n2, ID[18], OD[18]_n1);
nand OD18_n3(OD[18]_n3, OD[18]_n1, E[18]);
nand OD18_n4(OD[18], OD[18]_n2, OD[18]_n3);

/* xor OD19(OD[19], ID[19], E[19]);*/
nand OD19_n1(OD[19]_n1, ID[19], E[19]);
nand OD19_n2(OD[19]_n2, ID[19], OD[19]_n1);
nand OD19_n3(OD[19]_n3, OD[19]_n1, E[19]);
nand OD19_n4(OD[19], OD[19]_n2, OD[19]_n3);

/* xor OD20(OD[20], ID[20], E[20]);*/
nand OD20_n1(OD[20]_n1, ID[20], E[20]);
nand OD20_n2(OD[20]_n2, ID[20], OD[20]_n1);
nand OD20_n3(OD[20]_n3, OD[20]_n1, E[20]);
nand OD20_n4(OD[20], OD[20]_n2, OD[20]_n3);

/* xor OD21(OD[21], ID[21], E[21]);*/
nand OD21_n1(OD[21]_n1, ID[21], E[21]);
nand OD21_n2(OD[21]_n2, ID[21], OD[21]_n1);
nand OD21_n3(OD[21]_n3, OD[21]_n1, E[21]);
nand OD21_n4(OD[21], OD[21]_n2, OD[21]_n3);

/* xor OD22(OD[22], ID[22], E[22]);*/
nand OD22_n1(OD[22]_n1, ID[22], E[22]);
nand OD22_n2(OD[22]_n2, ID[22], OD[22]_n1);
nand OD22_n3(OD[22]_n3, OD[22]_n1, E[22]);
nand OD22_n4(OD[22], OD[22]_n2, OD[22]_n3);

/* xor OD23(OD[23], ID[23], E[23]);*/
nand OD23_n1(OD[23]_n1, ID[23], E[23]);
nand OD23_n2(OD[23]_n2, ID[23], OD[23]_n1);
nand OD23_n3(OD[23]_n3, OD[23]_n1, E[23]);
nand OD23_n4(OD[23], OD[23]_n2, OD[23]_n3);

/* xor OD24(OD[24], ID[24], E[24]);*/
nand OD24_n1(OD[24]_n1, ID[24], E[24]);
nand OD24_n2(OD[24]_n2, ID[24], OD[24]_n1);
nand OD24_n3(OD[24]_n3, OD[24]_n1, E[24]);
nand OD24_n4(OD[24], OD[24]_n2, OD[24]_n3);

/* xor OD25(OD[25], ID[25], E[25]);*/
nand OD25_n1(OD[25]_n1, ID[25], E[25]);
nand OD25_n2(OD[25]_n2, ID[25], OD[25]_n1);
nand OD25_n3(OD[25]_n3, OD[25]_n1, E[25]);
nand OD25_n4(OD[25], OD[25]_n2, OD[25]_n3);

/* xor OD26(OD[26], ID[26], E[26]);*/
nand OD26_n1(OD[26]_n1, ID[26], E[26]);
nand OD26_n2(OD[26]_n2, ID[26], OD[26]_n1);
nand OD26_n3(OD[26]_n3, OD[26]_n1, E[26]);
nand OD26_n4(OD[26], OD[26]_n2, OD[26]_n3);

/* xor OD27(OD[27], ID[27], E[27]);*/
nand OD27_n1(OD[27]_n1, ID[27], E[27]);
nand OD27_n2(OD[27]_n2, ID[27], OD[27]_n1);
nand OD27_n3(OD[27]_n3, OD[27]_n1, E[27]);
nand OD27_n4(OD[27], OD[27]_n2, OD[27]_n3);

/* xor OD28(OD[28], ID[28], E[28]);*/
nand OD28_n1(OD[28]_n1, ID[28], E[28]);
nand OD28_n2(OD[28]_n2, ID[28], OD[28]_n1);
nand OD28_n3(OD[28]_n3, OD[28]_n1, E[28]);
nand OD28_n4(OD[28], OD[28]_n2, OD[28]_n3);

/* xor OD29(OD[29], ID[29], E[29]);*/
nand OD29_n1(OD[29]_n1, ID[29], E[29]);
nand OD29_n2(OD[29]_n2, ID[29], OD[29]_n1);
nand OD29_n3(OD[29]_n3, OD[29]_n1, E[29]);
nand OD29_n4(OD[29], OD[29]_n2, OD[29]_n3);

/* xor OD30(OD[30], ID[30], E[30]);*/
nand OD30_n1(OD[30]_n1, ID[30], E[30]);
nand OD30_n2(OD[30]_n2, ID[30], OD[30]_n1);
nand OD30_n3(OD[30]_n3, OD[30]_n1, E[30]);
nand OD30_n4(OD[30], OD[30]_n2, OD[30]_n3);

/* xor OD31(OD[31], ID[31], E[31]);*/
nand OD31_n1(OD[31]_n1, ID[31], E[31]);
nand OD31_n2(OD[31]_n2, ID[31], OD[31]_n1);
nand OD31_n3(OD[31]_n3, OD[31]_n1, E[31]);
nand OD31_n4(OD[31], OD[31]_n2, OD[31]_n3);


endmodule /* Correction */

