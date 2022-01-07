/// Copyright © Alexander Kaluzhnyy

#include <gmock/gmock.h>
#include <stdexcept>
#include <system_mock.hpp>
#include <stdio.h>
#include <exception>
#include "log.h"

#define wrap(name) __wrap_##name

extern "C" {
void* wrap(malloc)(size_t size);
void* wrap(calloc)(size_t nmemb, size_t size);
void* wrap(realloc)(void* ptr, size_t size);
char* wrap(strdup)(const char* s);
void wrap(free)(void*);
}

using namespace ::testing;

static void setup_default_behaviour(system_mock* mock)
{
    ON_CALL(*mock, malloc(_)).WillByDefault(Invoke(real(malloc)));
    ON_CALL(*mock, calloc(_, _)).WillByDefault(Invoke(real(calloc)));
    ON_CALL(*mock, realloc(_, _)).WillByDefault(Invoke(real(realloc)));
    ON_CALL(*mock, strdup(_)).WillByDefault(Invoke(real(strdup)));
    ON_CALL(*mock, free(_)).WillByDefault(Invoke(real(free)));
}

system_mock* system_mock::thiz = nullptr;

system_mock::system_mock()
{
    log_trace_func();
    if (thiz) {
        throw std::runtime_error("class already have instance!");
    }
    setup_default_behaviour(this);
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

#define mock_call(call) (system_mock::instance() ? ({log_trace_func();system_mock::instance()->call; }) : __real_##call)

void* wrap(malloc)(size_t size)
{
    return mock_call(malloc(size));
}

void* wrap(calloc)(size_t nmemb, size_t size)
{
    return mock_call(calloc(nmemb, size));
}

void* wrap(realloc)(void* ptr, size_t size)
{
    return mock_call(realloc(ptr, size));
}

char* wrap(strdup)(const char* s)
{
    return mock_call(strdup(s));
}

void wrap(free)(void* ptr)
{
    mock_call(free(ptr));
}
