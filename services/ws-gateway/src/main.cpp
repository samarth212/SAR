#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

#include <cstdlib>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace asio      = boost::asio;
namespace beast     = boost::beast;
namespace http      = beast::http;
namespace websocket = beast::websocket;

using tcp = asio::ip::tcp;

// ---------- tiny helpers ----------
static int env_int(const char* name, int fallback) {
  if (const char* v = std::getenv(name)) return std::atoi(v);
  return fallback;
}

static std::string env_str(const char* name, const std::string& fallback) {
  if (const char* v = std::getenv(name)) return std::string(v);
  return fallback;
}

static http::response<http::string_body>
make_text(http::status st, unsigned version, bool keep_alive,
          const std::string& text) {
  http::response<http::string_body> res{st, version};
  res.set(http::field::content_type, "text/plain");
  res.keep_alive(keep_alive);
  res.body() = text;
  res.prepare_payload();
  return res;
}

// ALLOWED_ORIGINS="http://localhost:3000,http://127.0.0.1:3000"
static bool origin_allowed(const http::request<http::string_body>& req,
                           const std::string& allowed_csv) {
  if (allowed_csv.empty()) return true; // allow all if not set

  // Origin header might be missing for non-browser clients.
  // We only enforce when it's present.
  auto it = req.find(http::field::origin);
  if (it == req.end()) return true;

  std::string origin = it->value().to_string();

  // simple CSV match
  size_t start = 0;
  while (start < allowed_csv.size()) {
    size_t comma = allowed_csv.find(',', start);
    if (comma == std::string::npos) comma = allowed_csv.size();
    std::string token = allowed_csv.substr(start, comma - start);

    // trim spaces (lightweight)
    while (!token.empty() && token.front() == ' ') token.erase(token.begin());
    while (!token.empty() && token.back() == ' ') token.pop_back();

    if (!token.empty() && token == origin) return true;
    start = comma + 1;
  }
  return false;
}

// Forward decl
struct SharedState;

// ---------- WebSocket session ----------
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
 public:
  WebSocketSession(tcp::socket&& socket,
                   std::shared_ptr<SharedState> state)
      : ws_(std::move(socket)),
        strand_(ws_.get_executor()),
        state_(std::move(state)) {}

  void run(http::request<http::string_body> req);

  // Called by /broadcast to push a message to this client
  void send(std::string msg);

 private:
  void on_accept(beast::error_code ec);
  void do_read();
  void on_read(beast::error_code ec, std::size_t bytes);
  void do_write();
  void on_write(beast::error_code ec, std::size_t bytes);

  websocket::stream<tcp::socket> ws_;
  asio::strand<asio::any_io_executor> strand_;
  beast::flat_buffer buffer_;

  std::shared_ptr<SharedState> state_;

  // Queue to ensure we never have two writes at once
  std::deque<std::string> outbox_;
};

// ---------- Shared state (client list + broadcast) ----------
struct SharedState : public std::enable_shared_from_this<SharedState> {
  std::mutex mu;
  std::unordered_set<std::shared_ptr<WebSocketSession>> clients;

  void join(const std::shared_ptr<WebSocketSession>& s) {
    std::lock_guard<std::mutex> lock(mu);
    clients.insert(s);
  }

  void leave(const std::shared_ptr<WebSocketSession>& s) {
    std::lock_guard<std::mutex> lock(mu);
    clients.erase(s);
  }

  std::size_t client_count() const {
    // mutex is mutable pain; easiest: cast away const safely for this tiny call
    auto* self = const_cast<SharedState*>(this);
    std::lock_guard<std::mutex> lock(self->mu);
    return self->clients.size();
  }

  std::size_t broadcast(const std::string& msg) {
    std::vector<std::shared_ptr<WebSocketSession>> snapshot;
    {
      std::lock_guard<std::mutex> lock(mu);
      snapshot.reserve(clients.size());
      for (auto& c : clients) snapshot.push_back(c);
    }

    for (auto& c : snapshot) {
      c->send(msg);
    }
    return snapshot.size();
  }
};

// WebSocketSession impl
void WebSocketSession::run(http::request<http::string_body> req) {
  // Tell Beast we want text frames by default
  ws_.text(true);

  // Perform the WebSocket handshake
  ws_.async_accept(req,
    asio::bind_executor(
      strand_,
      beast::bind_front_handler(
        &WebSocketSession::on_accept,
        shared_from_this()
      )
    )
  );
}

void WebSocketSession::on_accept(beast::error_code ec) {
  if (ec) {
    return;
  }
  // Add ourselves to the shared client list
  state_->join(shared_from_this());

  // Start reading (keeps connection alive)
  do_read();
}

void WebSocketSession::do_read() {
  ws_.async_read(
    buffer_,
    asio::bind_executor(
      strand_,
      beast::bind_front_handler(
        &WebSocketSession::on_read,
        shared_from_this()
      )
    )
  );
}

void WebSocketSession::on_read(beast::error_code ec, std::size_t) {
  if (ec) {
    // Client disconnected or error; remove from list
    state_->leave(shared_from_this());
    return;
  }

  // We don't actually need to do anything with incoming messages in Phase 1.
  // Just clear buffer and keep reading so the socket stays alive.
  buffer_.consume(buffer_.size());
  do_read();
}

void WebSocketSession::send(std::string msg) {
  // IMPORTANT: all writes must happen on the session's strand.
  asio::post(
    strand_,
    [self = shared_from_this(), msg = std::move(msg)]() mutable {
      bool writing = !self->outbox_.empty();
      self->outbox_.push_back(std::move(msg));
      if (!writing) self->do_write();
    }
  );
}

void WebSocketSession::do_write() {
  ws_.async_write(
    asio::buffer(outbox_.front()),
    asio::bind_executor(
      strand_,
      beast::bind_front_handler(
        &WebSocketSession::on_write,
        shared_from_this()
      )
    )
  );
}

void WebSocketSession::on_write(beast::error_code ec, std::size_t) {
  if (ec) {
    state_->leave(shared_from_this());
    return;
  }
  outbox_.pop_front();
  if (!outbox_.empty()) do_write();
}

// ---------- HTTP session (handles one TCP connection for HTTP) ----------
class HttpSession : public std::enable_shared_from_this<HttpSession> {
 public:
  HttpSession(tcp::socket&& socket,
              std::shared_ptr<SharedState> state,
              std::string allowed_origins)
      : socket_(std::move(socket)),
        state_(std::move(state)),
        allowed_origins_(std::move(allowed_origins)) {}

  void run() { do_read(); }

 private:
  void do_read();
  void on_read(beast::error_code ec, std::size_t bytes);
  void send_response(http::response<http::string_body>&& res);

  tcp::socket socket_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;

  std::shared_ptr<SharedState> state_;
  std::string allowed_origins_;
};

void HttpSession::do_read() {
  req_ = {};
  http::async_read(
    socket_,
    buffer_,
    req_,
    beast::bind_front_handler(
      &HttpSession::on_read,
      shared_from_this()
    )
  );
}

void HttpSession::on_read(beast::error_code ec, std::size_t) {
  if (ec) return;

  const std::string target = std::string(req_.target());

  // If it's a WebSocket upgrade to /ws, hand the socket off to WebSocketSession.
  if (target == "/ws" && websocket::is_upgrade(req_)) {
    if (!origin_allowed(req_, allowed_origins_)) {
      send_response(make_text(http::status::forbidden, req_.version(), false,
                              "Origin not allowed\n"));
      return;
    }

    std::make_shared<WebSocketSession>(std::move(socket_), state_)->run(std::move(req_));
    return; // IMPORTANT: socket ownership moved, HttpSession ends here
  }

  // Regular HTTP endpoints
  if (req_.method() == http::verb::get && target == "/healthz") {
    send_response(make_text(http::status::ok, req_.version(), req_.keep_alive(), "ok\n"));
    return;
  }
  if (req_.method() == http::verb::get && target == "/readyz") {
    send_response(make_text(http::status::ok, req_.version(), req_.keep_alive(), "ready\n"));
    return;
  }
  if (req_.method() == http::verb::get && target == "/metrics") {
    std::string body = "ws_clients_connected " + std::to_string(state_->client_count()) + "\n";
    auto res = make_text(http::status::ok, req_.version(), req_.keep_alive(), body);
    res.set(http::field::content_type, "text/plain; version=0.0.4");
    send_response(std::move(res));
    return;
  }

  if (req_.method() == http::verb::post && target == "/broadcast") {
    // Phase 1: broadcast whatever body you send (expected JSON string)
    std::string msg = req_.body();
    std::size_t sent = state_->broadcast(msg);

    std::string body = "ok sent=" + std::to_string(sent) + "\n";
    send_response(make_text(http::status::ok, req_.version(), req_.keep_alive(), body));
    return;
  }

  send_response(make_text(http::status::not_found, req_.version(), req_.keep_alive(), "not found\n"));
}

void HttpSession::send_response(http::response<http::string_body>&& res) {
  auto sp = std::make_shared<http::response<http::string_body>>(std::move(res));
  http::async_write(
    socket_,
    *sp,
    [self = shared_from_this(), sp](beast::error_code, std::size_t) {
      beast::error_code ec;
      self->socket_.shutdown(tcp::socket::shutdown_send, ec);
    }
  );
}

// ---------- Listener (accepts connections and spawns HttpSession) ----------
class Listener : public std::enable_shared_from_this<Listener> {
 public:
  Listener(asio::io_context& ioc,
           tcp::endpoint endpoint,
           std::shared_ptr<SharedState> state,
           std::string allowed_origins)
      : acceptor_(ioc),
        state_(std::move(state)),
        allowed_origins_(std::move(allowed_origins)) {
    beast::error_code ec;
    acceptor_.open(endpoint.protocol(), ec);
    acceptor_.set_option(asio::socket_base::reuse_address(true), ec);
    acceptor_.bind(endpoint, ec);
    acceptor_.listen(asio::socket_base::max_listen_connections, ec);
  }

  void run() { do_accept(); }

 private:
  void do_accept() {
    acceptor_.async_accept(
      beast::bind_front_handler(
        &Listener::on_accept,
        shared_from_this()
      )
    );
  }

  void on_accept(beast::error_code ec, tcp::socket socket) {
    if (!ec) {
      std::make_shared<HttpSession>(
        std::move(socket),
        state_,
        allowed_origins_
      )->run();
    }
    do_accept();
  }

  tcp::acceptor acceptor_;
  std::shared_ptr<SharedState> state_;
  std::string allowed_origins_;
};

// ---------- main ----------
int main() {
  const std::string service = "ws-gateway";
  const int port = env_int("WS_PORT", 7005);
  const std::string allowed_origins = env_str("ALLOWED_ORIGINS", "http://localhost:3000");

  try {
    asio::io_context ioc{1};

    auto state = std::make_shared<SharedState>();

    std::make_shared<Listener>(
      ioc,
      tcp::endpoint{tcp::v4(), static_cast<unsigned short>(port)},
      state,
      allowed_origins
    )->run();

    std::cout << "{\"service\":\"" << service
              << "\",\"level\":\"INFO\",\"msg\":\"listening\",\"port\":" << port
              << "}\n";

    ioc.run();
  } catch (const std::exception& e) {
    std::cerr << "{\"service\":\"" << service
              << "\",\"level\":\"ERROR\",\"msg\":\"fatal\",\"err\":\""
              << e.what() << "\"}\n";
    return 1;
  }
}
