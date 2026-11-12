
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

void build_lookup_table(uint8_t table[256], double warmth) {
    for (int i = 0; i < 256; i++) {
        // Apply warmth
        uint8_t value = clamp(i * warmth);

        // Reduce contrast
        value = clamp((value - 128) * 0.85 + 128);

        // Apply fade
        table[i] = clamp(value * 0.90 + 20);
    }
}

// Apply all three adjustments in a single pass

void apply_vintage_filter(Pixel *input, Pixel *output, int count) {
    uint8_t redTable[256];
    uint8_t greenTable[256];
    uint8_t blueTable[256];

    // Precompute all possible color transformations
    build_lookup_table(redTable, 1.10);
    build_lookup_table(greenTable, 1.02);
    build_lookup_table(blueTable, 0.90);

    // Process the entire image in one pass
    for (int i = 0; i < count; i++) {
        output[i].red = redTable[input[i].red];
        output[i].green = greenTable[input[i].green];
        output[i].blue = blueTable[input[i].blue];
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
    Pixel *filtered = malloc(count * sizeof(Pixel));

    if (!original || !filtered) {
        printf("Memory allocation failed.\n");
        free(original);
        free(filtered);
        fclose(inputFile);
        return 1;
    }

    if (fread(original, sizeof(Pixel), count, inputFile) != count) {
        printf("Error reading image pixels.\n");
        free(original);
        free(filtered);
        fclose(inputFile);
        return 1;
    }
    fclose(inputFile);

    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    apply_vintage_filter(original, filtered, (int)count);

    QueryPerformanceCounter(&end);

    double elapsed = (double)(end.QuadPart - start.QuadPart)
                     * 1000.0 / frequency.QuadPart;

    FILE *outputFile = fopen(argv[2], "wb");
    if (outputFile == NULL) {
        printf("Error opening output image.\n");
        free(original);
        free(filtered);
        return 1;
    }

    fprintf(outputFile, "P6\n%d %d\n255\n", width, height);
    if (fwrite(filtered, sizeof(Pixel), count, outputFile) != count) {
        printf("Error writing image pixels.\n");
        fclose(outputFile);
        free(original);
        free(filtered);
        return 1;
    }
    fclose(outputFile);

    printf("Image size: %d x %d\n", width, height);
    printf("Optimized processing time: %.3f ms\n", elapsed);

    free(original);
    free(filtered);

    return 0;
}
