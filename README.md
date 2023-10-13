# CZICheck
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![REUSE status](https://api.reuse.software/badge/github.com/ZEISS/czicheck)](https://api.reuse.software/info/github.com/ZEISS/czicheck)
[![CMake](https://github.com/ZEISS/czicheck/actions/workflows/cmake.yml/badge.svg?branch=main&event=push)](https://github.com/ZEISS/czicheck/actions/workflows/cmake.yml)
[![CodeQL](https://github.com/ZEISS/czicheck/actions/workflows/codeql.yml/badge.svg?branch=main&event=push)](https://github.com/ZEISS/czicheck/actions/workflows/codeql.yml)
[![MegaLinter](https://github.com/ZEISS/czicheck/actions/workflows/mega-linter.yml/badge.svg?branch=main&event=push)](https://github.com/ZEISS/czicheck/actions/workflows/mega-linter.yml)

CZICheck is a command-line application developed using libCZI, enabling users to assess the integrity and structural correctness of a CZI document.

Checking the validity of a CZI becomes more complex the closer one is to the application domain (e.g. application-specific metadata).
So this console application is more of a utility to help users who are directly using libCZI, or its python wrapper [pylibCZIrw](https://pypi.org/project/pylibCZIrw/), than it is an official validation tool for any ZEISS-produced CZIs.

CZICheck runs a collection of *checkers* which evaluate a well defined rule.
Each *checker* reports back findings of type Fatal, Warn, or Info.

Please check the tool's internal help by running `CZICheck.exe --help` and check additional documentation [here](documentation/czicheck.md).



## Guidelines
[Code of Conduct](./CODE_OF_CONDUCT.md)  
[Contributing](./CONTRIBUTING.md)

## Disclaimer
ZEISS, ZEISS.com are registered trademarks of Carl Zeiss AG.
