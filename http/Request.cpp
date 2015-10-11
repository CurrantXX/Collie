#include "Request.hpp"
#include "../base/Utility.hpp"
#include "../httpd.hpp"


namespace MiniHttp { namespace Http {

Request::Request() :
    method(Method::NONE),
    url(),
    httpVersion() {}

Request Request::parse(const std::string& str) {
    std::stringstream in(str);
    Request req;
    std::string line;

    std::getline(in, line);  // get url, method and httpVersion

    auto head = Base::StringHelper::split(line, ' ');
    if(head.size() != 3) {
        Log(WARN) << "Unknow header field: " << line;
        return req;
    }
    req.method = getMethod(head[0]);
    req.url = head[1];
    req.httpVersion = head[2];

    while(std::getline(in, line)) {
        Base::StringHelper::removeSpace(line);
        const std::size_t foundPos = line.find(':');
        if(foundPos != std::string::npos) {
            std::string field = line.substr(0, foundPos);
            std::string content = line.substr(foundPos + 1);
            req.header[field] = content;
        }
    }
    return req;
}
}}
