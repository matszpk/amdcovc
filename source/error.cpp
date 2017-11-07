#include "error.h"

Error::Error(const char* _description) : description(_description)
{

}

Error::Error(int error, const char* _description)
{
    char errorBuf[32];
    snprintf(errorBuf, 32, "code %d: ", error);
    description = errorBuf;
    description += _description;
}

const char* Error::what() const noexcept
{
    return description.c_str();
}

Error::~Error() noexcept
{

}
