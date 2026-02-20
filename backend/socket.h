#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "anomaly_detector.h"
#include "data_parser.h"
#include <mutex>

int run_socket();