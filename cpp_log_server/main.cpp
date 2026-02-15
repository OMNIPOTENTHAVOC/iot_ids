#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "LogStore.h"  // <--- NEW: Include the Log Manager

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

// Global LogStore instance (Simple for this demo)
LogStore log_manager;  // <--- NEW: Create the manager

// Helper to read exactly 'length' bytes
std::string read_framed_message(ssl::stream<tcp::socket>& socket) {
    uint32_t net_len;
    boost::asio::read(socket, boost::asio::buffer(&net_len, 4));
    uint32_t body_len = ntohl(net_len);

    std::vector<char> buf(body_len);
    boost::asio::read(socket, boost::asio::buffer(buf));

    return std::string(buf.begin(), buf.end());
}

void session(ssl::stream<tcp::socket> socket) {
    try {
        socket.handshake(ssl::stream_base::server);
        std::cout << "[Server] Handshake successful!" << std::endl;

        while (true) {
            try {
                std::string msg = read_framed_message(socket);
                std::cout << "[Received] " << msg << std::endl;

                // <--- NEW: Send data to LogStore
                log_manager.add_log(msg);

            } catch (std::exception& e) {
                break;
            }
        }
    } catch (std::exception& e) {
        std::cerr << "[Error] Session: " << e.what() << std::endl;
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        ssl::context ctx(ssl::context::tls_server);

        // Load Certs (Ensure paths are correct!)
        ctx.use_certificate_chain_file("../../certs/server.crt");
        ctx.use_private_key_file("../../certs/server.key", ssl::context::pem);
        ctx.load_verify_file("../../certs/ca.crt");
        ctx.set_verify_mode(ssl::verify_peer | ssl::verify_fail_if_no_peer_cert);

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8888));
        std::cout << "[Server] Listening on port 8888 (mTLS enabled)..." << std::endl;

        while (true) {
            // Create a socket and pass ownership to the thread
            // Note: In newer Boost versions, we move the socket.
            // If this line fails, revert to your previous connection logic.
            ssl::stream<tcp::socket> ssl_sock(io_context, ctx);
            acceptor.accept(ssl_sock.next_layer());

            std::thread(session, std::move(ssl_sock)).detach();
        }
    } catch (std::exception& e) {
        std::cerr << "[Server Error] " << e.what() << std::endl;
    }
    return 0;
}
