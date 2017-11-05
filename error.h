#ifndef ERROR_H
#define ERROR_H

class Error: public std::exception
{

private:

    std::string description;

public:

    explicit Error(const char* _description) : description(_description);

    Error(int error, const char* _description);

    virtual ~Error() noexcept;

    const char* what() const noexcept;

};


#endif /* ERROR_H */
