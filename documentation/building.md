# Building CZICheck

## overview

The CZICheck-project is using the [CMake](https://cmake.org/) build system. It is a cross-platform build system that can generate native build files for many platforms and IDEs. 
The following external packages are required for building.

| component | description | referenced via | comment
|--|--|--|--|
| [cli11](https://github.com/CLIUtils/CLI11) | command line parser | CMake's [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) | |
| [libCZI](https://github.com/ZEISS/libczi.git) | CZI file format library | CMake's [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) | |
| [XercesC](https://xerces.apache.org/xerces-c/) |validating XML parser | CMake's [find_package](https://cmake.org/cmake/help/latest/command/find_package.html) | If the XercesC-package cannot be found, the checker "xmlmetadataschema" will be disabled and not be available.|


## building
Building CZICheck is done by running those commands. Assume that the current working directory is the root of the CZICheck-project.

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

When running the CMake-configure step, a line stating whether the XercesC-package was found or not will be printed.  

If it is **not** found, you will find a line

```
XercesC library not found, the checker 'xmlmetadataschema' will **not** be available
```

whereas if it was found, it will read

```
XercesC library available -> version: 3.2.4
XercesC library found, the checker 'xmlmetadataschema' will be available
```

The resulting binary is then located in the build/CZICheck directory.

A recording of the build process is available [here](https://asciinema.org/a/593620).


## installing the XercesC-package

For installing the XercesC-package it is recommended to use the package-manager of your operating system. 

For example, on Ubuntu XercesC can be installed by running

```bash
sudo apt-get install libxerces-c-dev
```

On Windows, the [vcpkg-package-manager](https://vcpkg.io/en/) can be used. Running this command will install the XercesC-package (adjust the triplet to your configuration).

```bash
vcpkg install xerces-c --triplet x64-windows-static
```

## running the tests

For configuring the tests, the CMake-option CZICHECK_BUILD_TESTS has to be set to ON. This can be done by adding the option to the CMake-configure step.

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DCZICHECK_BUILD_TESTS=ON
```

During the CMake-run, the test-data will be downloaded (to the build-directory) and the tests will be configured.

The tests can then be run by executing the following command in the build-directory.

```bash
ctest -C Release
```
