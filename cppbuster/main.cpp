#include <iostream>
#include <asio.hpp>

// make sure to use standalone ASIO lib (doesnt depend on boost framework)
#define ASIO_STANDALONE 1

int main() {
    std::cout << "Hello World" << std::endl;
    return 0;
}
