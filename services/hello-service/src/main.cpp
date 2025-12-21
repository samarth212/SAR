#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http  = beast::http;
using tcp = asio::ip::tcp;

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
  metrics_requests_total++;

  http::response<http::string_body> res;
  res.version(req.version());
  res.keep_alive(req.keep_alive());

  if (req.method() != http::verb::get) {
    res.result(http::status::method_not_allowed);
    res.set(http::field::content_type, "text/plain");
    res.body() = "Only GET is supported in hello-service.\n";
    res.prepare_payload();
    return res;
  }

  std::string target = std::string(req.target());

  if (target == "/healthz") {
    res.result(http::status::ok);
    res.set(http::field::content_type, "text/plain");
    res.body() = "ok\n";
  } else if (target == "/readyz") {
    // Phase 0: “ready” just means the server is up.
    // Later services will check real dependencies here.
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
    res.result(http::status::not_found);
    res.set(http::field::content_type, "text/plain");
    res.body() = "not found\n";
  }

  res.prepare_payload();
  return res;
}

int main() {
  const std::string service_name = "hello-service";

  // PORT is a standard container-friendly pattern.
  // We’ll use fixed ports per service later, but this helps reuse templates.
  int port = 7001;
  if (const char* p = std::getenv("PORT")) {
    port = std::atoi(p);
  }

  try {
    asio::io_context ioc;

    tcp::acceptor acceptor(ioc, {tcp::v4(), static_cast<unsigned short>(port)});

    log_json(service_name, "INFO",
             "listening on port " + std::to_string(port));

    uint64_t requests_total = 0;

    for (;;) {
      tcp::socket socket(ioc);
      acceptor.accept(socket);

      beast::flat_buffer buffer;
      http::request<http::string_body> req;
      http::read(socket, buffer, req);

      auto res = handle_request(req, service_name, requests_total);
      http::write(socket, res);

      // Close socket if client didn't request keep-alive.
      beast::error_code ec;
      socket.shutdown(tcp::socket::shutdown_send, ec);
    }
  } catch (const std::exception& e) {
    log_json(service_name, "ERROR", std::string("fatal: ") + e.what());
    return 1;
  }
}
