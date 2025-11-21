# Simulate the Windows build-native job for czicheck-sharp workflow

$ErrorActionPreference = "Stop"

Write-Information "=== CziCheckSharp Workflow Simulation (Windows) ===" -InformationAction Continue

# Step 0: Initialize Visual Studio Developer Environment
Write-Information "`n[0/7] Initializing Visual Studio Developer Environment..." -InformationAction Continue
Import-Module "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell 8a781a1b -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"
Write-Information "Visual Studio Developer Environment loaded" -InformationAction Continue

# Step 1: Prep vcpkg (simplified - assumes you have vcpkg)
Write-Information "`n[1/7] Checking vcpkg..." -InformationAction Continue
$VCPKG_DIR = "V:\Repos\github\m-ringler\czicheck\external\vcpkg"
if (-not (Test-Path "$VCPKG_DIR\vcpkg.exe")) {
    Write-Information "ERROR: vcpkg not found at $VCPKG_DIR" -InformationAction Continue
    Write-Information "Please bootstrap vcpkg first" -InformationAction Continue
    exit 1
}
Write-Information "vcpkg found at: $VCPKG_DIR" -InformationAction Continue

# Step 2: Configure CMake
Write-Information "`n[2/7] Configuring CMake with static triplet..." -InformationAction Continue
cmake -B "build-test" -DCMAKE_BUILD_TYPE=Release `
      -DCMAKE_TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake" `
      -DVCPKG_TARGET_TRIPLET=x64-windows-static

if ($LASTEXITCODE -ne 0) {
    Write-Information "ERROR: CMake configuration failed" -InformationAction Continue
    exit $LASTEXITCODE
}
Write-Information "CMake configuration successful" -InformationAction Continue

# Step 3: Build
Write-Information "`n[3/7] Building native library..." -InformationAction Continue
cmake --build build-test --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Information "ERROR: Build failed" -InformationAction Continue
    exit $LASTEXITCODE
}
Write-Information "Build successful" -InformationAction Continue

# Step 4: Package native libraries
Write-Information "`n[4/7] Packaging native libraries..." -InformationAction Continue
New-Item -ItemType Directory -Force -Path "native-artifacts" | Out-Null

if (-not (Test-Path "build-test/CZICheck/capi/Release/libczicheckc.dll")) {
    Write-Information "ERROR: libczicheckc.dll not found" -InformationAction Continue
    exit 1
}

Copy-Item "build-test/CZICheck/capi/Release/libczicheckc.dll" "native-artifacts/"
Write-Information "Copied: libczicheckc.dll" -InformationAction Continue

if (Test-Path "build-test/CZICheck/capi/Release/libczicheckc.pdb") {
    Copy-Item "build-test/CZICheck/capi/Release/libczicheckc.pdb" "native-artifacts/"
    Write-Information "Copied: libczicheckc.pdb" -InformationAction Continue
}

Write-Information "`nNative artifacts:" -InformationAction Continue
Get-ChildItem "native-artifacts/" | Format-Table Name, Length, LastWriteTime

# Step 5: Run tests
Write-Information "`n[5/7] Running tests..." -InformationAction Continue
dotnet test CziCheckSharp.Tests/CziCheckSharp.Tests.csproj `
    --configuration Release `
    -p:CziCheckNativeLibraryPath="$PWD/native-artifacts"

if ($LASTEXITCODE -ne 0) {
    Write-Information "ERROR: Tests failed" -InformationAction Continue
    exit $LASTEXITCODE
}
Write-Information "Tests passed" -InformationAction Continue

# Step 6: Create NuGet package
Write-Information "`n[6/7] Creating NuGet package..." -InformationAction Continue
New-Item -ItemType Directory -Force -Path "nuget-output" | Out-Null

dotnet pack CziCheckSharp/CziCheckSharp.csproj `
    --configuration Release `
    --output ./nuget-output `
    -p:Winx64LibLocation="$PWD/native-artifacts/*" `
    -p:Linuxx64LibLocation=""

if ($LASTEXITCODE -ne 0) {
    Write-Information "ERROR: NuGet pack failed" -InformationAction Continue
    exit $LASTEXITCODE
}

Write-Information "`nGenerated NuGet packages:" -InformationAction Continue
Get-ChildItem "nuget-output/" | Format-Table Name, Length, LastWriteTime

# Verify package contents
Write-Information "`n=== Package Contents Verification ===" -InformationAction Continue
$nupkg = Get-ChildItem "nuget-output/*.nupkg" | Select-Object -First 1
if ($nupkg) {
    Write-Information "Inspecting: $($nupkg.Name)" -InformationAction Continue

    # Extract and check for native libraries
    $tempDir = "temp-nupkg-extract"
    if (Test-Path $tempDir) {
        Remove-Item -Recurse -Force $tempDir
    }

    Expand-Archive -Path $nupkg.FullName -DestinationPath $tempDir

    Write-Information "`nChecking for native libraries in package..." -InformationAction Continue
    $winLibs = Get-ChildItem "$tempDir/runtimes/win-x64/native/*" -ErrorAction SilentlyContinue
    if ($winLibs) {
        Write-Information "✓ Found Windows native libraries:" -InformationAction Continue
        $winLibs | ForEach-Object { Write-Information "  - $($_.Name)" -InformationAction Continue }
    } else {
        Write-Information "✗ No Windows native libraries found in package" -InformationAction Continue
    }

    Remove-Item -Recurse -Force $tempDir
}

Write-Information "`n=== Workflow Simulation Complete! ===" -InformationAction Continue
Write-Information "All steps completed successfully" -InformationAction Continue

# Step 7: Test with NuGet package
Write-Information "`n[7/7] Testing with NuGet package..." -InformationAction Continue

# Clean test project build output to ensure we only use the NuGet package
Write-Information "Cleaning test project build output..." -InformationAction Continue
if (Test-Path "CziCheckSharp.Tests/bin") {
    Remove-Item -Recurse -Force "CziCheckSharp.Tests/bin"
    Write-Information "  Removed CziCheckSharp.Tests/bin" -InformationAction Continue
}
if (Test-Path "CziCheckSharp.Tests/obj") {
    Remove-Item -Recurse -Force "CziCheckSharp.Tests/obj"
    Write-Information "  Removed CziCheckSharp.Tests/obj" -InformationAction Continue
}

# Verify NuGet package exists
$nupkg = Get-ChildItem "nuget-output/*.nupkg" -Exclude "*.snupkg" | Select-Object -First 1
if (-not $nupkg) {
    Write-Information "ERROR: NuGet package not found in nuget-output/" -InformationAction Continue
    exit 1
}

Write-Information "Using NuGet package: $($nupkg.Name)" -InformationAction Continue

# Extract version from package filename
$version = [regex]::Match($nupkg.Name, 'm-ringler\.CziCheckSharp\.(.*?)\.nupkg$').Groups[1].Value
Write-Information "Extracted version: $version" -InformationAction Continue

# Run tests with NuGet package
Write-Information "\nRunning tests with NuGet package..." -InformationAction Continue

Write-Information "Restoring NuGet packages..." -InformationAction Continue
dotnet restore CziCheckSharp.Tests/CziCheckSharp.Tests.csproj `
    --source "https://api.nuget.org/v3/index.json" `
    --source "$PWD\nuget-output" `
    -p:Configuration=Release `
    -p:CziCheckSharpVersion=$version `
    -p:UseNuGetPackage=true

if ($LASTEXITCODE -ne 0) {
    Write-Information "ERROR: NuGet restore failed" -InformationAction Continue
    exit $LASTEXITCODE
}

Write-Information "Running tests..." -InformationAction Continue
dotnet test CziCheckSharp.Tests/CziCheckSharp.Tests.csproj `
    --configuration Release `
    --no-restore `
    -p:UseNuGetPackage=true

if ($LASTEXITCODE -ne 0) {
    Write-Information "ERROR: NuGet package tests failed" -InformationAction Continue
    exit $LASTEXITCODE
}
Write-Information "NuGet package tests passed" -InformationAction Continue

Write-Information "`n=== All Tests Complete (including NuGet package validation)! ===" -InformationAction Continue
