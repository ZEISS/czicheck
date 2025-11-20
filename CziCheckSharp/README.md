# CziCheckSharp

A .NET wrapper for CZICheck, providing validation and checking capabilities for CZI (Carl Zeiss Image) files through a native P/Invoke interface.

## Overview

CziCheckSharp provides a type-safe C# API for validating CZI files using the native CZICheck library. It wraps the C API (`libczicheckc`) and provides:

- Structured validation results with detailed findings
- Configurable validation checks
- JSON output parsing
- Proper resource management through `IDisposable`
- **Cross-platform support** with native libraries bundled for Windows (x64) and Linux (x64)

## Installation

```bash
dotnet add package m-ringler.CziCheckSharp
```

The NuGet package includes the native libraries for supported platforms, so no additional installation steps are required.

## Quick Start

```csharp
using CziCheckSharp;

// Create a checker with default configuration
var configuration = new Configuration();
using var checker = new CziChecker(configuration);

// Validate a CZI file
var result = checker.Check("sample.czi");

// Check validation status
if (result.OverallResult == "✔")
{
    Console.WriteLine("Validation passed!");
}
else
{
    Console.WriteLine($"Validation issues found:");
    foreach (var checkerResult in result.CheckerResults)
    {
        Console.WriteLine($"  {checkerResult.CheckName}: {checkerResult.Summary}");
    }
}
```

## Platform Support

This package includes native libraries for the following platforms:

- **Windows x64**: `libczicheckc.dll` (included in package)
- **Linux x64**: `libczicheckc.so` (included in package)

The appropriate library is automatically loaded based on your runtime platform. No additional configuration is required.

> **Note:** Other platforms (ARM64, macOS, 32-bit systems) are not currently supported. If you need support for additional platforms, you can build the native library yourself and use a custom DLL import resolver.

## Configuration

Customize which checks to run and how they behave:

```csharp
var configuration = new Configuration
{
    Checks = Checks.Default,           // Or Checks.All, or specific flags
    MaxFindings = 100,                 // Limit findings per check (-1 for unlimited)
    LaxParsing = false,                // Enable tolerant CZI parsing
    IgnoreSizeM = false                // Ignore M dimension for pyramid subblocks
};

using var checker = new CziChecker(configuration);
var result = checker.Check("file.czi");
```

### Available Checks

```csharp
// Individual checks (can be combined with | operator)
Checks.HasValidSubBlockPositions
Checks.HasValidSubBlockSegments
Checks.HasConsistentSubBlockDimensions
Checks.HasNoDuplicateSubBlockCoordinates
Checks.DoesNotUseBIndex
Checks.HasOnlyOnePixelTypePerChannel
Checks.HasPlaneIndicesStartingAtZero
Checks.HasConsecutivePlaneIndices
Checks.AllSubblocksHaveMIndex
Checks.HasBasicallyValidMetadata
Checks.HasXmlSchemaValidMetadata          // Opt-in (requires XercesC)
Checks.HasNoOverlappingScenesAtScale1
Checks.HasValidSubBlockBitmaps            // Opt-in (expensive)
Checks.HasValidApplianceMetadataTopography

// Convenience flags
Checks.Default    // All checks except opt-in
Checks.All        // All available checks
Checks.OptIn      // Only the opt-in checks
```

## Results

The `CziCheckResult` class provides:

```csharp
public class CziCheckResult
{
    public string? OverallResult { get; set; }           // "✔" or "❌"
    public List<CheckerResult> CheckerResults { get; set; }
    public string? Version { get; set; }
    public string? ErrorOutput { get; set; }
}

public class CheckerResult
{
    public string? CheckName { get; set; }
    public string? Summary { get; set; }
    public List<Finding>? Findings { get; set; }
}

public class Finding
{
    public string? Severity { get; set; }
    public string? Information { get; set; }
}
```

## Advanced: Custom Native Library Path

In most cases, the bundled native libraries work automatically. However, if you need to use a custom-built native library or specify a different location, you can use `NativeLibrary.SetDllImportResolver`:

```csharp
using System.Runtime.InteropServices;

NativeLibrary.SetDllImportResolver(typeof(CziChecker).Assembly, (libraryName, assembly, searchPath) =>
{
    if (libraryName == "libczicheckc")
    {
        // Load from custom path
        return NativeLibrary.Load("/custom/path/to/libczicheckc.so");
    }
    return IntPtr.Zero;
});
```

### Building the Native Library

If you need to build the native library yourself (e.g., for unsupported platforms):

```bash
# Clone the repository
git clone https://github.com/ZEISS/czicheck.git
cd czicheck

# Build with CMake (example for Windows)
cmake --preset x64-Debug
cmake --build out/build/x64-Debug

# The library will be in: out/build/x64-Debug/CZICheck/capi/
```

See the [CZICheck building documentation](https://github.com/ZEISS/czicheck/blob/main/documentation/building.md) for more details.

## API Reference

### CziChecker Class

```csharp
public class CziChecker : IDisposable
{
    // Constructor
    public CziChecker(Configuration configuration);
    
    // Methods
    public CziCheckResult Check(string cziFilePath);
    public static string GetVersion();
    
    // Properties
    public Configuration Configuration { get; }
    public bool IsDisposed { get; }
}
```

### Configuration Class

```csharp
public class Configuration
{
    public Checks Checks { get; init; } = Checks.Default;
    public int MaxFindings { get; init; } = -1;
    public bool LaxParsing { get; init; } = false;
    public bool IgnoreSizeM { get; init; } = false;
}
```

## Error Handling

The library handles errors through the `ErrorOutput` property of `CziCheckResult`:

```csharp
var result = checker.Check("file.czi");

if (!string.IsNullOrEmpty(result.ErrorOutput))
{
    Console.WriteLine($"Error: {result.ErrorOutput}");
}
```

Common error scenarios:
- **File not found**: Check the file path
- **DllNotFoundException**: This should not occur with the NuGet package as native libraries are bundled. If it does occur, ensure you're using a supported platform (Windows x64 or Linux x64)
- **Unavailable checks**: Some checks (e.g., `HasXmlSchemaValidMetadata`) may not be available if the native library wasn't compiled with required dependencies

## Version Information

Get the version of the underlying CZICheck library:

```csharp
string version = CziChecker.GetVersion();
Console.WriteLine($"CZICheck version: {version}");
```

## License

This project is licensed under the MIT License - see the [LICENSE](https://github.com/ZEISS/czicheck/blob/main/LICENSE) file for details.

## Links

- [CZICheck Repository](https://github.com/ZEISS/czicheck)
- [CZICheck Documentation](https://github.com/ZEISS/czicheck/tree/main/documentation)
- [Description of Checkers](https://github.com/ZEISS/czicheck/blob/main/documentation/description_of_checkers.md)
- [Version History](https://github.com/ZEISS/czicheck/blob/main/documentation/version-history.md)
