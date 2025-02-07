#include <iostream>
#include <fstream>
#include <string>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <filesystem>
#include <cstring>
#include <vector>

namespace fs = std::filesystem;

class SystemInfo {
private:
    struct utsname uname_data;
    struct sysinfo sys_data;
    
    // Helper function to read proc files
    std::string read_proc_file(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return "Unable to read";
        std::string line;
        std::getline(file, line);
        return line;
    }
    
    // Convert bytes to human readable format
    std::string format_bytes(unsigned long bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int i = 0;
        double size = bytes;
        while (size >= 1024 && i < 4) {
            size /= 1024;
            i++;
        }
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[i]);
        return std::string(buffer);
    }

public:
    SystemInfo() {
        uname(&uname_data);
        sysinfo(&sys_data);
    }
    
    void print_system_info() {
        std::cout << "\n=== System Information ===\n";
        std::cout << "Hostname: " << uname_data.nodename << "\n";
        std::cout << "OS: " << uname_data.sysname << "\n";
        std::cout << "Kernel Version: " << uname_data.release << "\n";
        std::cout << "Architecture: " << uname_data.machine << "\n";
        
        // CPU Info
        std::cout << "\n=== CPU Information ===\n";
        std::cout << "Number of CPU cores: " << std::thread::hardware_concurrency() << "\n";
        std::cout << "CPU Model: " << read_proc_file("/proc/cpuinfo/model name") << "\n";
        
        // Memory Info
        std::cout << "\n=== Memory Information ===\n";
        std::cout << "Total RAM: " << format_bytes(sys_data.totalram) << "\n";
        std::cout << "Free RAM: " << format_bytes(sys_data.freeram) << "\n";
        std::cout << "Used RAM: " << format_bytes(sys_data.totalram - sys_data.freeram) << "\n";
        std::cout << "Total Swap: " << format_bytes(sys_data.totalswap) << "\n";
        std::cout << "Free Swap: " << format_bytes(sys_data.freeswap) << "\n";
        
        // Disk Info
        std::cout << "\n=== Disk Information ===\n";
        struct statvfs disk_info;
        if (statvfs("/", &disk_info) == 0) {
            unsigned long total = disk_info.f_blocks * disk_info.f_frsize;
            unsigned long available = disk_info.f_bfree * disk_info.f_frsize;
            unsigned long used = total - available;
            std::cout << "Total Disk Space: " << format_bytes(total) << "\n";
            std::cout << "Used Disk Space: " << format_bytes(used) << "\n";
            std::cout << "Available Disk Space: " << format_bytes(available) << "\n";
        }
        
        // User Info
        std::cout << "\n=== User Information ===\n";
        uid_t uid = geteuid();
        struct passwd *pw = getpwuid(uid);
        if (pw) {
            std::cout << "Current User: " << pw->pw_name << "\n";
            std::cout << "User ID: " << uid << "\n";
            std::cout << "Home Directory: " << pw->pw_dir << "\n";
        }
        
        // Process Info
        std::cout << "\n=== Process Information ===\n";
        std::cout << "Current Process ID: " << getpid() << "\n";
        std::cout << "Parent Process ID: " << getppid() << "\n";
        
        // Load Average
        std::cout << "\n=== System Load ===\n";
        std::cout << "Load Averages (1, 5, 15 min): "
                  << (float)sys_data.loads[0]/65536.0 << ", "
                  << (float)sys_data.loads[1]/65536.0 << ", "
                  << (float)sys_data.loads[2]/65536.0 << "\n";
        
        // System Uptime
        std::cout << "\n=== System Uptime ===\n";
        long uptime = sys_data.uptime;
        int days = uptime / (24*3600);
        int hours = (uptime % (24*3600)) / 3600;
        int mins = (uptime % 3600) / 60;
        int secs = uptime % 60;
        std::cout << "System Uptime: " << days << " days, " 
                  << hours << " hours, " << mins << " minutes, "
                  << secs << " seconds\n";
    }
    
    // Get CPU temperature (if available)
    void print_cpu_temp() {
        std::cout << "\n=== CPU Temperature ===\n";
        const char* temp_paths[] = {
            "/sys/class/thermal/thermal_zone0/temp",
            "/sys/class/hwmon/hwmon0/temp1_input"
        };
        
        for (const auto& path : temp_paths) {
            std::ifstream temp_file(path);
            if (temp_file.is_open()) {
                int temp;
                temp_file >> temp;
                std::cout << "CPU Temperature: " << temp/1000.0 << "°C\n";
                return;
            }
        }
        std::cout << "CPU temperature information not available\n";
    }
};

int main() {
    try {
        SystemInfo sysinfo;
        sysinfo.print_system_info();
        sysinfo.print_cpu_temp();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
