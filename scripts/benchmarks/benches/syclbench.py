# Copyright (C) 2024 Intel Corporation
# Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
# See LICENSE.TXT
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import os
import csv
import io
from utils.utils import run, git_clone, create_build_path
from .base import Benchmark
from .result import Result
from .options import options

class SyclBench:
    def __init__(self, directory):
        self.directory = directory
        self.built = False
        self.setup()
        return

    def setup(self):
        build_path = os.path.join(self.directory, 'sycl-bench-build')
        self.bins = build_path

        if self.built or not options.rebuild:
            return

        create_build_path(build_path, '')

        repo_path = git_clone(self.directory, "sycl-bench-repo", "https://github.com/mateuszpn/sycl-bench.git", "1e6ab2cfd004a72c5336c26945965017e06eab71")
        print(f"Clone repo to: {repo_path}")

        print(f"Build path: {build_path}")

        configure_command = [
            "cmake",
            f"-B {build_path}",
            f"-S {repo_path}",
            f"-DCMAKE_BUILD_TYPE=Release",
            f"-DCMAKE_CXX_COMPILER={options.sycl}/bin/clang++",
            f"-DCMAKE_C_COMPILER={options.sycl}/bin/clang",
            f"-DSYCL_IMPL=dpcpp"
        ]

        print(f"Run {configure_command}")
        run(configure_command, add_sycl=True)
        print(f"Run cmake --build {build_path}")
        run(f"cmake --build {build_path} -j", add_sycl=True)

        self.built = True
        return
    
class SyclBenchmark(Benchmark):
    def __init__(self, bench, name, test):
        self.bench = bench
        self.bench_name = name
        self.test = test
        super().__init__(bench.directory)

    def bin_args(self) -> list[str]:
        return []

    def extra_env_vars(self) -> dict:
        return {}

    def unit(self):
        return "ms"

    def setup(self):
        self.bench.setup()
        self.benchmark_bin = os.path.join(self.bench.bins, self.bench_name)

    def run(self, env_vars) -> Result:
        outputfile = f"{self.bench.directory}/{self.test}.csv"
        command = [
            f"{self.benchmark_bin}",
            f"--warmup-run",
            f"--num-runs=3",
            f"--output={outputfile}"
        ]
        bin_dir = self.bench.bins

        command += self.bin_args()
        env_vars.update(self.extra_env_vars())

        print(f"Command: {command}")
        result = self.run_bench(command, env_vars)

        with open(outputfile, 'r') as f:
            reader = csv.reader(f)
            res_list = []
            for row in reader:
                if not row[0].startswith('#'):
                    res_list.append(
                        Result(label=row[0], 
                               value=float(row[12]) * 1000, # convert to ms 
                               command=command, 
                               env=env_vars, 
                               stdout=result))
            
            median_list = []
            for label in set(result.label for result in res_list):
                values = [result.value for result in res_list if result.label == label]
                median_value = sorted(values)[len(values) // 2]
                median_list.append(Result(label=label, value=median_value, command=command, env=env_vars, stdout=result))

        return median_list

    def teardown(self):
        return

    def name(self):
        return self.test

class Arith(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "arith", "Arith_int32_512")

    def bin_args(self) -> list[str]:
        return [
            f"--size=16384",
        ]

class TwoDConvolution(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "2DConvolution", "2DConvolution")

class Two_mm(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "2mm", "2mm")

    def bin_args(self) -> list[str]:
        return [
            f"--size=512",
        ]

class Three_mm(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "3mm", "3mm")

    def bin_args(self) -> list[str]:
        return [
            f"--size=512",
        ]

class Atax(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "atax", "Atax")

    def bin_args(self) -> list[str]:
        return [
            f"--size=8192",
        ]

class Atomic_reduction(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "atomic_reduction", "ReductionAtomic_fp64")

class Bicg(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "bicg", "Bicg")

    def bin_args(self) -> list[str]:
        return [
            f"--size=8192",
        ]
    
class Correlation(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "correlation", "Correlation")

    def bin_args(self) -> list[str]:
        return [
            f"--size=512",
        ]

class Covariance(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "covariance", "Covariance")

    def bin_args(self) -> list[str]:
        return [
            f"--size=512",
        ]

class Gemm(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "gemm", "Gemm")

    def bin_args(self) -> list[str]:
        return [
            f"--size=1024",
        ]
    
class Gesumv(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "gesummv", "Gesummv")

    def bin_args(self) -> list[str]:
        return [
            f"--size=8192",
        ]

class Gramschmidt(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "gramschmidt", "Gramschmidt")

    def bin_args(self) -> list[str]:
        return [
            f"--size=512",
        ]

class KMeans(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "kmeans", "Kmeans")

    def bin_args(self) -> list[str]:
        return [
            f"--size=67108864",
        ]

class LinRegCoeff(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "lin_reg_coeff", "LinearRegressionCoeff")

class LinRegError(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "lin_reg_error", "LinearRegression")

class MatmulChain(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "matmulchain", "MatmulChain")

    def bin_args(self) -> list[str]:
        return [
            f"--size=1024",
        ]

# ** bad input file path **
# 
# class Median(SyclBenchmark):
#     def __init__(self, bench):
#         super().__init__(bench, "median", "MedianFilter")
# 
#     def bin_args(self) -> list[str]:
#         return [
#             f"--size=512",
#         ]

class MolDyn(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "mol_dyn", "MolecularDynamics")


class Mvt(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "mvt", "Mvt")

    def bin_args(self) -> list[str]:
        return [
            f"--size=16384",
        ]
    
# ** verification fail **
# 
# class NBody(SyclBenchmark):
#     def __init__(self, bench):
#         super().__init__(bench, "nbody", "NBody_")

class Sf(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "sf", "sf_16")

    def bin_args(self) -> list[str]:
        return [
            f"--size=--size=100000000",
        ]
    
# bad input file path
# 
# class SobelX(SyclBenchmark):
#     def __init__(self, bench):
#         super().__init__(bench, "sobel", "SobelFilter")

class Syr2k(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "syr2k", "Syr2k")

    def bin_args(self) -> list[str]:
        return [
            f"--size=1024",
        ]
    
class Syrk(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "syrk", "Syrk")

    def bin_args(self) -> list[str]:
        return [
            f"--size=1024",
        ]

# multi benchmarks
class Blocked_transform(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "blocked_transform", "BlockedTransform_multi")

    def bin_args(self) -> list[str]:
        return [
            f"--size=512",
        ]

class DagTaskI(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "dag_task_throughput_independent", "IndependentDAGTaskThroughput_multi")

    def bin_args(self) -> list[str]:
        return [
            f"--size=512",
        ]
    
class DagTaskS(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "dag_task_throughput_sequential", "DAGTaskThroughput_multi")

    def bin_args(self) -> list[str]:
        return [
            f"--size=512",
        ]

class HostDevBandwidth(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "host_device_bandwidth", "HostDeviceBandwidth_multi")

class LocalMem(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "local_mem", f"LocalMem_multi")

    def bin_args(self) -> list[str]:
        return [
            f"--size=512",
        ]

class Pattern_L2(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "pattern_L2", "L2_multi")

class Reduction(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "reduction", "Pattern_Reduction_multi")

class ScalarProd(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "scalar_prod", "ScalarProduct_multi")

class SegmentReduction(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "segmentedreduction", "Pattern_SegmentedReduction_multi")

class UsmAccLatency(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "usm_accessors_latency", "USM_Latency_multi")

class UsmAllocLatency(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "usm_allocation_latency", "USM_Allocation_latency_multi")

class UsmInstrMix(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "usm_instr_mix", "USM_Instr_Mix_multi")

class UsmPinnedOverhead(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "usm_pinned_overhead", "USM_Pinned_Overhead_multi")

class VecAdd(SyclBenchmark):
    def __init__(self, bench):
        super().__init__(bench, "vec_add", "VectorAddition_multi")

