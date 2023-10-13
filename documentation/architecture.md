# architecture

### general

The basic architecture of the application is:

* An [ICZIReader](https://zeiss.github.io/libczi/classlib_c_z_i_1_1_i_c_z_i_reader.html)-object is created (allowing for access of the CZI-file).
* A result-gathering object is created (to which the check-results are reported).
* We then loop over all enabled checkers and
* ...create instance of the checker
* ...pass to the checker-instance a reference to the ICZIReader-object and the result-gathering object
* ...call the checker's "RunCheck"-method

This is implemented in the file runchecks.cpp.

### checkers

In order to make a checker-class usable by the application, it needs to implement the interface `IChecker` (defined in checker.h).  
And it needs to be registered with the class-factory `CheckerFactory` (defined in checkerfactory.h).
At creation time, a reference to the result-gathering object, the ICZIReader object and an additional argument object (CheckerCreateInfo) is passed to the constructor of the checker.
This addtitional argument object can be used to pass additional information to the checker, currently the only field is the size of the file itself (which cannot retrieved from the ICZIReader object).

In the checker's RunCheck-method, the checker can then access the CZI-file and the result-gathering object.  
It is required to call the result-gathering object's "StartCheck"-method once it starts operation. After this, the checker can 
call the result-gathering object's "ReportFinding"-method to report findings. This may be called multiple times (e.g. if the checker finds multiple issues), but it is not required to call it at all (e.g. if the checker does not find any issues).  
Finally, the checker needs to call the result-gathering object's "FinishCheck"-method once it is done.

A finding is classified by a "severity" (enum `CheckResult::Severity`). There are three levels of severity:

| severity | description 
|--|--|
| Fatal | The checker found an issue, and this issue is expected with some certainty to result in adverse behavior. |
| Warning | The checker found an issue, but it is not evident whether this issue will result in adverse behavior. |
| Info | The checker found something unusual, but it is not expected or not evident to result in adverse behavior. |

### architectural decisions

* The checkers are intended to run completely independent of each other.
* This means that they must not rely on each other's results, and they must not rely on the order in which they are run.
* There are no provisions to pass information from one checker to another for this reason.
* A checker should have a very narrow scope, i.e. it should check for a very specific issue.
* It might be tempting to reuse data generated in one checker in another checker, but this is to be avoided. 
* If two checkers need the same data, they should both generate it themselves. The performance penalty is accepted.
* Performance so far does not have a high priority, but it should be kept in mind that the checkers are run on potentially large files.
* The checkers should be as simple and focussed as possible, and should not contain any logic that is not directly related to the check they are performing.

