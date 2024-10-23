#include <iostream>
#include <asio.hpp>
#include <regex>

// make sure to use standalone ASIO lib (doesnt depend on boost framework)
#define ASIO_STANDALONE 1

// ---------------- Logging
namespace logging {
    void print(const char* message) {
        std::cout << message << std::endl;
    }
    void print(const std::string message) {
        print(message.c_str());
    }

    void print_help() {
        print("C++Buster Part of Respawn Project");
        print("Usage:");
        print("\tcppbuster [command] [args]");
        print("Commands:");
        print("\tdir");
        print("\tdns");
        print("\tfuzz");
        print("Options:");
        print("\t-u, --url            URL | IP Address.");
        print("\t-w, --wordlist       Path to word list (for fuzzing).");
        print("\t-ws, --wordlist-sep  Words in wordlist separator, defaul to '\\n'.");
        print("\t-p, --proxy          HTTP proxy");
    }

    void log_request_code(int code, std::string url) {
        if (code == 200)
            print("[+] Found: " + url);
        else if (code == 404)
            print("[!] Not Found: " + url);
        else if (code == 400)
            print("[*] Redirected: " + url);
    }
}


// ---------------- Utils
namespace utils
{
    using asio::ip::tcp;

    namespace dir_enum {
        using paths_t = std::vector<std::string>;

        int exists(std::string &server, std::string &path) {
            // error handling
            asio::error_code err_code;

            // create a context
            asio::io_context context;

            // resolve the url
            tcp::resolver resolver(context);
            tcp::resolver::results_type endpoint = resolver.resolve(server, "80");

            // initiating a socket
            tcp::socket socket(context);
            asio::connect(socket, endpoint, err_code);

            if (err_code) {
                return -1;
            }

            // making raw-request
            std::string request = 
                "GET " + path + "HTTP/1.1\r\n" +
                "Host: " + server + "\r\n" + 
                "Connection: close\r\n\r\n";

            // send the request
            asio::write(socket, asio::buffer(request));

            // whait for the response
            socket.wait(socket.wait_read);

            // read data
            size_t availableBytes = socket.available();

            std::vector<char> buffer(availableBytes);

            socket.read_some(asio::buffer(buffer.data(), buffer.size()), err_code);

            if (availableBytes <= 0 || err_code) {
                return -1;
            }

            // convert to string
            std::string response(buffer.begin(), buffer.end());

            // getting status code
            std::regex pattern(R"(HTTP\/\d\.\d\s+(\d{3}))");

            std::smatch match;

            if (std::regex_search(response, match, pattern) && match.size() > 1) {
                return std::stoi(match[1].str());
            }

            return -1;

        }
    
        void enumerate(std::string &server, paths_t &paths) {
            for (std::string &dir : paths) {
                int sts_code = exists(server, dir);
                logging::log_request_code(sts_code, server + "/" + dir);
            }
        }

    }
}

int main() {
    std::string server = "scanme.nmap.org";
    utils::dir_enum::paths_t paths = {"admin", "login", "images", "user"};

    utils::dir_enum::enumerate(server, paths);
    return 0;
}
