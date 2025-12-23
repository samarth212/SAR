#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

// websocket packages
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http  = beast::http;
using tcp = asio::ip::tcp;

// build utc string for logs
static std::string now_iso8601_utc() {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto t = system_clock::to_time_t(now);
  std::tm tm{};
#if defined(_WIN32)
  gmtime_s(&tm, &t);
#else
  gmtime_r(&t, &tm);
#endif
  char buf[32];
  std::snprintf(buf, sizeof(buf),
                "%04d-%02d-%02dT%02d:%02d:%02dZ",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec);
  return std::string(buf);
}

 
static void log_json(const std::string& service,
                     const std::string& level,
                     const std::string& msg) {
  boost::json::object o;
  o["ts"] = now_iso8601_utc();
  o["service"] = service;
  o["level"] = level;
  o["msg"] = msg;
  std::cout << boost::json::serialize(o) << "\n";
}


static http::response<http::string_body>
handle_request(const http::request<http::string_body>& req,
               const std::string& service_name,
               uint64_t& metrics_requests_total) {

  // increase count of total requests
  metrics_requests_total++;

  // create http response object
  http::response<http::string_body> res;
  res.version(req.version());
  res.keep_alive(req.keep_alive());

  // check if the request is GET because these paths only accept get
  if (req.method() != http::verb::get) {
    res.result(http::status::method_not_allowed);
    res.set(http::field::content_type, "text/plain");
    res.body() = "Only GET is supported in hello-service.\n";
    res.prepare_payload();
    return res;
  }

  // string path of the target request path
  std::string target = std::string(req.target());

  // handle when it hits these endpoints
  if (target == "/healthz") {
    res.result(http::status::ok);
    res.set(http::field::content_type, "text/plain");
    res.body() = "ok\n";
  } else if (target == "/readyz") {
   
    res.result(http::status::ok);
    res.set(http::field::content_type, "text/plain");
    res.body() = "ready\n";
  } else if (target == "/metrics") {
    res.result(http::status::ok);
    res.set(http::field::content_type, "text/plain; version=0.0.4");
    res.body() =
      "hello_requests_total " + std::to_string(metrics_requests_total) + "\n";
  } else if (target == "/") {
    res.result(http::status::ok);
    res.set(http::field::content_type, "text/plain");
    res.body() = "hello from SAR (Phase 0)\n";
  } else {
    // not found
    res.result(http::status::not_found);
    res.set(http::field::content_type, "text/plain");
    res.body() = "not found\n";
  }

  res.prepare_payload();
  return res;
}

int main() {
  const std::string service_name = "hello-service";

  // choose port from the env
  // docker/k8 will define port in the .env so use that one instead if exists
  int port = 7001;
  if (const char* p = std::getenv("PORT")) {
    port = std::atoi(p);
  }

  try {

    // engine that runs the networking operations
    asio::io_context ioc;

    // acceptor listening on that port for requests
    tcp::acceptor acceptor(ioc, {tcp::v4(), static_cast<unsigned short>(port)});

    log_json(service_name, "INFO",
             "listening on port " + std::to_string(port));

    uint64_t requests_total = 0;

    // infitie loop 
    for (;;) {

      // creates new TCP socket  
      tcp::socket socket(ioc);

      // the program will pause here until a client (web browser) connects
      // once a connection is made the socket object will contain the details of the connection 
      acceptor.accept(socket);

      // hold request and parse functionality
      beast::flat_buffer buffer;
      http::request<http::string_body> req;

      // pulls the raw data from the socket, stores it with buffer and then parses it to req
      // it waits until the full HTTP request has been received
      http::read(socket, buffer, req);

      // handles request and returns HTTP response object 
      auto res = handle_request(req, service_name, requests_total);

      // server sends the generated response back to the client through the socket
      // client recieves the data
      http::write(socket, res);

      // shutdown socket, go back to top to wait for the next connection
      beast::error_code ec;
      socket.shutdown(tcp::socket::shutdown_send, ec);
    }
  } catch (const std::exception& e) {
    log_json(service_name, "ERROR", std::string("fatal: ") + e.what());
    return 1;
  }
}
