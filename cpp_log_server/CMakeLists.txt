cmake_minimum_required(VERSION 3.10)
project(SecureLogServer)

set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

# Include directories
include_directories(${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})

# Define the executable
add_executable(server main.cpp)

# Link libraries
target_link_libraries(server
    OpenSSL::SSL
    OpenSSL::Crypto
    ${Boost_LIBRARIES}
    Threads::Threads
)
