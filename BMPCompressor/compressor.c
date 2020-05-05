#include "compressor.h"

int main(int argc, char const *argv[]) {

    long size = 0;
    FILE *file = NULL;
    unsigned char *bmpImage = NULL;

    BMPFILEHEADER *bmpFile = (BMPFILEHEADER *)malloc(14);
    BMPINFOHEADER *bmpInfo = (BMPINFOHEADER *)malloc(40);

    file = fopen("images/cachorro.bmp", "rb"); // Openning image that we want to compress.

    if (file == NULL) { // Checking if there was an error opening the image.
        printf("error reading file");
        return ERROR;
    }

    // Reading the bmp file header and info header so we can read image data without troubles.
    if (!readBMPFileHeader(file, bmpFile) || !readBMPInfoHeader(file, bmpInfo))
        return ERROR;

    // Moving our file pointer to the bitmap data region.
    moveToBitmapData(file, bmpFile);
    
    // We're going to split the RGB channels into these 3 matrices below:
    unsigned char **R = NULL, **G = NULL, **B = NULL;

    // Allocating enough memory to store R, G and B channels.
    R = allocMatrix(R, getHeight(bmpInfo), getWidth(bmpInfo));
    G = allocMatrix(G, getHeight(bmpInfo), getWidth(bmpInfo));
    B = allocMatrix(B, getHeight(bmpInfo), getWidth(bmpInfo));

    // Separates the bitmap data into its RGB components.
    separateComponents(file, bmpInfo, R, G, B);

    // Now we're going to convert from RGB to YCbCr to increase DCT performance.
    float **Y = NULL, **Cb = NULL, **Cr = NULL;

    Y = allocFloatMatrix(Y, getHeight(bmpInfo), getWidth(bmpInfo));
    Cb = allocFloatMatrix(Cb, getHeight(bmpInfo), getWidth(bmpInfo));
    Cr = allocFloatMatrix(Cr, getHeight(bmpInfo), getWidth(bmpInfo));

    rgbToYcbcr(R, G, B, Y, Cb, Cr, getHeight(bmpInfo), getWidth(bmpInfo));

    // Dividing each component into 8x8 matrices in order to use DCT (Discrete Cosine Transform) algorithm,
    // at each 8x8 matrix, due to some researchs proving that this division increases the efficiency of DCT.
    float **dctCoefs = allocFloatMatrix(dctCoefs, getHeight(bmpInfo), getWidth(bmpInfo));

    levelShift(dctCoefs, 128, getHeight(bmpInfo), getWidth(bmpInfo)); // Applying level shift in order to increase DCT performance.

    Y = divideMatrices(Y, dctCoefs, getHeight(bmpInfo), getWidth(bmpInfo));
    Cb = divideMatrices(Cb, dctCoefs, getHeight(bmpInfo), getWidth(bmpInfo));
    Cr = divideMatrices(Cr, dctCoefs, getHeight(bmpInfo), getWidth(bmpInfo));

    // Starting the quantization step. Here we're going to divide our DCT coefficients by
    // the quantization table so we can perform coefficients quantization.
    float **quantCoefs = allocFloatMatrix(quantCoefs, getHeight(bmpInfo), getWidth(bmpInfo));

    Y = quantization(quantCoefs, Y, getHeight(bmpInfo), getWidth(bmpInfo));
    Cb = quantization(quantCoefs, Cb, getHeight(bmpInfo), getWidth(bmpInfo));
    Cr = quantization(quantCoefs, Cr, getHeight(bmpInfo), getWidth(bmpInfo));

    // printf("\n");
    // for (int i = 0; i < getHeight(bmpInfo); i++) {
    //     for (int j = 0; j < getWidth(bmpInfo); j++) {
    //         printf("%.3f ", Y[i][j]);
    //     }
    //     printf("\n");
    // }

    // // On this step, we're going to apply vectorization using zig-zag scan. We do this to
    // // make easier for us to compress the image by moving all the zero values to the end of the vector.
    // // Its told that this step helps to increase run-length encoding performance.
    // // int vectorY[64] = {}, vectorCb[64] = {}, vectorCr[64] = {};

    // // vectorization(vectorY, Y);
    // // vectorization(vectorCb, Cb);
    // // vectorization(vectorCr, Cr);

    // // Applying run-length encoding to each component (Y, Cb and Cr).
    // FILE *rleFile = fopen("compressed", "wb+");

    // // runlength2(vectorY, rleFile);
    // // runlength2(vectorCb, rleFile);
    // // runlength2(vectorCr, rleFile);

    // runlength(Y, getHeight(bmpInfo), getWidth(bmpInfo), rleFile);
    // runlength(Cb, getHeight(bmpInfo), getWidth(bmpInfo), rleFile);
    // runlength(Cr, getHeight(bmpInfo), getWidth(bmpInfo), rleFile);

    // // Free allocated memory.
    // fclose(rleFile);
    // fclose(file);
    // free(bmpFile);
    // free(bmpInfo);
    // free(bmpImage);
    // freeMatrix(R, getHeight(bmpInfo));
    // freeMatrix(G, getHeight(bmpInfo));
    // freeMatrix(B, getHeight(bmpInfo));
    // freeDoubleMatrix(Y, getHeight(bmpInfo));
    // freeDoubleMatrix(Cb, getHeight(bmpInfo));
    // freeDoubleMatrix(Cr, getHeight(bmpInfo));
    // freeDoubleMatrix(dctCoefs, getHeight(bmpInfo));
    // freeDoubleMatrix(quantCoefs, getHeight(bmpInfo));

    return SUCCESS;
}