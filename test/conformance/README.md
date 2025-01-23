# Conformance tests

At this point, conformance tests include matches for individual adapters
that allow you to ignore errors from the listed tests.
This solution allows regular execution of cts tests on GHA
and prevents further errors.
In the future, when all bugs are fixed, and the tests pass,
this solution will no longer be necessary.
When you fix any test, the match file must be updated
Empty match files indicate that there are no failing tests
in a particular group for the corresponding adapter.

## How to set test device/platform name or limit the test devices/platforms count

To limit how many devices/platforms you want to run the conformance and
adapters tests on, use CMake option UR_TEST_DEVICES_COUNT or
UR_TEST_PLATFORMS_COUNT. If you want to run the tests on
all available devices/platforms, set 0. The default value is 1.
If you run binaries for the tests, you can use the parameter
`--platforms_count=COUNT` or `--devices_count=COUNT`.
To set test device/platform name you want to run the tests on, use
parameter `--platform=NAME` or `--device=NAME`.

## Known failures

The `UUR_KNOWN_FAILURE_ON` macro can be used to skip tests on devices where the
test is known to fail. This can be done in the following situations.

For all devices in an adapter:

```cpp
UUR_KNOWN_FAILURE_ON(uur::LevelZero{});
```

By substring match of the device name within and adapter:

```cpp
UUR_KNOWN_FAILURE_ON(uur::OpenCL{"Intel(R) UHD Graphics 770"});
```

In certain test suits, where there is no access to a device, the platform name
is used instead:

```cpp
UUR_KNOWN_FAILURE_ON(uur::CUDA{"NVIDIA CUDA BACKEND"});
```

When neither device or platform is available in a test suite, the name is
ignored and only the adapter backend is used to determine if the test is a
known failure.

The macro is variadic making it possible to specify known failures for multiple
adapters in a single place and multiple names can also be provided per adapter:

```cpp
UUR_KNOWN_FAILURE_ON(
uur::OpenCL{
    "Intel(R) UHD Graphics 750",
    "Intel(R) UHD Graphics 770",
},
uur::HIP{"Radeon RX 7700"},
uur::NativeCPU{});
```

The following adapter matcher objects are available:

* `uur::OpenCL`
* `uur::LevelZero`
* `uur::LevelZeroV2`
* `uur::CUDA`
* `uur::HIP`
* `uur::NativeCPU`
