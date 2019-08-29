// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef PRINT_DEBUG_H
#define PRINT_DEBUG_H

#ifdef NDEBUG
#define PRINT_DEBUG(format, ...)
#define PRINT_VERBOSE(format, ...)
#else
#include <stdio.h>
#include <string>
#include <mutex>
static std::mutex g_stderr_mutex;
#define PRINT_DEBUG(format, ...) { std::lock_guard<std::mutex> guard(g_stderr_mutex); fprintf(stderr, format, ##__VA_ARGS__); fprintf(stderr, "\n"); }
#ifdef VERBOSE
#define PRINT_VERBOSE PRINT_DEBUG
#else
#ifdef PRINT_VERBOSE
#undef PRINT_VERBOSE
#endif
#define PRINT_VERBOSE(format, ...)
#endif // VERBOSE
#endif // NDEBUG

#endif // PRINT_DEBUG_H
