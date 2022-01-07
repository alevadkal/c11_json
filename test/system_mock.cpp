/// Copyright © Alexander Kaluzhnyy

#include <gmock/gmock.h>
#include <stdexcept>
#include <system_mock.hpp>
#include <stdio.h>
#include <exception>
#include "log.h"

#define wrap(name) __wrap_##name
#define real(name) __real_##name

extern "C" {

void* wrap(calloc)(size_t nmemb, size_t size);
void* real(calloc)(size_t nmemb, size_t size);
void* wrap(realloc)(void* ptr, size_t size);
void* real(realloc)(void* ptr, size_t size);
char* wrap(strdup)(const char* s);
char* real(strdup)(const char* s);
void wrap(free)(void*);
void real(free)(void*);
}

using ::testing::_;
using ::testing::Invoke;

system_mock* system_mock::thiz
    = nullptr;

system_mock::system_mock()
{
    log_trace_func();
    if (thiz) {
        throw std::runtime_error("class already have instance!");
    }
    ON_CALL(*this, calloc(_, _)).WillByDefault(Invoke(real(calloc)));
    ON_CALL(*this, realloc(_, _)).WillByDefault(Invoke(real(realloc)));
    ON_CALL(*this, strdup(_)).WillByDefault(Invoke(real(strdup)));
    ON_CALL(*this, free(_)).WillByDefault(Invoke(real(free)));
    thiz = this;
}
system_mock* system_mock::instance()
{
    return system_mock::thiz;
}

system_mock::~system_mock()
{
    log_trace_func();
    thiz = nullptr;
}

#define mock_call(call) (system_mock::instance() ? system_mock::instance()->call : __real_##call)

void* wrap(calloc)(size_t nmemb, size_t size)
{
    log_trace_func();
    return mock_call(calloc(nmemb, size));
}

void* wrap(realloc)(void* ptr, size_t size)
{
    log_trace_func();
    return mock_call(realloc(ptr, size));
}

char* wrap(strdup)(const char* s)
{
    log_trace_func();
    return mock_call(strdup(s));
}

void wrap(free)(void* ptr)
{
    log_trace_func();
    mock_call(free(ptr));
}
