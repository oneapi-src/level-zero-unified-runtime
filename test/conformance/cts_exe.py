#! /usr/bin/env python3
"""
 Copyright (C) 2023 Intel Corporation

 Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
 See LICENSE.TXT
 SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
# Printing conformance test output from gtest and checking failed tests with match files.
# The match files contain tests that are expected to fail.

import os
import shlex
import sys
from argparse import ArgumentParser
import subprocess  # nosec B404
import signal
import re
from collections import OrderedDict


def _print_cmdline(cmd_args, env, cwd, file=sys.stderr):
    cwd = shlex.quote(cwd)
    env_args = " ".join(
        "%s=%s" % (shlex.quote(k), shlex.quote(v)) for k, v in env.items()
    )
    cmd_str = " ".join(map(shlex.quote, cmd_args))
    print(f"### env -C {cwd} -i {env_args} {cmd_str}", file=file)


if __name__ == "__main__":

    parser = ArgumentParser()
    parser.add_argument("--test_command", help="Ctest test case")
    parser.add_argument("--devices_count", type=str, help="Number of devices on which tests will be run")
    parser.add_argument("--platforms_count", type=str, help="Number of platforms on which tests will be run")
    parser.add_argument("--backend", type=str, help="Number of platforms on which tests will be run")
    args = parser.parse_args()
    invocation = [
        args.test_command,
        "--gtest_brief=1",
        f"--devices_count={args.devices_count}",
        f"--platforms_count={args.platforms_count}",
        f"--backend={args.backend}",
    ]
    _print_cmdline(invocation, os.environ, os.getcwd())

    result = subprocess.Popen(  # nosec B603
        invocation, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True
    )

    pat = re.compile(r'\[( )*FAILED( )*\]')
    output_list = []
    test_cases = []
    for line in result.stdout:
        output_list.append(line)
        if pat.search(line):
            test_case = line.split(" ")[5]
            test_case = test_case.rstrip(',')
            test_cases.append(test_case)

    # Every fail has a single corresponding match line but if there are multiple
    # devices being tested there will be multiple lines with the same failure
    # message. To avoid matching mismatch, remove lines that differ only by device ID.
    test_cases = [re.sub(r'ID[0-9]ID', 'X', tc) for tc in test_cases]
    test_cases = list(OrderedDict.fromkeys(test_cases))

    for tc in test_cases:
        print(tc)

    rc = result.wait()
    if rc < 0:
        print(signal.strsignal(abs(rc)))

    print("#### GTEST_OUTPUT ####", file=sys.stderr)
    print(''.join(output_list), file=sys.stderr)
    print("#### GTEST_OUTPUT_END ####", file=sys.stderr)
