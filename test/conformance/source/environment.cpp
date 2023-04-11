// Copyright (C) 2022-2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include <cstring>
#include <fstream>

#if __has_include(<filesystem>)
#include <filesystem>
namespace filesystem = std::filesystem;
#else
#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;
#endif

#include <uur/environment.h>
#include <uur/utils.h>

namespace uur {

constexpr char ERROR_NO_ADAPTER[] = "Could not load adapter";

PlatformEnvironment *PlatformEnvironment::instance = nullptr;

std::ostream &operator<<(std::ostream &out,
                         const ur_platform_handle_t &platform) {
    size_t size;
    urPlatformGetInfo(platform, UR_PLATFORM_INFO_NAME, 0, nullptr, &size);
    std::vector<char> name(size);
    urPlatformGetInfo(platform, UR_PLATFORM_INFO_NAME, size, name.data(),
                      nullptr);
    out << name.data();
    return out;
}

std::ostream &operator<<(std::ostream &out,
                         const std::vector<ur_platform_handle_t> &platforms) {
    for (auto platform : platforms) {
        out << "\n  * \"" << platform << "\"";
    }
    return out;
}

uur::PlatformEnvironment::PlatformEnvironment(int argc, char **argv)
    : platform_options{parsePlatformOptions(argc, argv)} {
    instance = this;
    ur_device_init_flags_t device_flags = 0;
    switch (urInit(device_flags)) {
    case UR_RESULT_SUCCESS:
        break;
    case UR_RESULT_ERROR_UNINITIALIZED:
        error = ERROR_NO_ADAPTER;
        return;
    default:
        error = "urInit() failed";
        return;
    }

    uint32_t count = 0;
    if (urPlatformGet(0, nullptr, &count)) {
        error = "urPlatformGet() failed to get number of platforms.";
        return;
    }

    if (count == 0) {
        error = "Failed to find any platforms.";
        return;
    }

    std::vector<ur_platform_handle_t> platforms(count);
    if (urPlatformGet(count, platforms.data(), nullptr)) {
        error = "urPlatformGet failed to get platforms.";
        return;
    }

    if (platform_options.platform_name.empty()) {
        if (platforms.size() == 1) {
            platform = platforms[0];
        } else {
            std::stringstream ss_error;
            ss_error << "Select a single platform from below using the "
                        "--platform=NAME "
                        "command-line option:"
                     << platforms;
            error = ss_error.str();
            return;
        }
    } else {
        for (auto candidate : platforms) {
            size_t size;
            if (urPlatformGetInfo(candidate, UR_PLATFORM_INFO_NAME, 0, nullptr,
                                  &size)) {
                error = "urPlatformGetInfoFailed";
                return;
            }
            std::vector<char> platform_name(size);
            if (urPlatformGetInfo(candidate, UR_PLATFORM_INFO_NAME, size,
                                  platform_name.data(), nullptr)) {
                error = "urPlatformGetInfo() failed";
                return;
            }
            if (platform_options.platform_name == platform_name.data()) {
                platform = candidate;
                break;
            }
        }
        if (!platform) {
            std::stringstream ss_error;
            ss_error << "Platform \"" << platform_options.platform_name
                     << "\" not found. Select a single platform from below "
                        "using the "
                        "--platform=NAME command-line options:"
                     << platforms;
            error = ss_error.str();
            return;
        }
    }
}

void uur::PlatformEnvironment::SetUp() {
    if (!error.empty()) {
        if (error == ERROR_NO_ADAPTER) {
            GTEST_SKIP() << error;
        } else {
            FAIL() << error;
        }
    }
}

void uur::PlatformEnvironment::TearDown() {
    if (error == ERROR_NO_ADAPTER) {
        return;
    }
    ur_tear_down_params_t tear_down_params{};
    if (urTearDown(&tear_down_params)) {
        FAIL() << "urTearDown() failed";
    }
}

PlatformEnvironment::PlatformOptions
PlatformEnvironment::parsePlatformOptions(int argc, char **argv) {
    PlatformOptions options;
    for (int argi = 1; argi < argc; ++argi) {
        const char *arg = argv[argi];
        if (!(std::strcmp(arg, "-h") && std::strcmp(arg, "--help"))) {
            // TODO - print help
            break;
        } else if (std::strncmp(
                       arg, "--platform=", sizeof("--platform=") - 1) == 0) {
            options.platform_name =
                std::string(&arg[std::strlen("--platform=")]);
        }
    }
    return options;
}

DevicesEnvironment *DevicesEnvironment::instance = nullptr;

DevicesEnvironment::DevicesEnvironment(int argc, char **argv)
    : PlatformEnvironment(argc, argv) {
    instance = this;
    if (!error.empty()) {
        return;
    }
    uint32_t count = 0;
    if (urDeviceGet(platform, UR_DEVICE_TYPE_ALL, 0, nullptr, &count)) {
        error = "urDevicesGet() failed to get number of devices.";
        return;
    }
    if (count == 0) {
        error = "Could not find any devices associated with the platform";
        return;
    }
    devices.resize(count);
    if (urDeviceGet(platform, UR_DEVICE_TYPE_ALL, count, devices.data(),
                    nullptr)) {
        error = "urDeviceGet() failed to get devices.";
        return;
    }
}

void DevicesEnvironment::SetUp() {
    PlatformEnvironment::SetUp();
    if (error == ERROR_NO_ADAPTER) {
        return;
    }
    if (devices.empty() || !error.empty()) {
        FAIL() << error;
    }
}

void DevicesEnvironment::TearDown() {
    PlatformEnvironment::TearDown();
    for (auto device : devices) {
        if (urDeviceRelease(device)) {
            error = "urDeviceRelease() failed";
            return;
        }
    }
}

KernelsEnvironment *KernelsEnvironment::instance = nullptr;

KernelsEnvironment::KernelsEnvironment(int argc, char **argv,
                                       std::string kernels_default_dir)
    : DevicesEnvironment(argc, argv),
      kernel_options(parseKernelOptions(argc, argv, kernels_default_dir)) {
    instance = this;
    if (!error.empty()) {
        return;
    }
}

KernelsEnvironment::KernelOptions
KernelsEnvironment::parseKernelOptions(int argc, char **argv,
                                       std::string kernels_default_dir) {
    KernelOptions options;
    for (int argi = 1; argi < argc; ++argi) {
        const char *arg = argv[argi];
        if (std::strncmp(arg, "--kernel_directory=",
                         sizeof("--kernel_directory=") - 1) == 0) {
            options.kernel_directory =
                std::string(&arg[std::strlen("--kernel_directory=")]);
        }
    }
    if (options.kernel_directory.empty()) {
        options.kernel_directory = kernels_default_dir;
    }

    return options;
}

std::string KernelsEnvironment::getSupportedILPostfix(uint32_t device_index) {
    std::stringstream IL;

    if (instance->GetDevices().size() == 0) {
        error = "no devices available on the platform";
        return {};
    }

    auto device = instance->GetDevices()[device_index];
    std::string IL_version;
    if (uur::GetDeviceILVersion(device, IL_version)) {
        error = "failed to get device IL version";
        return {};
    }

    // Delete the ETX character at the end as it is not part of the name.
    IL_version.pop_back();

    // TODO: Add other IL types like ptx when they are defined how they will be
    // reported.
    if (IL_version.find("SPIR-V") != std::string::npos) {
        IL << ".spv";
    } else {
        error = "Undefined IL version: " + IL_version;
        return {};
    }

    return IL.str();
}

std::string
KernelsEnvironment::getKernelSourcePath(const std::string &kernel_name,
                                        uint32_t device_index) {
    std::stringstream path;
    path << kernel_options.kernel_directory << "/" << kernel_name;
    std::string il_postfix = getSupportedILPostfix(device_index);

    if (il_postfix.empty()) {
        return {};
    }

    std::string binary_name;
    for (const auto &entry : filesystem::directory_iterator(path.str())) {
        auto file_name = entry.path().filename().string();
        if (file_name.find(il_postfix) != std::string::npos) {
            binary_name = file_name;
            break;
        }
    }

    if (binary_name.empty()) {
        error =
            "failed retrieving kernel source path for kernel: " + kernel_name;
        return {};
    }

    path << "/" << binary_name;

    return path.str();
}

void KernelsEnvironment::LoadSource(
    const std::string &kernel_name, uint32_t device_index,
    std::shared_ptr<std::vector<char>> &binary_out) {
    std::string source_path =
        instance->getKernelSourcePath(kernel_name, device_index);

    if (source_path.empty()) {
        FAIL() << error;
    }

    if (cached_kernels.find(source_path) != cached_kernels.end()) {
        binary_out = cached_kernels[source_path];
        return;
    }

    std::ifstream source_file;
    source_file.open(source_path,
                     std::ios::binary | std::ios::in | std::ios::ate);

    if (!source_file.is_open()) {
        FAIL() << "failed opening kernel path: " + source_path;
    }

    size_t source_size = static_cast<size_t>(source_file.tellg());
    source_file.seekg(0, std::ios::beg);

    std::vector<char> device_binary(source_size);
    source_file.read(device_binary.data(), source_size);
    if (!source_file) {
        source_file.close();
        FAIL() << "failed reading kernel source data from file: " + source_path;
    }
    source_file.close();

    auto binary_ptr =
        std::make_shared<std::vector<char>>(std::move(device_binary));
    cached_kernels[kernel_name] = binary_ptr;
    binary_out = binary_ptr;
}

void KernelsEnvironment::SetUp() {
    DevicesEnvironment::SetUp();
    if (!error.empty()) {
        FAIL() << error;
    }
}

void KernelsEnvironment::TearDown() {
    cached_kernels.clear();
    DevicesEnvironment::TearDown();
}
} // namespace uur
