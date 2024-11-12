# SPDX-FileCopyrightText: 2023 Carl Zeiss Microscopy GmbH
#
# SPDX-License-Identifier: MIT

""" Test-Runner for CZICheck

This script is intended to run test of the CZICheck tool. What it does is:
 * It reads a CSV-formatted text file containing the test case.
 * A "test case" is given by a CZI-file, the expected exit code (of running CZICheck) and
    the expected output of CZICheck (to stdout).
 * This script will now run all the test-cases defined in the CSV-formatted test-list file.
 * If the output of CZICheck is different from the expected output, or if the exit code is
    different, then this script will exit with exit code 1.
 * If there are no problems with the tests, the exit code will be 0.
 * An exit code of 99 indicates that the command line arguments could not be parsed.
  
"""
import argparse
import csv
import os
import subprocess
import sys


class Parameters:
    """
    This class gathers all arguments specified on the command line.
    """

    _czicheck_executable: str = r'CZICheck.exe'
    _folder_with_czi_files: str = ''
    _folder_with_known_good_results: str = ''
    _test_cases_list_filename: str
    _czi_file_redirects: dict
    _czicheck_output_save_path: str

    def parse_commandline(self):
        """
        Parse the arguments specified on the command line. If the arguments are determined to be invalid, then
        the program is terminated (with exit code "99").
        """
        parser = argparse.ArgumentParser(
            description='test-runner for CZICheck - run with sample data, compare result to known-good-results')
        parser.add_argument('-e', '--executable', dest='czicheck_executable',
                            help='Filename of the CZICheck executable.')
        parser.add_argument('-s', '--czisourcepath', dest='czi_source_path',
                            help='Path where the CZI-files from the test-list are to be found.')
        parser.add_argument('-k', '--knowngoodresultspath', dest='known_good_results_path',
                            help='Path where the known-good results from the test-list are to be found. If not specified, the same folder as for the CZI-files is assumed.')
        parser.add_argument('-t', '--test_list', dest='test_list_file',
                            help='Filename of a CSV-file containing the test-cases.')
        parser.add_argument('-r', '--redirect', dest='czi_file_redirects', action='append',
                            help='Add a redirection, i.e. a string in the form <CZIName>=<redirected_filename>.')
        parser.add_argument('-o', '--outputsavepath', dest='czicheck_output_save_path',
                            help='Path where the output of CZICheck are saved (useful for re-creating the known-good results)')
        args = parser.parse_args()
        if args.czicheck_executable:
            self._czicheck_executable = args.czicheck_executable
        if args.czi_source_path:
            self._folder_with_czi_files = args.czi_source_path
        if args.known_good_results_path:
            self._folder_with_known_good_results = args.known_good_results_path
        else:
            self._folder_with_known_good_results = self._folder_with_czi_files
        if args.test_list_file:
            self._test_cases_list_filename = args.test_list_file
        if args.czicheck_output_save_path:
            self._czicheck_output_save_path = args.czicheck_output_save_path

        if args.czi_file_redirects:
            self._czi_file_redirects = {}
            for redirect in args.czi_file_redirects:
                position_of_equal = redirect.index('=')
                czi_filename = redirect[:position_of_equal]
                redirected_filename = redirect[1 + position_of_equal:]
                self._czi_file_redirects[czi_filename] = redirected_filename

        if not args.test_list_file or args.test_list_file.isspace():
            print('No argument given for test_list_file -> exiting')
            sys.exit(99)

    def build_fully_qualified_czi_filename(self, name):
        if self._czi_file_redirects:
            redirected_filename = self._czi_file_redirects.get(name)
            if redirected_filename:
                if os.path.isabs(redirected_filename):
                    return redirected_filename
                else:
                    os.path.join(self._folder_with_czi_files, redirected_filename)

        return os.path.join(self._folder_with_czi_files, name)

    def build_fully_qualified_expected_result_filename(self, name):
        return os.path.join(self._folder_with_known_good_results, name)

    def get_fully_qualified_czicheck_executable(self):
        return self._czicheck_executable

    def get_fully_qualified_testlist_filename(self):
        return self._test_cases_list_filename

    def get_fully_qualified_path_for_czicheck_output(self, expected_result_filename):
        """

        Args:
            expected_result_filename:

        Returns:
            Given a filename, this method returns a fully qualified filename for a CZICheck-output. The method
            may return 'None' in the case where we are instructed to **not** store the CZICheck-output
            (i.e. the option 'outputsavepath' was not given).
        """
        try:
            return os.path.join(self._czicheck_output_save_path, expected_result_filename)
        except AttributeError:
            pass  # return 'None' in case we are not given a path for saving the output


def compare_result_of_test_to_knowngood(result: str, knowngood: str):
    testrun_lines = result.splitlines()
    knowngood_lines = knowngood.splitlines()
    for lineno in range(0, max([len(testrun_lines), len(knowngood_lines)])):
        line_test = testrun_lines[lineno] if (lineno < len(testrun_lines)) else None
        line_knowngood = knowngood_lines[lineno] if (lineno < len(knowngood_lines)) else None
        if not line_test == line_knowngood:
            print(f"DIFFERENT({lineno + 1}) - is '{line_test}' expected: '{line_knowngood}'")
            return False

    return True


def check_file(cmdline_parameters: Parameters, czi_filename: str, expected_result_file: str, expected_returncode: int):
    cmdlineargs = [cmdline_parameters.get_fully_qualified_czicheck_executable(), '-s',
                   cmdline_parameters.build_fully_qualified_czi_filename(czi_filename),
                   '-c','all', '--laxparsing', 'true']
    print(f"test {czi_filename}")

    testouput_encoding_list = ["text", "json", "xml"]
    testoutput_results = [False, False, False]

    for i, encoding in enumerate(testouput_encoding_list):
        current_cmd_args = cmdlineargs[:]
        current_cmd_args.append("-e")
        current_cmd_args.append(encoding)

        output = subprocess.run(current_cmd_args, capture_output=True, check=False, universal_newlines=True)
        current_expected_result_file = expected_result_file
        if encoding != "text":
            current_expected_result_file = f"{expected_result_file}.{encoding}"

        # here we compare with the unmodified expected_result_file variable
        if expected_result_file != '*':
            fully_qualified_filename_to_save_output = cmdline_parameters.get_fully_qualified_path_for_czicheck_output(
                current_expected_result_file)
            if fully_qualified_filename_to_save_output:
                with open(fully_qualified_filename_to_save_output, 'w') as czicheck_output_save_file:
                    czicheck_output_save_file.write(output.stdout)

        testoutput_results[i] = output.returncode == expected_returncode
        if not testoutput_results[i]:
            print(
                f"exit code of 'CZICheck' was expected to be {expected_returncode}, but was found to be {output.returncode} for {encoding} output.")
            return False

        # # if the filename for the "known good result" is the special value "*", then we skip the comparison
        if expected_result_file != '*':
            with open(cmdline_parameters.build_fully_qualified_expected_result_filename(current_expected_result_file), "r") as text_file:
                expected_result = text_file.read()
                testoutput_results[i] = compare_result_of_test_to_knowngood(output.stdout, expected_result)

    return all(testoutput_results)

parameters = Parameters()
parameters.parse_commandline()
numberOfTests = 0
numberOfFailedTests = 0
with open(parameters.get_fully_qualified_testlist_filename()) as csv_file:
    csv_reader = csv.DictReader(csv_file, delimiter=',')
    for row in csv_reader:
        if row['czifilename'] and not row['czifilename'].isspace() and not row['czifilename'].startswith('#'):
            numberOfTests = numberOfTests + 1
            testOk = check_file(parameters, row['czifilename'], row['known_good_output'],
                                int(row['expected_return_code']))
            if not testOk:
                numberOfFailedTests = numberOfFailedTests + 1

if numberOfFailedTests > 0:
    sys.exit(1)
