#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <dirent.h> // dir stuff go brr
#include <pwd.h>

// ts tuff
class ProcessHunter {
private:
    // microsoft better hire me asap this codebase is better than the NT kernel
    struct ProcessInfo {
        int pid;
        std::string name;   // proc name gng
        std::string status; // keep the procs in line they be actin out
        std::string user;   // find the culprit
    };

    std::vector<ProcessInfo> processes; // store ts

public:

    ProcessHunter() {
        hunt_processes();
    }

    // doom guy of process hunting
    void hunt_processes() {
        DIR* proc_dir = opendir("/proc");
        if (!proc_dir) {
            std::cerr << "BRUH CANT OPEN /PROC. SOMETHING WENT WRONG!!!" << std::endl;
            return;
        }

        struct dirent* entry;
        while ((entry = readdir(proc_dir)) != nullptr) {
            // only fetch numeric dirs others useless
            if (entry->d_type == DT_DIR && std::string(entry->d_name).find_first_not_of("0123456789") == std::string::npos) {
                int pid = std::stoi(entry->d_name);
                ProcessInfo proc = extract_process_details(pid);
                if (!proc.name.empty()) {
                    processes.push_back(proc);
                }
            }
        }
        closedir(proc_dir);
    }

    // probe every process
    ProcessInfo extract_process_details(int pid) {
        ProcessInfo proc;
        proc.pid = pid;

        // still probing processes
        std::ifstream comm_file("/proc/" + std::to_string(pid) + "/comm");
        if (comm_file.is_open()) {
            std::getline(comm_file, proc.name);
        }

        // this process probing stuff tuff dawg
        std::ifstream status_file("/proc/" + std::to_string(pid) + "/status");
        if (status_file.is_open()) {
            std::string line;
            while (std::getline(status_file, line)) {
                if (line.substr(0, 6) == "State:") {
                    proc.status = line.substr(7);
                    break;
                }
            }
        }

        // again get the culprit
        struct passwd* pw = getpwuid(get_process_owner(pid));
        proc.user = pw ? pw->pw_name : "UNKNOWN USER";

        return proc;
    }

    // find proc uid
    uid_t get_process_owner(int pid) {
        std::ifstream stat_file("/proc/" + std::to_string(pid) + "/status");
        std::string line;
        while (std::getline(stat_file, line)) {
            if (line.substr(0, 4) == "Uid:") {
                std::istringstream iss(line);
                std::string token;
                std::vector<std::string> tokens;
                while (iss >> token) {
                    tokens.push_back(token);
                }
                return std::stoi(tokens[1]); //first uid is distraction second one the real deal
            }
        }
        return -1; // FAILED :(
    }

    // print the procs bcs why not
    void print_processes() {
        std::cout << "==== sys proc hunter ====" << std::endl;
        std::cout << "PID\tNAME\t\tSTATUS\t\tUSER" << std::endl;

        for (const auto& proc : processes) {
            std::cout << proc.pid << "\t"
                      << proc.name.substr(0, 15) << "\t\t"
                      << proc.status.substr(0, 10) << "\t\t"
                      << proc.user << std::endl;
        }
    }
};

int main() {
    ProcessHunter hunter;
    hunter.print_processes();

    // compare with ps here itself i will die if i have to write docs for this seperately
    std::cout << "\n🔍 COMPARE WITH 'ps -A' COMMAND:" << std::endl;
    std::cout << "THIS CODE IS SIMILAR BUT NOT EXACTLY THE SAME AS 'ps -A'\n";
    std::cout << "DIFFERENCES:\n";
    std::cout << "1. WE USE /PROC FILESYSTEM (LINUX MAGIC)\n";
    std::cout << "2. ps COMMAND HAS MORE DETAILED OUTPUT\n";
    std::cout << "3. WE ONLY GET BASIC INFO, ps GETS EVERYTHING!\n";

    return 0; // im done dawg
}