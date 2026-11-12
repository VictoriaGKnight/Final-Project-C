
# Vintage Film Photo Filter

## Project Overview

This project is a vintage film photo filter written in C for my Computer Systems final project. It reads PPM (P6) image files and applies three color adjustments:

- Warmth: increases red and green tones while reducing blue
- Reduced contrast: softens the difference between light and dark colors
- Fade: creates a lighter, vintage appearance

The project compares a baseline implementation with an optimized implementation to evaluate processing speed, memory usage, and generated assembly instructions.

## Program Versions

### Baseline (`baseline.c`)

The baseline processes the image in three separate passes:

1. Apply warmth
2. Reduce contrast
3. Apply fade

It uses four image buffers to store the original image and intermediate results.

### Optimized (`optimized.c`)

The optimized version processes the image in a single pass and uses only two full-image buffers.

It also uses three lookup tables to precompute color transformations. This reduces the floating-point calculations needed while processing individual pixels.

## Requirements

- GCC compiler
- Windows PowerShell
- PPM (P6) image files

## Compilation

Open a terminal in the project directory.

Compile both programs with GCC using the `-O2` optimization flag:

```powershell
gcc -Wall -O2 baseline.c -o baseline
gcc -Wall -O2 optimized.c -o optimized
```

## Running the Programs

Both programs require an input PPM image and an output filename.

Run the baseline:

```powershell
./baseline pink-cosmos-1024.ppm baseline-output.ppm
```

Run the optimized version:

```powershell
./optimized pink-cosmos-1024.ppm optimized-output.ppm
```

Replace `pink-cosmos-1024.ppm` with the name or path of your own PPM (P6) image.

Each program prints the image dimensions and processing time in milliseconds.

## Benchmark Method

Both versions were compiled with GCC using `-O2`.

I tested three image sizes:

- 512 × 512 pixels
- 1024 × 1024 pixels
- 2048 × 2048 pixels

Each program was run once to warm up and then seven additional times for each image size. I used the median processing time to compare the results.

Processing time was measured using Windows `QueryPerformanceCounter()`. The timing excludes reading the input file and writing the output file.

Speedup was calculated as:

Speedup = Baseline Time / Optimized Time

## Benchmark Results

| Image Size | Baseline | Optimized | Speedup |
|---|---:|---:|---:|
| 512 × 512 | 3.152 ms | 0.427 ms | 7.38x |
| 1024 × 1024 | 12.131 ms | 1.962 ms | 6.18x |
| 2048 × 2048 | 50.751 ms | 8.823 ms | 5.75x |

The optimized version was faster across all three image sizes.

Results may vary depending on the computer, compiler, and system activity.

## Output Verification

To verify that both programs produce identical output, compare their SHA-256 file hashes in PowerShell:

```powershell
(Get-FileHash .\baseline-output.ppm).Hash
(Get-FileHash .\optimized-output.ppm).Hash
```

For the 1024 × 1024 test image, both programs produced identical hashes.

## Assembly Analysis

I used Compiler Explorer with x86-64 GCC and the `-O2` optimization flag to compare the generated assembly instructions.

The baseline uses floating-point arithmetic inside its pixel-processing loops. The optimized version performs those calculations while building lookup tables, allowing its main processing loop to primarily use memory-loading and storing instructions.

## Memory Management

The baseline uses four dynamically allocated image buffers, while the optimized version uses two.

Both programs release their image buffers using `free()` when execution completes normally.

## Future Improvements

Future work includes comparing additional compiler optimization levels, investigating memory access patterns, and exploring how different processor architectures affect performance.
