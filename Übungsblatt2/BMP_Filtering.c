#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#pragma pack(push, 1)

typedef struct {
    uint16_t signature;
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t dataOffset;
} BMPFileHeader;

typedef struct {
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPPM;
    int32_t yPPM;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
} BMPInfoHeader;

#pragma pack(pop)

int kernel_smooth[3][3] = {
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1}
};

int kernel_sharp[3][3] = {
    {0, -1, 0},
    {-1, 5, -1},
    {0, -1, 0}
};

int kernel_edge[3][3] = {
    {0, 1, 0},
    {1, -4, 1},
    {0, 1, 0}
};

int kernel_emboss[3][3] = {
    {2, 1, 0},
    {1, 1, -1},
    {0, -1, -2}
};

uint8_t clamp(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return (uint8_t)value;
}

int getRowSize(int width, int bitsPerPixel) {
    int rowBytes = width * bitsPerPixel / 8;
    int remainder = rowBytes % 4;

    if (remainder == 0) {
        return rowBytes;
    }

    return rowBytes + (4 - remainder);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input.bmp> <filter>\n", argv[0]);
        printf("Filters: smooth, sharp, edge, emboss\n");
        return 1;
    }

    char* inputFile = argv[1];
    char* filterName = argv[2];


    int (*kernel)[3] = NULL;
    int divisor;

    if (strcmp(filterName, "smooth") == 0) {
        kernel = kernel_smooth;
        divisor = 9;
    }
    else if (strcmp(filterName, "sharp") == 0) {
        kernel = kernel_sharp;
        divisor = 1;
    }
    else if (strcmp(filterName, "edge") == 0) {
        kernel = kernel_edge;
        divisor = 1;
    }
    else if (strcmp(filterName, "emboss") == 0) {
        kernel = kernel_emboss;
        divisor = 1;
    }
    else {
        printf("Error: Unknown filter '%s'\n", filterName);
        return 1;
    }

    int fd = open(inputFile, O_RDONLY);
    if (fd == -1) {
        printf("Error: Cannot open file '%s'\n", inputFile);
        return 1;
    }
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    long fileHeaderBytesRead = read(fd, &fileHeader, sizeof(BMPFileHeader));
    if (fileHeaderBytesRead != sizeof(BMPFileHeader)) {
        printf("Error: Cannot read file header\n");
        close(fd);
        return 1;
    }

    long infoHeaderBytesRead = read(fd, &infoHeader, sizeof(BMPInfoHeader));
    if (infoHeaderBytesRead != sizeof(BMPInfoHeader)) {
        printf("Error: Cannot read info header\n");
        close(fd);
        return 1;
    }

    if (fileHeader.signature != 0x4D42) {
        printf("Error: Not a valid BMP file\n");
        close(fd);
        return 1;
    }

    if (infoHeader.bitsPerPixel != 24) {
        printf("Error: Only 24-bit BMP supported\n");
        close(fd);
        return 1;
    }

    if (infoHeader.compression != 0) {
        printf("Error: Compressed BMP not supported\n");
        close(fd);
        return 1;
    }

    if (infoHeader.width < 3 || infoHeader.height < 3) {
        printf("Error: Image must be at least 3x3 pixels\n");
        close(fd);
        return 1;
    }

    int width = infoHeader.width;
    int height = infoHeader.height;
    int rowSize = getRowSize(width, 24);

    uint8_t* pixelData = (uint8_t *)malloc(rowSize * height);
    if (pixelData == NULL) {
        printf("Error: Memory allocation failed\n");
        close(fd);
        return 1;
    }

    lseek(fd, fileHeader.dataOffset, SEEK_SET);
    if (read(fd, pixelData, rowSize * height) != rowSize * height) {
        printf("Error: Cannot read pixel data\n");
        free(pixelData);
        close(fd);
        return 1;
    }

    close(fd);

    int newWidth = width - 2;
    int newHeight = height - 2;
    int newRowSize = getRowSize(newWidth, 24);

    uint8_t* newPixelData = (uint8_t *)malloc(newRowSize * newHeight);
    if (newPixelData == NULL) {
        printf("Error: Memory allocation failed\n");
        free(pixelData);
        return 1;
    }

    memset(newPixelData, 0, newRowSize * newHeight);

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sumBlue = 0;
            int sumGreen = 0;
            int sumRed = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int srcX = x + kx;
                    int srcY = y + ky;

                    uint8_t* srcPixel = pixelData + srcY * rowSize + srcX * 3;

                    int kValue = kernel[1 - ky][1 - kx];

                    sumBlue += srcPixel[0] * kValue;
                    sumGreen += srcPixel[1] * kValue;
                    sumRed += srcPixel[2] * kValue;
                }
            }

            sumBlue = clamp(sumBlue / divisor);
            sumGreen = clamp(sumGreen / divisor);
            sumRed = clamp(sumRed / divisor);

            int destX = x - 1;
            int destY = y - 1;
            uint8_t* destPixel = newPixelData + destY * newRowSize + destX * 3;

            destPixel[0] = sumBlue;
            destPixel[1] = sumGreen;
            destPixel[2] = sumRed;
        }
    }
    char* fileName = strrchr(inputFile, '/');
    if (fileName)
        fileName++;
    else
        fileName = inputFile;

    char baseName[256];
    strcpy(baseName, fileName);
    char* dot = strrchr(baseName, '.');
    if (dot) *dot = '\0';

    char outputFile[256];
    snprintf(outputFile, sizeof(outputFile), "%s_output_%s.bmp", baseName, filterName);

    int fdOut = open(outputFile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fdOut == -1) {
        printf("Error: Cannot create output file '%s'\n", outputFile);
        printf("Reason: %s\n", strerror(errno));
        free(pixelData);
        free(newPixelData);
        return 1;
    }

    BMPFileHeader newFileHeader = fileHeader;
    BMPInfoHeader newInfoHeader = infoHeader;

    newInfoHeader.width = newWidth;
    newInfoHeader.height = newHeight;
    newInfoHeader.imageSize = newRowSize * newHeight;
    newFileHeader.fileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + newInfoHeader.imageSize;

    write(fdOut, &newFileHeader, sizeof(BMPFileHeader));
    write(fdOut, &newInfoHeader, sizeof(BMPInfoHeader));

    write(fdOut, newPixelData, newRowSize * newHeight);

    close(fdOut);

    free(pixelData);
    free(newPixelData);

    printf("Saved: %s\n", outputFile);
    return 0;
}
