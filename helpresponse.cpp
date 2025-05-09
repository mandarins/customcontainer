#include "helpresponse.h"
#include <string>
#include <iostream>
#include <unistd.h>

void HelpResponse::helpAndExit(const std::string & message)
{
    std::cout << "Usage: " << std::string(message) <<  "run options image application" << std::endl;
    std::cout << "  options:"  << std::endl
        << "        --help" << std::endl
        << "        --limit-pid #  where # is the number of processes to limit." << std::endl
        << "        --limit-mem #  where # is the size of memory in KB to limi." << std::endl;
    std::exit(EXIT_FAILURE);
}

void HelpResponse::info(const std::string & message)
{
    std::cout << "info: " << std::string(message) << std::endl;
}

void HelpResponse::errorHandler(const std::string & errorMessage)
{
    std::cout << "Error: " << errorMessage << std::endl;
}

void HelpResponse::abortHandler(const std::string & errorMessage)
{
    std::cout << "Abort: " << errorMessage << std::endl;
    std::exit(EXIT_FAILURE);
}

void HelpResponse::abortHandler(const std::string & errorMessage, int errorno)
{
    std::cout << "Abort: " << errorMessage << std::endl;
    std::cout << "Error number: " << errorno << std::endl;
    std::exit(EXIT_FAILURE);
}
    // Allocate memory for the stack    
