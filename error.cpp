#include "error.h"


explicit Error::Error(const char* _description) : description(_description)
{

}

Error::Error(int error, const char* _description)
{
    char errorBuf[32];
    snprintf(errorBuf, 32, "code %d: ", error);
    description = errorBuf;
    description += _description;
}

virtual Error::~Error() noexcept
{

}

const char* Error::what() const noexcept
{
    return description.c_str();
}
