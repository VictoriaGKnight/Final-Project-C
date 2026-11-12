
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <windows.h>

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Pixel;

// Keep color values between 0 and 255
uint8_t clamp(double value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return (uint8_t)(value + 0.5);
}

// Apply a warmer color tone
void apply_warmth(Pixel *input, Pixel *output, int count) {
    for (int i = 0; i < count; i++) {
        output[i].red = clamp(input[i].red * 1.10);
        output[i].green = clamp(input[i].green * 1.02);
        output[i].blue = clamp(input[i].blue * 0.90);
    }
}

// Reduce contrast for a softer appearance
void apply_contrast(Pixel *input, Pixel *output, int count) {
    for (int i = 0; i < count; i++) {
        output[i].red = clamp((input[i].red - 128) * 0.85 + 128);
        output[i].green = clamp((input[i].green - 128) * 0.85 + 128);
        output[i].blue = clamp((input[i].blue - 128) * 0.85 + 128);
    }
}

// Add a faded film appearance
void apply_fade(Pixel *input, Pixel *output, int count) {
    for (int i = 0; i < count; i++) {
        output[i].red = clamp(input[i].red * 0.90 + 20);
        output[i].green = clamp(input[i].green * 0.90 + 20);
        output[i].blue = clamp(input[i].blue * 0.90 + 20);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s input.ppm output.ppm\n", argv[0]);
        return 1;
    }

    FILE *inputFile = fopen(argv[1], "rb");
    if (inputFile == NULL) {
        printf("Error opening input image.\n");
        return 1;
    }

    int width, height, maxColor;
    if (fscanf(inputFile, "P6 %d %d %d", &width, &height, &maxColor) != 3 ||
        width <= 0 || height <= 0 || maxColor != 255 ||
        fgetc(inputFile) == EOF) {
        printf("Invalid PPM image.\n");
        fclose(inputFile);
        return 1;
    }

    size_t count = (size_t)width * height;
    if (count > SIZE_MAX / sizeof(Pixel) || count > INT32_MAX) {
        printf("Image is too large.\n");
        fclose(inputFile);
        return 1;
    }

    Pixel *original = malloc(count * sizeof(Pixel));
    Pixel *warm = malloc(count * sizeof(Pixel));
    Pixel *contrast = malloc(count * sizeof(Pixel));
    Pixel *faded = malloc(count * sizeof(Pixel));

    if (!original || !warm || !contrast || !faded) {
        printf("Memory allocation failed.\n");
        free(original);
        free(warm);
        free(contrast);
        free(faded);
        fclose(inputFile);
        return 1;
    }

    if (fread(original, sizeof(Pixel), count, inputFile) != count) {
        printf("Error reading image pixels.\n");
        free(original);
        free(warm);
        free(contrast);
        free(faded);
        fclose(inputFile);
        return 1;
    }
    fclose(inputFile);

    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    apply_warmth(original, warm, (int)count);
    apply_contrast(warm, contrast, (int)count);
    apply_fade(contrast, faded, (int)count);

    QueryPerformanceCounter(&end);

    double elapsed = (double)(end.QuadPart - start.QuadPart)
                    * 1000.0 / frequency.QuadPart;

    FILE *outputFile = fopen(argv[2], "wb");
    if (outputFile == NULL) {
        printf("Error opening output image.\n");
        free(original);
        free(warm);
        free(contrast);
        free(faded);
        return 1;
    }

    fprintf(outputFile, "P6\n%d %d\n255\n", width, height);
    if (fwrite(faded, sizeof(Pixel), count, outputFile) != count) {
        printf("Error writing image pixels.\n");
        fclose(outputFile);
        free(original);
        free(warm);
        free(contrast);
        free(faded);
        return 1;
    }
    fclose(outputFile);

    printf("Image size: %d x %d\n", width, height);
    printf("Baseline processing time: %.3f ms\n", elapsed);

    free(original);
    free(warm);
    free(contrast);
    free(faded);

    return 0;
}
