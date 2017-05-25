//
// Created by dk on 17-5-25.
//

#ifndef PARALLELCOMPUTEEXAMPLE_LOGGER_H
#define PARALLELCOMPUTEEXAMPLE_LOGGER_H

#include <android/log.h>
#include <libgen.h> // for basename()

#define LOGV(LOG_TAG, FMT, ...) __android_log_print(ANDROID_LOG_VERBOSE, (LOG_TAG), "[%s:%d:%s]\n:"FMT,	\
        __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__)
#define LOGD(LOG_TAG, FMT, ...) __android_log_print(ANDROID_LOG_DEBUG, (LOG_TAG), "[%s:%d:%s]:"FMT,	\
        basename(__FILE__), __LINE__, __FUNCTION__, ## __VA_ARGS__)
#define LOGI(LOG_TAG, FMT, ...) __android_log_print(ANDROID_LOG_INFO, (LOG_TAG), "[%s:%d:%s]:"FMT,	\
        basename(__FILE__), __LINE__, __FUNCTION__, ## __VA_ARGS__)
#define LOGW(LOG_TAG, FMT, ...) __android_log_print(ANDROID_LOG_WARN, (LOG_TAG), "[%s:%d:%s]:"FMT,	\
        basename(__FILE__), __LINE__, __FUNCTION__, ## __VA_ARGS__)
#define LOGE(LOG_TAG, FMT, ...) __android_log_print(ANDROID_LOG_ERROR, (LOG_TAG), "[%s:%d:%s]:"FMT,	\
        basename(__FILE__), __LINE__, __FUNCTION__, ## __VA_ARGS__)
#define LOGF(LOG_TAG, FMT, ...) __android_log_print(ANDROID_LOG_FATAL, (LOG_TAG), "[%s:%d:%s]:"FMT,	\
        basename(__FILE__), __LINE__, __FUNCTION__, ## __VA_ARGS__)

#endif //PARALLELCOMPUTEEXAMPLE_LOGGER_H
