#pragma once

#include "uri.h"

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>

#include <llhttp.h>
#include <uv.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#define MAX_WRITE_HANDLES 1000

#define ASSERT_STATUS(status, msg) \
  if (status != 0) { \
    std::cerr << msg << ": " << uv_err_name(status); \
    exit(1); \
  }

namespace simphttp {

template<class Type>
class Buffer;
template<class Type>
class IStream;

class Request;
class Response;
class Client;
class Context;

extern const std::string CRLF;
extern void free_context(uv_handle_t *);

template<class Type>
extern void attachEvents(Type *instance, llhttp_settings_t &settings);

template<class Type>
class Buffer : public std::stringbuf {

    friend class Request;
    friend class Response;

    Type *stream;

    explicit Buffer<Type>(std::ostream &str) {};
    ~Buffer() override = default;
    int sync() override {

        std::string out = str();
        std::ostringstream buf;
        buf << out;
        out = buf.str();
        stream->writeOrEnd(out, true);
        buf.flush();
        str("");
        return 0;
    }
};

class Request {
public:
    std::string url;
    std::string method;
    std::string status_code;
    std::stringstream body;
    std::map<const std::string, const std::string> headers;

    Request() = default;
    ~Request() = default;
};

class Response : public std::ostream {

    friend class Buffer<class Response>;
    friend class Server;

    std::stringstream stream;
    Buffer<Response> buffer;

    void writeOrEnd(const std::string &, bool);

    int write_count = 0;
    bool writtenOrEnded = false;
    bool ended = false;
    bool headersSet = false;
    bool statusSet = false;
    bool contentLengthSet = false;

public:
    llhttp_t parser = {};

    int statusCode = 200;
    std::string body;
    std::string statusAdjective = "OK";
    std::map<const std::string, const std::string> headers;

    void setHeader(const std::string &, const std::string &);
    void setStatus(int);
    void setStatus(int, const std::string &);

    void write(const std::string &);
    void end(const std::string &);
    void end();

    Response() :
        std::ostream(&buffer),
        buffer(stream) {
        buffer.stream = this;
    }
    ~Response() override = default;
};

/*
  // @TODO
  // Maybe have each op call write
  //
  inline Response &operator << (Response &res, const std::string &s) {
  res.write(s);
  return res;
}*/


class Context : public Request {
public:
    std::map<int, uv_write_t> writes;
    uv_tcp_t handle;
    uv_connect_t connect_req;
    uv_write_t write_req;
    llhttp_t parser;
};

class Client {
    template<typename Type>
    friend void attachEvents(Type *instance, llhttp_settings_t &settings);
    friend class Response;

private:
    typedef std::function<void(
        Response &res)> Listener;

    Listener listener;
    uv_loop_t *loop_ = nullptr;
    uv_tcp_t socket_ = {};

    void connect();
    int complete(llhttp_t *parser, const Listener &fn);
    void on_connect(uv_connect_t *req, int status);

protected:
    uv_getaddrinfo_t addr_req;
    uv_shutdown_t shutdown_req;

public:
    struct Options {
        std::string host = "localhost";
        int port = 80;
        std::string method = "GET";
        std::string url = "/";
    };

    Options opts;

    Client(Options o, Listener listener);
    Client(std::string u, Listener listener);
    ~Client() = default;
};

class Server {
    template<typename Type>
    friend void attachEvents(Type *instance, llhttp_settings_t &settings);
    friend class Response;

private:
    typedef std::function<void(
        Request &req,
        Response &res)> Listener;

    Listener listener;
    uv_loop_t *loop_ = nullptr;
    uv_tcp_t socket_ = {};

    int complete(llhttp_t *parser, const Listener &fn);

public:
    explicit Server(Listener listener);
    ~Server() = default;
    int listen(const char *, int);
};

} // namespace http
