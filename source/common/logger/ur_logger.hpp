// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#ifndef UR_LOGGER_HPP
#define UR_LOGGER_HPP 1

#include <algorithm>
#include <memory>

#include "ur_logger_details.hpp"
#include "ur_util.hpp"

namespace logger {

Logger create_logger(std::string logger_name);

inline Logger &get_logger(std::string name = "common") {
    static Logger logger = create_logger(name);
    return logger;
}

inline void init(std::string name) {
    get_logger(name);
}

template <typename... Args>
inline void debug(const char *format, Args &&...args) {
    get_logger().log(logger::Level::DEBUG, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void info(const char *format, Args &&...args) {
    get_logger().log(logger::Level::INFO, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void warning(const char *format, Args &&...args) {
    get_logger().log(logger::Level::WARN, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void error(const char *format, Args &&...args) {
    get_logger().log(logger::Level::ERR, format, std::forward<Args>(args)...);
}

inline void setLevel(logger::Level level) { get_logger().setLevel(level); }

inline void setFlushLevel(logger::Level level) { get_logger().setFlushLevel(level); }

/// @brief Create an instance of the logger with parameters obtained from the respective
///        environment variable or with default configuration if the env var is empty,
///        not set, or has the wrong format.
///        Logger env vars are in the format: UR_LOG_*, ie.:
///        - UR_LOG_LOADER (logger for loader library),
///        - UR_LOG_NULL (logger for null adapter).
///        Example of env var for setting up a loader library logger with logging
///        level set to `info`, flush level set to `warning`, and output set to
///        the `out.log` file:
///             UR_LOG_LOADER="level:info;flush:warning;output:file,out.log"
/// @param logger_name name that should be appended to the `UR_LOG_` prefix to
///        get the proper environment variable, ie. "loader"
/// @return an instance of a logger::Logger. In case of failure in the parsing of
///         the environment variable, returns a default logger with the following
///         options:
///             - log level: quiet, meaning no messages are printed
///             - flush level: error, meaning that only error messages are guaranteed
///                            to be printed immediately as they occur
///             - output: stderr
inline Logger create_logger(std::string logger_name) {
    std::transform(logger_name.begin(), logger_name.end(), logger_name.begin(),
                   ::toupper);
    std::stringstream env_var_name;
    const auto default_level = logger::Level::QUIET;
    const auto default_flush_level = logger::Level::ERR;
    const std::string default_output = "stderr";
    auto level = default_level;
    auto flush_level = default_flush_level;
    std::unique_ptr<logger::Sink> sink;

    env_var_name << "UR_LOG_" << logger_name;
    auto map = getenv_to_map(env_var_name.str().c_str());
    if (!map.has_value()) {
        return Logger(std::make_unique<logger::StderrSink>(logger_name));
    }

    try {
        auto kv = map->find("level");
        if (kv != map->end()) {
            auto value = kv->second.front();
            level = str_to_level(value);
        }

        kv = map->find("flush");
        if (kv != map->end()) {
            auto value = kv->second.front();
            flush_level = str_to_level(value);
        }

        std::vector<std::string> values = {default_output};
        kv = map->find("output");
        if (kv != map->end()) {
            values = kv->second;
        }

        sink = values.size() == 2 ? sink_from_str(logger_name, values[0], values[1])
                                  : sink_from_str(logger_name, values[0]);
    } catch (const std::invalid_argument &e) {
        std::cerr << "Error when creating a logger instance from environment variable" << e.what();
        return Logger(std::make_unique<logger::StderrSink>(logger_name));
    }
    sink->setFlushLevel(flush_level);

    return Logger(level, std::move(sink));
}

} // namespace logger

#endif /* UR_LOGGER_HPP */
