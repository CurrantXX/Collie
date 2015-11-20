#ifndef COLLIE_EXCEPTION_H
#define COLLIE_EXCEPTION_H

#include <cstring>
#include <exception>
#include <stdexcept>
#include <iostream>

namespace Collie {

/**
 * get system error message
 */
inline std::string
getError() {
    return std::string(strerror(errno));
}

/**
 * format info
 */
inline std::string
getDetail(const std::string & file, const unsigned & line,
          const std::string & func, const std::string & msg = "") {
    const std::string File = "File: " + file + "(" + std::to_string(line) + ")";
    const std::string Function = "Function : " + func;
    const std::string Message = "Message: " + msg;
    return "\n" + File + "\n" + Function + "\n" + Message;
}

class Exception : public std::exception {
public:
    explicit Exception(const std::string & msg) noexcept : msg(msg) {}
    explicit Exception(const char * msg) noexcept : msg(msg) {}
    ~Exception() noexcept override {}
    const char * what() const noexcept override { return msg.c_str(); }

protected:
    const std::string msg;
};

#define EXC_DETAIL ::Collie::getDetail(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#define THROW throw ::Collie::Exception(EXC_DETAIL);
#define THROW_SYS                                                              \
    throw ::Collie::Exception(EXC_DETAIL + "\nSystem Error: " +                \
                              ::Collie::getError());

#define EXC_DETAIL_(msg)                                                       \
    ::Collie::getDetail(__FILE__, __LINE__, __PRETTY_FUNCTION__, msg)
#define THROW_(msg) throw ::Collie::Exception(EXC_DETAIL_(msg));
#define THROW_SYS_(msg)                                                        \
    throw ::Collie::Exception(EXC_DETAIL_(msg) + "\nSystem Error: " +          \
                              ::Collie::getError());

#define REQUIRE(CONDITION)                                                     \
    if(!(CONDITION)) {                                                         \
        std::cout << EXC_DETAIL << std::endl << "REQUIRE" << std::endl;        \
        abort();                                                               \
    }
#define REQUIRE_SYS(CONDITION)                                                 \
    if(!(CONDITION)) {                                                         \
        std::cout << EXC_DETAIL << std::endl                                   \
                  << "System Error: " << ::Collie::getError() << std::endl;    \
        abort();                                                               \
    }

#define REQUIRE_(CONDITION, msg)                                               \
    if(!(CONDITION)) {                                                         \
        std::cout << EXC_DETAIL_(msg) << std::endl << "REQUIRE" << std::endl;  \
        abort();                                                               \
    }

#define TRACE_LOG Log(TRACE) << "TRACE log";
}

#endif /* COLLIE_EXCEPTION_H */
