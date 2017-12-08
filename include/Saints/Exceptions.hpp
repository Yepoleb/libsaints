#include <stdexcept>


namespace Saints {

class ParsingError : public std::runtime_error
{
public:
    explicit ParsingError(const char* what_arg) : std::runtime_error(what_arg) { }
};

class IOError : public std::runtime_error
{
public:
    explicit IOError(const char* what_arg) : std::runtime_error(what_arg) { }
};

}
