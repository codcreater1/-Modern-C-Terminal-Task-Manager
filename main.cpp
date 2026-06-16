#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <limits>

// Terminal Renk Kodları Burada
const std::string RESET  = "\033[0m";
const std::string RED    = "\033[31m";
const std::string GREEN  = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string CYAN   = "\033[36m";
const std::string BOLD   = "\033[1m";

enum class Priority { Low = 0, Medium = 1, High = 2 };

// Yardımcı Fonksiyonlar
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::string toLower(const std::string& str) {
    std::string data = str;
    std::transform(data.begin(), data.end(), data.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return data;
}

struct Task {
    int id;
    std::string description;
    Priority priority;
    bool completed;

    Task(int i, std::string desc, Priority p = Priority::Medium)
        : id(i), description(std::move(desc)), priority(p), completed(false) {}
};

class TaskManager {
private:
    std::vector<Task> tasks;
    int nextId = 1;
    const std::string filename = "tasks.txt";

    void load() {
        std::ifstream file(filename);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            int id, prioInt;
            bool completed;
            
            if (!(iss >> id >> prioInt >> completed)) continue;

            std::string description;
            std::getline(iss >> std::ws, description); // Boşlukları temizleyerek oku
            
            tasks.emplace_back(id, trim(description), static_cast<Priority>(prioInt));
            tasks.back().completed = completed;
            
            if (id >= nextId) nextId = id + 1;
        }
    }

    void save() const {
        std::ofstream file(filename);
        for (const auto& t : tasks) {
            file << t.id << " " << static_cast<int>(t.priority) << " "
                 << t.completed << " " << t.description << "\n";
        }
    }

    std::string priorityLabel(Priority p) const {
        switch (p) {
            case Priority::Low:    return GREEN + "Low" + RESET;
            case Priority::Medium: return YELLOW + "Medium" + RESET;
            case Priority::High:   return RED + "High" + RESET;
            default:               return "Unknown";
        }
    }

public:
    TaskManager() { load(); }
    ~TaskManager() { save(); }

    void add(const std::string& desc, Priority prio = Priority::Medium) {
        tasks.emplace_back(nextId++, trim(desc), prio);
        std::cout << GREEN << "✔ Task added successfully (ID: " << nextId - 1 << ")" << RESET << std::endl;
    }

    void list() {
        if (tasks.empty()) {
            std::cout << YELLOW << "No tasks found." << RESET << std::endl;
            return;
        }

        // Önceliğe göre sırala (High -> Low)
        std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
            if (a.completed != b.completed) return a.completed < b.completed; // Tamamlanmayanlar üstte
            return static_cast<int>(a.priority) > static_cast<int>(b.priority);
        });

        std::cout << BOLD << CYAN << "\n--- CURRENT TASKS (Sorted by Priority) ---" << RESET << std::endl;
        for (const auto& t : tasks) {
            std::string status = t.completed ? GREEN + "[✔]" + RESET : RED + "[ ]" + RESET;
            std::cout << status << " ID: " << t.id << " | " << t.description 
                      << " [" << priorityLabel(t.priority) << "]" << std::endl;
        }
    }

    void complete(int id) {
        auto it = std::find_if(tasks.begin(), tasks.end(), [id](const Task& t) { return t.id == id; });
        if (it != tasks.end()) {
            it->completed = true;
            std::cout << GREEN << "✔ Task " << id << " marked as done." << RESET << std::endl;
        } else {
            std::cout << RED << "✘ Error: Task ID " << id << " not found." << RESET << std::endl;
        }
    }

    void remove(int id) {
        auto it = std::remove_if(tasks.begin(), tasks.end(), [id](const Task& t) { return t.id == id; });
        if (it != tasks.end()) {
            tasks.erase(it, tasks.end());
            std::cout << GREEN << "✔ Task " << id << " removed." << RESET << std::endl;
        } else {
            std::cout << RED << "✘ Error: Task ID " << id << " not found." << RESET << std::endl;
        }
    }

    void search(const std::string& query) const {
        std::string lowerQuery = toLower(trim(query));
        bool found = false;
        for (const auto& t : tasks) {
            if (toLower(t.description).find(lowerQuery) != std::string::npos) {
                std::string status = t.completed ? "[✔]" : "[ ]";
                std::cout << status << " ID: " << t.id << " | " << t.description << std::endl;
                found = true;
            }
        }
        if (!found) std::cout << YELLOW << "No matches for: " << query << RESET << std::endl;
    }
};

void printHelp() {
    std::cout << BOLD << "\nCOMMANDS:" << RESET << std::endl;
    std::cout << "  add <desc> [high|medium|low]  |  list  |  done <id>  |  delete <id>  |  search <text>  |  exit" << std::endl;
}

int main() {
    TaskManager manager;
    std::string line;

    std::cout << BOLD << CYAN << "Modern C++ Terminal Task Manager Loaded." << RESET << std::endl;
    printHelp();

    while (true) {
        std::cout << BOLD << "\n> " << RESET;
        if (!std::getline(std::cin, line)) break;
        line = trim(line);
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        cmd = toLower(cmd);

        if (cmd == "exit") break;
        if (cmd == "help") { printHelp(); continue; }
        if (cmd == "list") { manager.list(); continue; }

        if (cmd == "add") {
            std::string remaining, desc, prioStr;
            std::getline(iss >> std::ws, remaining);
            
            size_t lastSpace = remaining.find_last_of(' ');
            Priority p = Priority::Medium;

            if (lastSpace != std::string::npos) {
                prioStr = toLower(remaining.substr(lastSpace + 1));
                if (prioStr == "high") { p = Priority::High; desc = remaining.substr(0, lastSpace); }
                else if (prioStr == "low") { p = Priority::Low; desc = remaining.substr(0, lastSpace); }
                else if (prioStr == "medium") { p = Priority::Medium; desc = remaining.substr(0, lastSpace); }
                else { desc = remaining; }
            } else {
                desc = remaining;
            }
            manager.add(desc, p);
        } 
        else if (cmd == "done" || cmd == "delete") {
            int id;
            if (iss >> id) {
                if (cmd == "done") manager.complete(id);
                else manager.remove(id);
            } else {
                std::cout << RED << "Invalid ID." << RESET << std::endl;
            }
        }
        else if (cmd == "search") {
            std::string q;
            std::getline(iss >> std::ws, q);
            manager.search(q);
        }
        else {
            std::cout << YELLOW << "Unknown command. Type 'help'." << RESET << std::endl;
        }
    }
    return 0;
}
