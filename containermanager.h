#ifndef CONTAINERMANAGER_H
#define CONTAINERMANAGER_H

#include <string>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include "helpresponse.h"
#include <fstream>
#include <limits.h> // For HOST_NAME_MAX


class ContainerManager
{
public:
    ContainerManager();
    ~ContainerManager();

    int runContainer(int argc, char *argv[]);

private:
    static int runApp(const char *app);
    static int setupContainer(void *args);
    static void setupCGroup(pid_t container_pid, int max_processes, const std::string & hostname = "");
    static bool MountCGroup( const std::string &cgroup_path);
    static bool createDirectories(const std::string &path, mode_t mode = 0755);

    static std::string _hostname;
    std::vector<std::string> containers;

    static const int STACK_SIZE = 1024 * 1024; // Stack size for child process
    struct CloneArgs
    {
        int argc;
        char **argv;
    };

    static std::string getHostName()
    {
        const std::string hostname_file = "/etc/hostname";
        std::ifstream file(hostname_file);
        if (file.is_open())
        {
            std::string hostname;
            std::getline(file, hostname);
            file.close();

            if (!hostname.empty())
            {
                std::string msg = std::string("Hostname from /etc/hostname: ") + hostname;
                HelpResponse::info(msg);
                return hostname;
            }
            else
            {
                HelpResponse::errorHandler("Hostname file is empty");
            }
        }
        else
        {
            HelpResponse::errorHandler("Failed to open /etc/hostname");
        }

        return ""; // Return an empty string if reading fails
    }
};

#endif // CONTAINERMANAGER_H