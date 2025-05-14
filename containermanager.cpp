#include "containermanager.h"

#include "argvparser.h"
#include <sched.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <limits.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

ContainerManager::ContainerManager() {}

ContainerManager::~ContainerManager() {}

using namespace std;

int ContainerManager::runApp(const char *app)
{
    char *argv[] = {const_cast<char *>(app), nullptr};
    char *envp[] = {const_cast<char *>("PATH=/bin:/bin/bash"), nullptr};

    int pid = fork();
    if (pid == 0)
    { // Child process
        if (execve(app, argv, envp) == -1)
        {
            HelpResponse::abortHandler(app + string(" execve failed"));
        }
    }
    else if (pid > 0)
    { // Parent process
        if (waitpid(pid, nullptr, 0) == -1)
        {
            HelpResponse::abortHandler("waitpid: Failed to wait for child process");
        }
    }
    else
    {
        HelpResponse::abortHandler("Failed to fork process");
        return -1;
    }

    return pid;
}

int ContainerManager::setupContainer(void *args)
{
    int pid = getpid();
    if (args == nullptr)
    {
        HelpResponse::errorHandler("Invalid arguments passed to container process");
        return -1;
    }

    CloneArgs *cl_args = static_cast<CloneArgs *>(args);
    ArgvParser parser(cl_args->argc, cl_args->argv);

    string image = parser.getImage();
    string hostname = image.substr(image.find_last_of('/')+1);


    if (parser.getOption() == "--limit-pid")
    {
        int limit = parser.getLimit();
        if (limit < 0)
        {
            HelpResponse::helpAndExit("Invalid limit value for --limit-pid option");
        }
        setupCGroup(pid, limit, hostname);
    }

    if (chroot(parser.getImage().c_str()) == -1)
    {
        HelpResponse::abortHandler("Failed to change root directory " + parser.getImage());
    }
    else
    {
        HelpResponse::info("Changed root directory to " + parser.getImage());
    }

    chdir("/");
    if (mount("proc", "/proc", "proc", 0, "") == -1)
    {
        HelpResponse::abortHandler("Failed to mount /proc filesystem");
    }


    sethostname( hostname.c_str(), hostname.length());

    HelpResponse::info("ContainerManager::setupContainer hostname: " + hostname);

    return runApp(parser.getApplication().c_str());
}

void ContainerManager::setupCGroup(pid_t container_pid, int max_processes, const string & hostname)
{
    const string cgroup_path_root = "/sys/fs/cgroup";
    string cgroup_path = cgroup_path_root + "/" + hostname;

    if (!MountCGroup(cgroup_path))
    {
        HelpResponse::abortHandler("Could not mount: " + cgroup_path_root, errno);
    }

    ofstream subtree_control_file(cgroup_path_root + "/cgroup.subtree_control");
    if (!subtree_control_file)
    {
        HelpResponse::abortHandler("Ensure that cgroup v2 is enabled and the pids controller is available.", errno);
    }
    subtree_control_file << "+pids";
    subtree_control_file.close();

    ofstream pids_max_file(cgroup_path + "/pids.max");
    if (!pids_max_file)
    {
        HelpResponse::abortHandler("setupCGroup: Failed to open pids.max", errno);
    }

    pids_max_file << max_processes;
    pids_max_file.close();

    HelpResponse::info(string("ContainerManager::setupCGroup Set pids.max to  ") + to_string(max_processes) + string(" in ")+ cgroup_path);
    HelpResponse::info(string("ContainerManager::setupCGroup Writing PID  ") + to_string(container_pid) + string(" to cgroup.procs"));

    ofstream cgroup_procs_file(cgroup_path + "/cgroup.procs");
    if (!cgroup_procs_file)
    {
        HelpResponse::abortHandler("Ensure that cgroup v2 is enabled and the pids controller is available.", errno);
    }

    HelpResponse::info("ContainerManager::setupCGroup Adding process to cgroup for container pid " + to_string(container_pid));
    cgroup_procs_file << container_pid;
    cgroup_procs_file.close();
}

int ContainerManager::runContainer(int argc, char *argv[])
{
    CloneArgs args = {argc, argv};
    ArgvParser parser(argc, argv);

    string image = parser.getImage();
    string hostname = image.substr(image.find_last_of('/')+1);
    string original_hostname = getHostName();

    HelpResponse::info( "ContainerManager::runContainer Setup");
    HelpResponse::info( "    Image: " + image);
    HelpResponse::info( "    Hostname: " + hostname);
    HelpResponse::info( "    Original Hostname: " + original_hostname);

    char *stack = new char[STACK_SIZE];
    if (stack == nullptr)
    {
        HelpResponse::helpAndExit("Failed to allocate stack for child process");
    }

    stack = stack + STACK_SIZE;
    pid_t pid = clone(&ContainerManager::setupContainer, stack,
                      CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, &args);
    if (pid < 0)
    {
        HelpResponse::abortHandler("ContainerManager::runContainer Failed to create container process; errno: ", errno);
    }

    string msg = "ContainerManager::runContainer Container created for PID: " + to_string(pid);
    HelpResponse::info(msg.c_str());

    // Add the containerized process to the cgroup
    if (parser.getOption() == "--limit-pid")
    {
        int limit = parser.getLimit();
        if (limit < 0)
        {
            HelpResponse::helpAndExit("ContainerManager::runContainer Invalid limit value for --limit-pid option");
        }

        HelpResponse::info( "ContainerManager::runContainer Calling ContainerManager::setupCGroup: with hostname: " + hostname);
        ContainerManager::setupCGroup(pid, limit, hostname); // Pass the PID returned by clone
    }

    if (waitpid(pid, nullptr, 0) == -1)
    {
        HelpResponse::helpAndExit("ContainerManager::runContainer Failed to wait for container process");
    }

    string mountproc = parser.getImage() + string("/proc");
    HelpResponse::info("ContainerManager::runContainer Unmounting /proc filesystem: " + mountproc);
    umount2(mountproc.c_str(), MNT_FORCE); // Unmount /proc filesystem
    if (errno != 0)
    {
        HelpResponse::errorHandler("ContainerManager::runContainer Failed to unmount /proc filesystem...errno [" + to_string(errno) + "]: ");
    }

    sethostname(original_hostname.c_str(), original_hostname.length());

    delete[] (stack - STACK_SIZE);
    return pid;
}

bool ContainerManager::MountCGroup(const string &cgroup_path)
{
    // return true; // For testing purposes, always return true

    HelpResponse::info("ContainerManager::MountCGroup: mkdir path: " + cgroup_path);


    if (mkdir(cgroup_path.c_str(), 0755) == -1 && errno != EEXIST)
    {
        HelpResponse::abortHandler("ContainerManager::MountCGroup: Failed to create cgroup mount point: " + to_string(errno));
        return false;
    }


    return true;
}

bool ContainerManager::createDirectories(const string &path, mode_t mode)
{
    size_t pos = 0;
    string currentPath;

    // Iterate through each component of the path
    while ((pos = path.find('/', pos)) != string::npos)
    {
        currentPath = path.substr(0, pos++);
        if (currentPath.empty())
            continue; // Skip root "/"

        // Check if the directory exists
        struct stat st;
        if (stat(currentPath.c_str(), &st) == -1)
        {
            // Directory does not exist, attempt to create it
            if (mkdir(currentPath.c_str(), mode) == -1 && errno != EEXIST)
            {
                cerr << "Failed to create directory: " << currentPath
                          << ", errno: " << strerror(errno) << endl;
                return false;
            }
        }
    }

    // Create the final directory
    if (mkdir(path.c_str(), mode) == -1 && errno != EEXIST)
    {
        HelpResponse::errorHandler("ContainerManager::createDirectories: Failed to create directory: " 
                                    + path 
                                    + "error: " 
                                    + to_string(errno));
        return false;
    }

    return true;
}