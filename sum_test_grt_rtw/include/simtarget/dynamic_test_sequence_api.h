/* Copyright 2021 The MathWorks, Inc. */
#ifndef DYNAMIC_TEST_SEQUENCE_API_H
#define DYNAMIC_TEST_SEQUENCE_API_H
#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#include <string>
#include "dynamic_ts_spec.hpp"

/* c API */
SHARED_DYNAMICTESTSEQUENCE_CORE_EXPORT bool dynamictestseq_dequeue_command(const std::string command);

SHARED_DYNAMICTESTSEQUENCE_CORE_EXPORT std::string dynamictestseq_get_curr_sequence_name();

SHARED_DYNAMICTESTSEQUENCE_CORE_EXPORT bool dynamictestseq_library_loaded(const std::string sequenceName);

SHARED_DYNAMICTESTSEQUENCE_CORE_EXPORT void* dynamictestseq_get_fcn_ptr(const std::string sequenceName, const std::string fcnName);

SHARED_DYNAMICTESTSEQUENCE_CORE_EXPORT void dynamictestseq_reset_sequence();

SHARED_DYNAMICTESTSEQUENCE_CORE_EXPORT void dynamictestseq_is_stopped();

SHARED_DYNAMICTESTSEQUENCE_CORE_EXPORT int dynamictestseq_get_commands_count(const int queueIndex);

#endif
