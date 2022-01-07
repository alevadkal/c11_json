/// Copyright © Alexander Kaluzhnyy
#include <gmock/gmock.h>

class system_mock {
private:
    static system_mock* thiz;
    system_mock(const system_mock&) = delete;
    void operator=(const system_mock&) = delete;

public:
    MOCK_METHOD(void*, calloc, (size_t __nmemb, size_t __size));
    MOCK_METHOD(void*, realloc, (void* __ptr, size_t __size));
    MOCK_METHOD(char*, strdup, (const char* __s));
    MOCK_METHOD(void, free, (void*));
    system_mock();
    static system_mock* instance();
    ~system_mock();
    bool VerifyAndClear()
    {
        return ::testing::Mock::VerifyAndClear(this);
    }
};