#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string CYAN = "\033[36m";
const std::string BOLD = "\033[1m";

enum class Priority { Low, Medium, High };

struct Task {
    int id;
    std::string description;
    Priority priority;
    bool completed;

    Task(int i, std::string desc, Priority p = Priority::Medium)
        : id(i), description(std::move(desc)), priority(p), completed(false) {
    }
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
            std::string description;

            iss >> id >> prioInt >> completed;
            std::getline(iss, description);
            if (description.size() > 1) description = description.substr(1); // remove leading space

            Priority prio = static_cast<Priority>(prioInt);
            tasks.emplace_back(id, description, prio);
            tasks.back().completed = completed;
        }

        if (!tasks.empty()) {
            nextId = std::max_element(tasks.begin(), tasks.end(),
                [](const Task& a, const Task& b) { return a.id < b.id; })->id + 1;
        }
    }

    void save() const {
        std::ofstream file(filename);
        for (const auto& t : tasks) {
            file << t.id << " " << static_cast<int>(t.priority) << " "
                << t.completed << " " << t.description << "\n";
        }
    }

    std::string priorityColor(Priority p) const {
        switch (p) {
        case Priority::Low:    return GREEN + "Low" + RESET;
        case Priority::Medium: return YELLOW + "Medium" + RESET;
        case Priority::High:   return RED + "High" + RESET;
        }
        return "";
    }

public:
    TaskManager() { load(); }
    ~TaskManager() { save(); }

    void add(const std::string& desc, Priority prio = Priority::Medium) {
        if (desc.empty()) {
            std::cout << RED << "Error: Task description cannot be empty!\n" << RESET;
            return;
        }
        tasks.emplace_back(nextId++, desc, prio);
        std::cout << GREEN << "Task added successfully (ID: " << nextId - 1 << ")\n" << RESET;
    }

    void list() const {
        if (tasks.empty()) {
            std::cout << YELLOW << "No tasks yet. Use 'add' to create one!\n" << RESET;
            return;
        }

        std::cout << BOLD << CYAN << "\n=== Task List ===\n" << RESET;
        for (const auto& t : tasks) {
            std::string status = t.completed ? GREEN + "✓" + RESET : RED + "✗" + RESET;
            std::cout << status << " [" << t.id << "] " << t.description
                << "  (Priority: " << priorityColor(t.priority) << ")\n";
        }
        std::cout << "\n";
    }

    void complete(int id) {
        auto it = std::find_if(tasks.begin(), tasks.end(), [id](const Task& t) { return t.id == id; });
        if (it != tasks.end()) {
            it->completed = true;
            std::cout << GREEN << "Task marked as completed: " << id << "\n" << RESET;
        }
        else {
            std::cout << RED << "Error: Task not found with ID: " << id << "\n" << RESET;
        }
    }

    void remove(int id) {
        auto oldSize = tasks.size();
        tasks.erase(std::remove_if(tasks.begin(), tasks.end(),
            [id](const Task& t) { return t.id == id; }), tasks.end());

        if (tasks.size() < oldSize) {
            std::cout << GREEN << "Task deleted: " << id << "\n" << RESET;
        }
        else {
            std::cout << RED << "Error: Task not found with ID: " << id << "\n" << RESET;
        }
    }

    void search(const std::string& keyword) const {
        if (keyword.empty()) return;
        std::string lowerKeyword = keyword;
        std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);

        bool found = false;
        for (const auto& t : tasks) {
            std::string descLower = t.description;
            std::transform(descLower.begin(), descLower.end(), descLower.begin(), ::tolower);
            if (descLower.find(lowerKeyword) != std::string::npos) {
                std::string status = t.completed ? GREEN + "✓" + RESET : RED + "✗" + RESET;
                std::cout << status << " [" << t.id << "] " << t.description
                    << "  (Priority: " << priorityColor(t.priority) << ")\n";
                found = true;
            }
        }
        if (!found) std::cout << YELLOW << "No tasks found matching your search.\n" << RESET;
    }
};

void showHelp() {
    std::cout << BOLD << CYAN << "\nAvailable Commands:\n" << RESET;
    std::cout << "  add <task description>                  → Add a new task\n";
    std::cout << "  add <task> high|medium|low              → Add with priority\n";
    std::cout << "  list                                    → Show all tasks\n";
    std::cout << "  done <id>                               → Mark task as completed\n";
    std::cout << "  delete <id>                             → Delete a task\n";
    std::cout << "  search <keyword>                        → Search in task descriptions\n";
    std::cout << "  help                                    → Show this help\n";
    std::cout << "  exit                                    → Quit the application\n\n";
}

int main() {
    TaskManager manager;
    std::string input;

    std::cout << BOLD << CYAN << "=== Modern C++ Terminal Task Manager ===\n" << RESET;
    showHelp();

    while (true) {
        std::cout << BOLD << "> " << RESET;
        std::getline(std::cin, input);

        if (input.empty()) continue;

        std::istringstream iss(input);
        std::string command;
        iss >> command;

        if (command == "exit" || command == "quit") {
            std::cout << YELLOW << "Goodbye!\n" << RESET;
            break;
        }
        else if (command == "help") {
            showHelp();
        }
        else if (command == "list") {
            manager.list();
        }
        else if (command == "add") {
            std::string rest;
            std::getline(iss, rest);
            if (rest.empty()) {
                std::cout << RED << "Error: Please provide a task description.\n" << RESET;
                continue;
            }

            std::string desc = rest;
            Priority prio = Priority::Medium;
            std::string lastWord;
            std::istringstream temp(rest);
            while (temp >> lastWord); // get last word

            if (lastWord == "high" || lastWord == "High") {
                prio = Priority::High;
                desc = rest.substr(0, rest.rfind(lastWord));
                while (!desc.empty() && std::isspace(desc.back())) desc.pop_back();
            }
            else if (lastWord == "medium" || lastWord == "Medium") {
                prio = Priority::Medium;
                desc = rest.substr(0, rest.rfind(lastWord));
                while (!desc.empty() && std::isspace(desc.back())) desc.pop_back();
            }
            else if (lastWord == "low" || lastWord == "Low") {
                prio = Priority::Low;
                desc = rest.substr(0, rest.rfind(lastWord));
                while (!desc.empty() && std::isspace(desc.back())) desc.pop_back();
            }

            manager.add(desc, prio);
        }
        else if (command == "done" || command == "complete") {
            int id;
            if (iss >> id) manager.complete(id);
            else std::cout << RED << "Error: Please provide a task ID.\n" << RESET;
        }
        else if (command == "delete" || command == "del") {
            int id;
            if (iss >> id) manager.remove(id);
            else std::cout << RED << "Error: Please provide a task ID.\n" << RESET;
        }
        else if (command == "search") {
            std::string keyword;
            std::getline(iss, keyword);
            if (!keyword.empty()) manager.search(keyword);
            else std::cout << RED << "Error: Please provide a search keyword.\n" << RESET;
        }
        else {
            std::cout << RED << "Unknown command: " << command << "\nType 'help' for available commands.\n" << RESET;
        }
    }

    return 0;
}