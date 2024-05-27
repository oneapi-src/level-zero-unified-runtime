#!/usr/bin/env python
"""
 Copyright (C) 2022 Intel Corporation

 Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
 See LICENSE.TXT
 SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""

from subprocess import  Popen, DEVNULL, PIPE
import argparse
import os
import json

TMP_RESULTS_FILE = "tmp-results-file.json"

def get_cts_test_suite_names(working_directory):
    process = Popen(["ctest", "--show-only=json-v1"], cwd=working_directory,
                    stdout=PIPE, env=os.environ.copy())
    out,_ = process.communicate()
    testsuites = json.loads(out)
    return [
        test['name'][:test['name'].rfind('-')] for test in testsuites['tests']
    ]

def percent(amount, total):
    return round((amount / (total or 1)) * 100, 2)

def summarize_results(results):
    total = results['Total']
    total_passed = results['Passed']
    total_skipped = results['Skipped']
    total_failed = results['Failed']

    pass_rate_incl_skipped = percent(total_passed + total_skipped, total)
    pass_rate_excl_skipped = percent(total_passed, total)

    skipped_rate = percent(total_skipped, total)
    failed_rate = percent(total_failed, total)

    ljust_param = len(str(total))

    print(
f"""[CTest Parser] Results:
    Total    [{str(total).ljust(ljust_param)}]
    Passed   [{str(total_passed).ljust(ljust_param)}]    ({pass_rate_incl_skipped}%) - ({pass_rate_excl_skipped}% with skipped tests excluded)
    Skipped  [{str(total_skipped).ljust(ljust_param)}]    ({skipped_rate}%)
    Failed   [{str(total_failed).ljust(ljust_param)}]    ({failed_rate}%)
"""
    )

def parse_results(results: dict):
    parsed_results = {"Passed": 0, "Skipped": 0, "Failed": 0, "Total": 0}
    for result in results.values():
        parsed_results['Failed'] += result['failures']
        parsed_results['Total'] += result['tests']
        for testsuite in result['testsuites']:
            for test in testsuite['testsuite']:
                if test['result'] == 'SKIPPED':
                    parsed_results['Skipped'] += 1
    parsed_results["Passed"] += (
        parsed_results["Total"] -
        parsed_results["Failed"] -
        parsed_results["Skipped"]
    )
    return parsed_results

def run(args):
    results = {}

    tmp_results_file = f"{args.ctest_path}/{TMP_RESULTS_FILE}"
    env = os.environ.copy()
    env['GTEST_OUTPUT'] = f"json:{tmp_results_file}"

    test_suite_names = get_cts_test_suite_names(f"{args.ctest_path}/test/conformance/")

    for suite in test_suite_names:
        ctest_path = f"{args.ctest_path}/test/conformance/{suite}"
        process = Popen(['ctest',ctest_path], env=env, cwd=ctest_path,
                        stdout=DEVNULL if args.quiet else None,
                        stderr=DEVNULL if args.quiet else None)
        process.wait()

        try:
            with open(tmp_results_file, 'r') as results_file:
                json_data = json.load(results_file)
                results[suite] = json_data
            os.remove(tmp_results_file)
        except FileNotFoundError:
            print(f"\033[91mConformance test suite '{suite}' : likely crashed! Unable to calculate pass rate.\033[0m")
            exit(1)

    return results

def dir_path(string):
    if os.path.isdir(string):
        return os.path.abspath(string)
    else:
        raise NotADirectoryError(string)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('ctest_path', type=dir_path, nargs='?', default='.',
                        help='Optional path to test directory containing '
                        'CTestTestfile. Defaults to current directory.')
    parser.add_argument('-q', '--quiet', action='store_true',
                        help='Output only failed tests.')
    args = parser.parse_args()

    raw_results = run(args)
    parsed_results = parse_results(raw_results)
    summarize_results(parsed_results)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        exit(130)
