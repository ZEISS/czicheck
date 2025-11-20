# CziCheckSharp.Tests

This project contains unit tests for the CziCheckSharp library, which provides a .NET wrapper around the native CZICheck C API.

## Overview

The tests validate that the C# wrapper correctly calls the native `libczicheckc` library and properly handles validation results. The test suite includes:

- **CziCheckerTests.cs** - Unit tests for the CziChecker class functionality
- **SampleCziTests.cs** - Integration tests that validate actual CZI files against expected results

## Prerequisites

The test project requires the native CZICheck C API library to be built before running tests.

### Native Library Location

The project is configured to copy native libraries from the path specified by the `NativeLibraryPath` MSBuild property in `CziCheckSharp.Tests.csproj`:

```xml
<NativeLibraryPath>..\..\out\build\x64-Debug\CZICheck\capi</NativeLibraryPath>
```

This corresponds to the CMake build output directory when using the `x64-Debug` configuration (see `CMakeSettings.json` in the repository root).

The following files are copied to the test output directory:
- `*.dll` - Native library (Windows)
- `*.so` - Native library (Linux)
- `*.pdb` - Debug symbols (Windows)

### Building the Native Library

Before running the tests, ensure you've built the native CZICheck library:

1. Open the solution in Visual Studio
2. Build the CMake project using the `x64-Debug` configuration
3. The native library will be output to `out\build\x64-Debug\CZICheck\capi\`

Alternatively, use the CMake command line:

```powershell
cmake --preset x64-Debug
cmake --build out/build/x64-Debug
```

### CMake Configuration

The expected build output path is defined in `CMakeSettings.json`:

```json
{
  "name": "x64-Debug",
  "configurationType": "Debug",
  "buildRoot": "${projectDir}\\out\\build\\${name}",
  ...
}
```

If you use a different CMake configuration or build directory, update the `<NativeLibraryPath>` property in `CziCheckSharp.Tests.csproj` to point to your build output directory.

Alternatively, you can override it on the command line without modifying the `.csproj` file:

```bash
dotnet test -p:NativeLibraryPath="..\..\out\build\x64-Release\CZICheck\capi"
```

Or when building:

```bash
dotnet build -p:NativeLibraryPath="path\to\your\native\libs"
```

## Running Tests

```bash
dotnet test
```

Or use Visual Studio's Test Explorer.

## Troubleshooting

**DllNotFoundException**: If you see this error, the native library wasn't found. Ensure:
1. The native CZICheck library has been built
2. The build output path in the `.csproj` matches your CMake build directory
3. The native library files were copied to the test output directory

**Memory Access Violations**: Ensure you're using debug symbols (`.pdb`) and that the native library version matches the C# wrapper's expectations.
