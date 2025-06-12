#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <filesystem>

struct Session {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::string ip_address;
    long long bytes_sent = 0;
    long long bytes_received = 0;
};

std::chrono::system_clock::time_point parseTimestamp(const std::string &ts) {
    std::tm tm = {};
    std::istringstream ss(ts);
    ss >> std::get_time(&tm, "%Y/%m/%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string formatTimestamp(const std::chrono::system_clock::time_point &tp) {
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&t);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

int main() {
    // Путь к логу AnyDesk
    std::string log_path = "C:\\ProgramData\\AnyDesk\\ad_svc.trace";
    if (!std::filesystem::exists(log_path)) {
        std::cerr << "Log file not found: " << log_path << std::endl;
        return 1;
    }

    std::ifstream infile(log_path);
    if (!infile) {
        std::cerr << "Failed to open file: " << log_path << std::endl;
        return 1;
    }

    std::regex timestamp_re(R"((\d{4}/\d{2}/\d{2} \d{2}:\d{2}:\d{2}))");
    std::regex ip_re(R"(RemoteAddress=(\d{1,3}(?:\.\d{1,3}){3}))");
    std::regex bytes_re(R"(TxBytes=(\d+), RxBytes=(\d+))");

    std::vector<Session> sessions;
    Session current;
    bool in_session = false;
    std::string line;

    while (std::getline(infile, line)) {
        std::smatch m;
        // для поиска «session started/ended» в нижнем регистре
        std::string lower = line;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower.find("session started") != std::string::npos) {
            in_session = true;
            current = Session();
            if (std::regex_search(line, m, timestamp_re))
                current.start_time = parseTimestamp(m[1]);
        }
        else if (lower.find("session ended") != std::string::npos) {
            if (in_session) {
                if (std::regex_search(line, m, timestamp_re))
                    current.end_time = parseTimestamp(m[1]);
                sessions.push_back(current);
                in_session = false;
            }
        }
        else if (in_session) {
            if (std::regex_search(line, m, ip_re))
                current.ip_address = m[1];
            else if (std::regex_search(line, m, bytes_re)) {
                current.bytes_sent     = std::stoll(m[1]);
                current.bytes_received = std::stoll(m[2]);
            }
        }
    }

    // Сортировка по стартовому времени
    std::sort(sessions.begin(), sessions.end(),
        [](auto &a, auto &b){ return a.start_time < b.start_time; });

    // Запись в CSV
    std::ofstream outfile("anydesk_sessions.csv");
    outfile << "start_time,end_time,duration,ip_address,bytes_sent,bytes_received\n";
    for (auto &s : sessions) {
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(s.end_time - s.start_time).count();
        int h = sec/3600, m = (sec%3600)/60, sx = sec%60;
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d:%02d:%02d", h, m, sx);

        outfile
          << formatTimestamp(s.start_time) << ","
          << formatTimestamp(s.end_time)   << ","
          << buf                           << ","
          << (s.ip_address.empty() ? "n/a" : s.ip_address) << ","
          << s.bytes_sent   << ","
          << s.bytes_received
          << "\n";
    }

    std::cout << "Done! Data saved to anydesk_sessions.csv\n";
    return 0;
}