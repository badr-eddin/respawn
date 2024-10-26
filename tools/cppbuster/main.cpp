#include <iostream>
#include <asio.hpp>
#include <regex>
#include <cstring>
#include <fstream>

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
    using lists = std::vector<std::string>;

    namespace dir_enum {
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
    
        void enumerate(std::string &server, utils::lists &paths) {
            for (std::string &dir : paths) {
                int sts_code = exists(server, dir);
                logging::log_request_code(sts_code, server + "/" + dir);
            }
        }

    }

    lists read_file_and_split(std::string file_path, std::string sep = "\n") {
        std::ifstream file(file_path);
        lists tokens = {};

        if (!file.is_open()) {
            logging::print(file_path + " word list file couldn't be opened!");
            return tokens;
        }

        std::string file_content((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());

        file.close();

        size_t start = 0;
        size_t end = file_content.find(sep, start);

        while (end != std::string::npos) {
            tokens.push_back(file_content.substr(start, end - start));
            start = end + sep.length();
            end = file_content.find(sep, start);
        }

        tokens.push_back(file_content.substr(start));

        return tokens;   
    }

}

int main(int argc, char* argv[]) {
    
    if (argc < 2) {
        logging::print_help();
        std::cout <<  argc << "mm\n";
        return 0;
    }

    std::string arg = argv[1];

    if (arg == "--help" || arg == "-h") {
        logging::print_help();
        return 0;

    } else {
        std::string url = "";
        std::string word_lists_path = "";
        std::string separator = "\n";

        for (int i = 0; i < argc; i++) {
            std::string current_arg = argv[i];
            bool next_is_valid = i < argc - 1 && strncmp(argv[i+1], "-", 1) != 0;

            if (next_is_valid && (current_arg == "--url" || current_arg == "-u")) {
                url = argv[i+1];
            } else if (next_is_valid && (current_arg == "--wordlist" || current_arg == "-w")) {
                word_lists_path = argv[i+1];
            } else if (next_is_valid && (current_arg == "--wordlist-sep" || current_arg == "-ws")) {
                separator = argv[i+1];
            }
        }

        if (url.empty() || word_lists_path.empty()) {
            logging::print_help();

            return 0;
        }

        if (arg == "dir") {
            logging::print("directory enumeration ...");
            utils::lists dirs = utils::read_file_and_split(word_lists_path, separator);
            utils::dir_enum::enumerate(url, dirs);
        }

    }

    return 0;
}
