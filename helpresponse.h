#ifndef HELPRESPONSE_H
#define HELPRESPONSE_H   

#include <string>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

class HelpResponse {
public:
    static void helpAndExit(const std::string & message);

    static void info(const std::string & message);

    static void errorHandler(const std::string & message);

    static void abortHandler(const std::string & message);

    static void abortHandler(const std::string & errorMessage, int errorno);
};


#endif