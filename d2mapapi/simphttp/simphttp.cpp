#include "simphttp.h"

#include <string_view>

namespace simphttp {

const std::string CRLF = "\r\n";

Server::Server(Listener fn) :
    listener(std::move(fn)) {

}

Client::Client(std::string ustr, Listener fn) :
    listener(std::move(fn)) {
    auto u = uri::ParseHttpUrl(ustr);
    opts.host = u.host;
    if (u.port) {
        opts.port = u.port;
    }
    if (!u.path.empty()) {
        opts.url = uri::UriEncode(u.path);
    }

    connect();
}

Client::Client(Client::Options o, Listener fn) :
    listener(std::move(fn)) {
    opts = std::move(o);
    listener = fn;
    connect();
}

void free_context(uv_handle_t *handle) {
    auto *context = reinterpret_cast<Context *>(handle->data);
    context->writes.clear();
    free(context);
}

//
// Events
//
template<class Type>
void attachEvents(Type *instance, llhttp_settings_t &settings) {

    // http parser callback types
    static std::function<int(llhttp_t *)> on_message_complete;

    static auto callback = instance->listener;
    // called once a connection has been made and the message is complete.
    on_message_complete = [&](llhttp_t *parser) -> int {
        return instance->complete(parser, callback);
    };

    // called after the url has been parsed.
    settings.on_url =
        [](llhttp_t *parser, const char *at, size_t len) -> int {
            auto *context = static_cast<Context *>(parser->data);
            if (at && context) { context->url = std::string(at, len); }
            return 0;
        };

    // called when there are either fields or values in the request.
    settings.on_header_field =
        [](llhttp_t *parser, const char *at, size_t length) -> int {
            return 0;
        };

    // called when header value is given
    settings.on_header_value =
        [](llhttp_t *parser, const char *at, size_t length) -> int {
            return 0;
        };

    // called once all fields and values have been parsed.
    settings.on_headers_complete =
        [](llhttp_t *parser) -> int {
            auto *context = static_cast<Context *>(parser->data);
            context->method = std::string(llhttp_method_name((llhttp_method_t)parser->method));
            return 0;
        };

    // called when there is a body for the request.
    settings.on_body =
        [](llhttp_t *parser, const char *at, size_t len) -> int {
            auto *context = static_cast<Context *>(parser->data);
            if (at && context && (int)len > -1) {
                context->body << std::string_view(at, len);
            }
            return 0;
        };

    // called after all other events.
    settings.on_message_complete =
        [](llhttp_t *parser) -> int {
            return on_message_complete(parser);
        };
}

template void attachEvents<Client>(Client *instance, llhttp_settings_t &settings);
template void attachEvents<Server>(Server *instance, llhttp_settings_t &settings);

//
// Response.
//
void Response::setHeader(const std::string &key, const std::string &val) {
    headersSet = true;
    if (writtenOrEnded) throw std::runtime_error("Can not set headers after write");

    if (key == "Content-Length") {
        contentLengthSet = true;
    }
    headers.emplace(key, val);
}

void Response::setStatus(int code) {

    statusSet = true;
    if (writtenOrEnded) throw std::runtime_error("Can not set status after write");
    statusCode = code;
}

void Response::setStatus(int code, const std::string &ad) {

    statusSet = true;
    if (writtenOrEnded) throw std::runtime_error("Can not set status after write");
    statusCode = code;
    statusAdjective = ad;
}

void Response::writeOrEnd(const std::string &str, bool end) {

    if (ended) throw std::runtime_error("Can not write after end");

    std::stringstream ss;

    if (!writtenOrEnded) {

        ss << "HTTP/1.1 " << statusCode << " " << statusAdjective << CRLF;

        for (auto &header: headers) {
            ss << header.first << ": " << header.second << CRLF;
        }
        ss << CRLF;
        writtenOrEnded = true;
    }

    bool isChunked = headers.count("Transfer-Encoding")
        && headers["Transfer-Encoding"] == "chunked";

    if (isChunked) {
        ss << std::hex << str.size()
           << std::dec << CRLF << str << CRLF;
    } else {
        ss << str;
    }

    if (isChunked && end) {
        ss << "0" << CRLF << CRLF;
    }

    auto str2 = ss.str();

    // response buffer
    uv_buf_t resbuf = {
        .len = (unsigned long)str2.size(),
        .base = (char *)str2.c_str(),
    };

    auto *context = static_cast<Context *>(this->parser.data);

    auto id = write_count++;

    uv_write_t write_req;
    context->writes.insert({id, write_req});

    if (end) {

        ended = true;

        uv_write(&context->writes.at(id), (uv_stream_t *)&context->handle, &resbuf, 1,
                 [](uv_write_t *req, int status) {
                     if (!uv_is_closing((uv_handle_t *)req->handle)) {
                         uv_close((uv_handle_t *)req->handle, free_context);
                     }
                 }
        );
    } else {
        uv_write(&context->writes.at(id), (uv_stream_t *)&context->handle, &resbuf, 1, nullptr);
    }
}

void Response::write(const std::string &s) {
    this->writeOrEnd(s, false);
}

void Response::end(const std::string &s) {
    this->writeOrEnd(s, true);
}

void Response::end() {
    this->writeOrEnd("", true);
}

int Server::complete(llhttp_t *parser, const Listener &cb) {
    auto *context = reinterpret_cast<Context *>(parser->data);
    Request req;
    Response res;

    req.url = uri::UriDecode(context->url);
    req.method = context->method;
    res.parser = *parser;
    cb(req, res);
    return 0;
}

int Server::listen(const char *ip, int port) {
    //
    // parser settings needs to be static.
    //
    //
    static llhttp_settings_t settings;
    attachEvents(this, settings);

    int status = 0;

#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int cores = sysinfo.dwNumberOfProcessors;
#else
    int cores = sysconf(_SC_NPROCESSORS_ONLN);
#endif

    std::stringstream cores_string;
    cores_string << cores;

#ifdef _WIN32
    SetEnvironmentVariable("UV_THREADPOOL_SIZE", cores_string.str().c_str());
#else
    setenv("UV_THREADPOOL_SIZE", cores_string.str().c_str(), 1);
#endif

    sockaddr_storage address = {};

    static std::function<void(uv_stream_t *, int)> on_connect;
    static std::function<void(uv_stream_t *, ssize_t, const uv_buf_t *)> read;

    loop_ = uv_default_loop();
    uv_tcp_init(loop_, &socket_);

    //
    // @TODO - Not sure exactly how to use this,
    // after the initial timeout, it just
    // seems to kill the server.
    //
    //uv_tcp_keepalive(&socket_,1,60);
    uv_tcp_nodelay(&socket_, 1);

    status = uv_ip6_addr(ip, port, (struct sockaddr_in6 *)&address);
    if (status != 0) {
        status = uv_ip4_addr(ip, port, (struct sockaddr_in *)&address);
    }
    ASSERT_STATUS(status, "Resolve Address");

    status = uv_tcp_bind(&socket_, (const struct sockaddr *)&address, 0);
    ASSERT_STATUS(status, "Bind");

    // called once a connection is made.
    on_connect = [&](uv_stream_t *handle, int status) {
        auto *context = new Context();

        // init tcp handle
        uv_tcp_init(loop_, &context->handle);

        // init http parser
        llhttp_init(&context->parser, HTTP_REQUEST, &settings);

        // client reference for parser routines
        context->parser.data = context;

        // client reference for handle data on requests
        context->handle.data = context;

        // accept connection passing in refernce to the client handle
        uv_accept(handle, (uv_stream_t *)&context->handle);

        // called for every read
        read = [&](uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf) {
            ssize_t parsed;
            auto *context = static_cast<Context *>(tcp->data);

            if (nread >= 0) {
                parsed = (ssize_t)llhttp_execute(&context->parser,
                                                 buf->base,
                                                 nread);

                // close handle
                if (parsed < nread) {
                    uv_close((uv_handle_t *)&context->handle, free_context);
                }
            } else {
                if (nread != UV_EOF) {
                    // @TODO - debug error
                }

                // close handle
                uv_close((uv_handle_t *)&context->handle, free_context);
            }

            // free request buffer data
            free(buf->base);
        };

        // allocate memory and attempt to read.
        uv_read_start((uv_stream_t *)&context->handle,
            // allocator
                      [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
                          *buf = uv_buf_init((char *)malloc(suggested_size), suggested_size);
                      },

            // reader
                      [](uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf) {
                          read(tcp, nread, buf);
                      });
    };

    status = uv_listen((uv_stream_t *)&socket_, MAX_WRITE_HANDLES,
        // listener
                       [](uv_stream_t *socket, int status) {
                           on_connect(socket, status);
                       });

    ASSERT_STATUS(status, "Listen");

    // init loop
    uv_run(loop_, UV_RUN_DEFAULT);
    return 0;
}

int Client::complete(llhttp_t *parser, const Listener &cb) {
    auto *context = reinterpret_cast<Context *>(parser->data);

    Response res;
    res.body = context->body.str();
    res.parser = *parser;

    cb(res);
    return 0;
}

void Client::on_connect(uv_connect_t *req, int status) {

    auto *context = reinterpret_cast<Context *>(req->handle->data);

    if (status == -1) {
        // @TODO
        // Error Callback
        uv_close((uv_handle_t *)req->handle, free_context);
        return;
    }

    static std::function<void(
        uv_stream_t *, ssize_t, const uv_buf_t *)> read;

    read = [&](uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf) {

        auto *context = static_cast<Context *>(tcp->data);

        if (nread >= 0) {
            auto parsed = (ssize_t)llhttp_execute(
                &context->parser, buf->base, nread);

            if (parsed < nread) {
                uv_close((uv_handle_t *)&context->handle, free_context);
            }
            if (parsed != nread) {
                // @TODO
                // Error Callback
            }
        } else {
            if (nread != UV_EOF) {
                return; // maybe do something interesting here...
            }
            uv_close((uv_handle_t *)&context->handle, free_context);
        }
        free(buf->base);
    };

    uv_buf_t reqbuf;
    std::string reqstr =
        opts.method + " " + opts.url + " HTTP/1.1" + CRLF +
            //
            // @TODO
            // Add user's headers here
            //
            "Connection: keep-alive" + CRLF + CRLF;

    reqbuf.base = (char *)reqstr.c_str();
    reqbuf.len = reqstr.size();

    uv_read_start(
        req->handle,
        [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
            *buf = uv_buf_init((char *)malloc(suggested_size), suggested_size);
        },
        [](uv_stream_t *tcp, ssize_t nread, const uv_buf_t *buf) {
            read(tcp, nread, buf);
        });

    uv_write(
        &context->write_req,
        req->handle,
        &reqbuf,
        1,
        nullptr);
}

void Client::connect() {
    addrinfo ai = {
        .ai_flags = 0,
        .ai_family = PF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
    };

    loop_ = uv_default_loop();

    static std::function<void(
        uv_getaddrinfo_t *req, int status, struct addrinfo *res)> on_resolved;

    static std::function<void(uv_connect_t *req, int status)> on_before_connect;

    on_before_connect = [&](uv_connect_t *req, int status) {

        // @TODO
        // Populate address and time info for logging / stats etc.

        on_connect(req, status);
    };

    on_resolved = [&](uv_getaddrinfo_t *req, int status, struct addrinfo *res) {

        static llhttp_settings_t settings;
        attachEvents(this, settings);

        char addr[17] = {'\0'};

        uv_ip4_name((struct sockaddr_in *)res->ai_addr, addr, 16);
        uv_freeaddrinfo(res);

        sockaddr_in dest = {};
        uv_ip4_addr(addr, opts.port, &dest);

        auto *context = new Context();

        context->handle.data = context;
        llhttp_init(&context->parser, HTTP_RESPONSE, &settings);
        context->parser.data = context;

        uv_tcp_init(loop_, &context->handle);
        //uv_tcp_keepalive(&context->handle, 1, 60);

        uv_tcp_connect(
            &context->connect_req,
            &context->handle,
            (const struct sockaddr *)&dest,
            [](uv_connect_t *req, int status) {
                on_before_connect(req, status);
            });
    };

    auto cb = [](uv_getaddrinfo_t *req, int status, struct addrinfo *res) {
        on_resolved(req, status, res);
    };

    uv_getaddrinfo(loop_, &addr_req, cb, opts.host.c_str(), std::to_string(opts.port).c_str(), &ai);
    uv_run(loop_, UV_RUN_DEFAULT);
}

}
