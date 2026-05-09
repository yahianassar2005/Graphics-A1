# Computer Graphics Project

## Description

This is a Computer Graphics project implemented in C++ using the Windows API for drawing graphics primitives. The application provides a console-based menu system to perform various graphics operations such as drawing lines, circles, ellipses, curves, filling shapes, and clipping.

## Features

- **File Menu**: Clear screen, save/load shapes data to/from file.
- **Preferences Menu**: Change window background to white, change mouse cursor shape, set shape color.
- **Lines Menu**: Draw lines using DDA, Midpoint, and Parametric algorithms.
- **Circles Menu**: Draw circles using Direct, Polar, Iterative Polar, Midpoint, and Modified Midpoint algorithms.
- **Ellipse Menu**:  Direct, Polar, Midpoint algorithms.
- **Curves Menu**:  Cardinal Spline Curve.
- **Filling Menu**:  Fill circle with lines/circles, fill square with Hermite, fill rectangle with Bezier, convex/non-convex filling, recursive/non-recursive flood fill.
- **Clipping Menu**:  Rectangle and square clipping for points, lines, polygons.
- **Bonus Menu**:  Circle clipping, draw happy/sad faces.

## Build Instructions

Option 1: Direct compile with MinGW/GCC:

```powershell
cd "path"
g++ -g "MainArch.cpp" -o "MainArch.exe" -lgdi32
```

Option 2: Build with CMake:

1. Ensure you have CMake (version 3.28 or higher) and a C++14 compatible compiler installed.
2. Navigate to the project directory.
3. Create a build directory: `mkdir build && cd build`
4. Generate build files: `cmake ..`
5. Build the project: `cmake --build .`

This will create an executable named `MainArch.exe`.
```
.\MainArch.exe
```

## Usage

1. Run the executable from the command line.
2. Follow the console prompts to select menu options:
   - Enter `1` for File Menu
   - Enter `2` for Preferences Menu
   - Enter `3` to start mouse interaction for drawing (currently a placeholder)
3. For file operations, provide filenames as prompted (e.g., "shapes_data.txt").
4. For preferences, follow sub-menu options.

## Requirements

- Windows operating system (uses Windows.h API)
- CMake 3.28+
- C++14 compatible compiler (e.g., Visual Studio, MinGW)

## Project Structure

- `Main Arch.cpp`: Main source file containing all implementations.
- `CMakeLists.txt`: CMake build configuration.
- `cmake-build-debug/`: Build artifacts (generated).
