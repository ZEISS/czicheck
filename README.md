# CZICheck

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