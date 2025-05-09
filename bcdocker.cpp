//#define _GNU_SOURCE
#include <sched.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <cstdlib>
#include <sys/wait.h>
#include "helpresponse.h"
#include <string>
#include <limits.h> // For HOST_NAME_MAX
#include <stdlib.h>
#include <fstream> // For file operations
#include <sstream> // For string streams
#include "argvparser.h"
#include <cerrno> // For strcmp
#include "containermanager.h"
// Define the clone_args structure

#define STACK_SIZE (1024 * 1024) // Stack size for child process




int main(int argc, char *argv[]) {
    if (argc < 4) {
        HelpResponse::helpAndExit(argv[0]);
        return EXIT_FAILURE;
    }

    ArgvParser printArgs(argc, argv);
    printArgs.printArgs();

    ContainerManager manager;

    if (strcmp(argv[1], "run") == 0) {
        manager.runContainer(argc, argv);
    } else {
        HelpResponse::helpAndExit(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
