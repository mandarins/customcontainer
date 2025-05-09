#include <string>
#include <stdexcept>
#include <iostream>

class ArgvParser {
private:
    std::string command;
    std::string option;
    int limit;
    std::string image;
    std::string application;

public:
    ArgvParser(int argc, char *argv[]) : limit(-1) { // Default limit is -1 (no limit)
        if (argc < 4) {
            throw std::invalid_argument("Insufficient arguments provided. Expected format: run [--option limit] <image> <application>");
        }

        // Parse command
        command = argv[1];
        if (command != "run") {
            throw std::invalid_argument("Invalid command. Expected 'run'.");
        }

        // Parse optional argument
        int currentIndex = 2;
        if (std::string(argv[currentIndex]).rfind("--", 0) == 0) { // Check if it starts with "--"
            option = argv[currentIndex];
            currentIndex++;

            if (currentIndex >= argc) {
                throw std::invalid_argument("Option provided without a limit value.");
            }

            try {
                limit = std::stoi(argv[currentIndex]);
                currentIndex++;
            } catch (const std::exception &e) {
                throw std::invalid_argument("Invalid limit value for option.");
            }
        }

        // Parse image
        if (currentIndex >= argc) {
            throw std::invalid_argument("Image argument is missing.");
        }
        image = argv[currentIndex];
        currentIndex++;

        // Parse application
        if (currentIndex >= argc) {
            throw std::invalid_argument("Application argument is missing.");
        }
        application = argv[currentIndex];
    }

    // Getters
    std::string getCommand() const { return command; }
    std::string getOption() const { return option; }
    int getLimit() const { return limit; }
    std::string getImage() const { return image; }
    std::string getApplication() const { return application; }

    // Debugging helper
    void printArgs() const {
        std::cout << "Command: " << command << std::endl;
        if (!option.empty()) {
            std::cout << "Option: " << option << " (Limit: " << limit << ")" << std::endl;
        }
        std::cout << "Image: " << image << std::endl;
        std::cout << "Application: " << application << std::endl;
    }
};