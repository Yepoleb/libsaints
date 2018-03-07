#include <stdexcept>


namespace Saints {

class ParsingError : public std::runtime_error
{
public:
    explicit ParsingError(const char* what_arg) : std::runtime_error(what_arg) { }
    explicit ParsingError(QString what_arg) : std::runtime_error(what_arg.toUtf8()) { }
};

class FieldError : public ParsingError
{
public:
    explicit FieldError(const QString& name, const QString& value) :
        m_name(name),
        m_value(value),
        ParsingError(QString("Invalid value in field %1 (%2)").arg(name, value))
    {

    }

    QString getName() const {return m_name;}
    QString getValue() const {return m_value;}

private:
    QString m_name;
    QString m_value;
};

class IOError : public std::runtime_error
{
public:
    explicit IOError(const char* what_arg) : std::runtime_error(what_arg) { }
    explicit IOError(QString what_arg) : std::runtime_error(what_arg.toUtf8()) { }
};

}
