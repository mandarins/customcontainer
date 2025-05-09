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
// Define the clone_args structure
struct clone_args
{
    int argc;
    char **argv;
};

#define STACK_SIZE (1024 * 1024) // Stack size for child process
const char BC_HOSTNAME[] = "bcdocker";
int run_app( const char * app );

using namespace std;

// Function executed inside the container
int setup_container(void *args)
{
    int pid = getpid();
    if ((clone_args *)args == nullptr)
    {
        HelpResponse::errorHandler("Invalid arguments passed to container process");
        return -1;
    }


    // Cast args to clone_args pointer
    clone_args *cl_args = static_cast<clone_args *>(args);

    ArgvParser parser(cl_args->argc, cl_args->argv);
    cout << "****************" << endl;
    cout << "Command: " << parser.getCommand() << endl;
    cout << "Option: " << parser.getOption() << endl;   
    cout << "Limit: " << parser.getLimit() << endl;
    cout << "Image: " << parser.getImage() << endl;
    cout << "Application: " << parser.getApplication() << endl;
    cout << "****************" << endl;

    sethostname(BC_HOSTNAME, sizeof(BC_HOSTNAME)); // Set hostname
    if (sethostname(BC_HOSTNAME, sizeof(BC_HOSTNAME)) == -1)
    {
        HelpResponse::errorHandler("Failed to set hostname");
        return -1;
    }

    char hostname[HOST_NAME_MAX + 1];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        string msg = std::string("Hostname: ") + hostname;
        HelpResponse::info(msg.c_str());
    }
    else
    {
        HelpResponse::errorHandler("gethostname failed");
    }

    if (chroot(parser.getImage().c_str()) == -1)
    {
        HelpResponse::abortHandler("Failed to change root directory");
    }

    chdir("/");
    //std::cout << "Lsting files:\n";

    if ( mount("proc", "/proc", "proc", 0, "") == -1)
    {
        HelpResponse::abortHandler("Failed to mount /proc filesystem");
    }

    std::cout << "running cmd: << cmd << endl";

    pid = run_app(parser.getApplication().c_str()); // Run the application


    return pid;
}

int run_app( const char * app )   
{
    char *argv[] = {const_cast<char*>(app), nullptr};
    char *envp[] = {const_cast<char*>("PATH=/bin:/bin/bash"), nullptr};

    int pid = 0;
    if ( (pid = fork()) == 0 ) // Child process
    { 
        int res = 0;
        if ( (res =  execve(app, argv, envp) ) == -1 )
        {
            HelpResponse::errorHandler(app + string( " value : " ) + std::to_string(res));

        } 
    }
    else if ( pid > 0 )
    {
        if ( waitpid( pid, nullptr, 0 ) == -1 )
        {
            HelpResponse::abortHandler("waitpid: Failed to wait for child process");
        }
    } else
    {
        HelpResponse::abortHandler("Failed to fork process");
        return -1;      

    }

    return pid;
}

void setup_cgroup(pid_t container_pid, int max_processes)
{
    const std::string cgroup_path = "/sys/fs/cgroup/pids/bcdocker"; // Cgroup path

    // Create the cgroup directory
    if (mkdir(cgroup_path.c_str(), 0755) == -1 && errno != EEXIST)
    {
        perror("Failed to create cgroup directory");
        exit(EXIT_FAILURE);
    }

    // Set the maximum number of processes
    std::ofstream pids_max_file(cgroup_path + "/pids.max");
    if (!pids_max_file)
    {
        perror("Failed to open pids.max");
        exit(EXIT_FAILURE);
    }
    pids_max_file << max_processes;
    pids_max_file.close();

    // Add the container process to the cgroup
    std::ofstream cgroup_procs_file(cgroup_path + "/cgroup.procs");
    if (!cgroup_procs_file)
    {
        perror("Failed to open cgroup.procs");
        exit(EXIT_FAILURE);
    }
    cgroup_procs_file << container_pid;
    cgroup_procs_file.close();

    std::cout << "Cgroup configured with max processes: " << max_processes << std::endl;
}

int run_container(int argc, char *argv[])
{

    ArgvParser parser(argc, argv);

    struct clone_args args = {argc, argv};
    char *stack = new char[STACK_SIZE]; // Allocate stack for child process
    if (stack == nullptr)
    {
        const char *msg = "Failed to allocate stack for child process\n";
        HelpResponse::helpAndExit(msg);
    }

    // Create a new process using namespaces
    stack = stack + STACK_SIZE; // Move stack pointer to the top of the stack
    pid_t pid = clone(setup_container, stack,
                      CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, &args);
    if (pid < 0)
    {
        string msg = "Failed to create container process; errno: " + std::to_string(errno);
        msg += "\n";
        HelpResponse::helpAndExit(msg.c_str());
    }

    string msg = "Container PID: " + std::to_string(getpid());
    HelpResponse::info(msg.c_str());

    if ( parser.getOption() == "--limit-pid" )
    {
        int limit = parser.getLimit();
        if ( limit < 0 )
        {
            HelpResponse::helpAndExit("Invalid limit value for --limit-pid option");
        }
        setup_cgroup(pid, limit); // Setup cgroup with a limit of 2 processes
    }

    

    if (waitpid(pid, nullptr, 0) == -1)
    {
        HelpResponse::helpAndExit("Failed to wait for container process");
    } // Wait for container to exit

    return pid;
}

int main(int argc, char *argv[])
{
    int pid = 0;

   if (argc < 4)
    {
        HelpResponse::helpAndExit(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "run") == 0)
    {
        pid = run_container(argc, argv);
    }
    else
    {
        HelpResponse::helpAndExit(argv[0]);
        return EXIT_FAILURE;
    }

    waitpid(pid, nullptr, 0); // Wait for container to exit

    return EXIT_SUCCESS;
}
