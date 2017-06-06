/*********** 
# Copyright (c) 2017, Xilinx, Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
# 
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.
#
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY 
# DIRECT, INDIRECT,INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO,PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# 
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
# OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
************/

/*******************************************************************************

    This is simple matrix multiplication example to demonstrate full 2D-array 
    access

*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "mmult.h"


void mmult_accel(int *a, int *b, int *c, int dim) {

    //2D Array is used to store input and output matrices
    int bufa[MAX_MATRIX_DIM][MAX_MATRIX_DIM];
    int bufb[MAX_MATRIX_DIM][MAX_MATRIX_DIM];
    int bufc[MAX_MATRIX_DIM][MAX_MATRIX_DIM];

    int matrix_size = dim*dim;
    // Burst Read data from DDR memory and write into 2D local buffer for a, 
    int x = 0, y = 0;
    read_data_a: for (int i = 0 ; i < matrix_size ; i++){
        #pragma HLS PIPELINE
        #pragma HLS LOOP_TRIPCOUNT min=1 max=65536
        int tmpData_a = a[i];
        int tmpData_b = b[i];
        bufa[x][y] = tmpData_a;
        bufb[x][y] = tmpData_b;
        if (y == dim-1){ x++; y = 0; } else{ y++; }
    }
#if 0
    // Burst Read data from DDR memory and write into 2D local buffer for b, 
    read_data_b: for (int i = 0, x=0, y=0; i < matrix_size ; i++){
        #pragma HLS PIPELINE
        #pragma HLS LOOP_TRIPCOUNT min=1 max=65536
        int tmpData_b = b[i];
        bufb[x][y] = tmpData_b;
        if (y == dim-1){ x++; y = 0;} else{ y++;}
    }
#endif 
    // Calculate matrix multiplication using local data buffers 
    // and write result into local buffer for c
    matrix_mult: for (int row = 0; row < dim; row++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=256
        for (int col = 0; col < dim; col++) {
            #pragma HLS LOOP_TRIPCOUNT min=1 max=256
            int result = 0;
            for (int k = 0; k < dim; k++) {
                #pragma HLS LOOP_TRIPCOUNT min=1 max=256
                #pragma HLS pipeline
                result += bufa[row][k] * bufb[k][col];
            }
            bufc[row][col] = result;
        }
    }
    // Burst Write result to DDR memory from local buffer 
    int m = 0, n = 0;
    write_data: for (int i = 0 ; i < matrix_size ; i++){
        #pragma HLS LOOP_TRIPCOUNT min=1 max=65536
        #pragma HLS PIPELINE
        int tmpData_c = bufc[m][n];
        c[i] = tmpData_c;
        if (n == dim-1){ m++; n = 0; }else{ n++; }
    }
}

