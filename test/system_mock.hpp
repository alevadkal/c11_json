/// Copyright © Alexander Kaluzhnyy
#include <gmock/gmock.h>

#define real(name) __real_##name

class system_mock {
private:
    static system_mock* thiz;
    system_mock(const system_mock&) = delete;
    void operator=(const system_mock&) = delete;

public:
    MOCK_METHOD(void*, malloc, (size_t size));
    MOCK_METHOD(void*, calloc, (size_t nmemb, size_t size));
    MOCK_METHOD(void*, realloc, (void* ptr, size_t size));
    MOCK_METHOD(char*, strdup, (const char* s));
    MOCK_METHOD(void, free, (void*));
    system_mock();
    static system_mock* instance();
    ~system_mock();
    bool VerifyAndClearExpectations();
    bool VerifyAndClear();
};

extern "C" {
void* real(malloc)(size_t size);
void* real(calloc)(size_t nmemb, size_t size);
void* real(realloc)(void* ptr, size_t size);
char* real(strdup)(const char* s);
void real(free)(void*);
}
