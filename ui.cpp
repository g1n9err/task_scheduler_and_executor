#include "ui.hpp"

#include <iostream>

namespace {

void printHelp() {
    std::cout << "Commands:\n"
              << "  add calculation <id> <priority> <add|sub|mul|div> <ch1> <ch2>\n"
              << "  add message <id> <priority> \"<text>\"\n"
              << "  add delay <id> <priority> <duration_ms>\n"
              << "  execute_next | execute_all\n"
              << "  cancel <id> | status <id>\n"
              << "  list | history | counts | help | quit\n";
}

void printOutcome(const Task& task) {
    if (task.status() == TaskStatus::Completed) {
        std::cout << "Result: " << task.result() << "\n";
    } else if (task.status() == TaskStatus::Failed) {
        std::cout << "Error: " << task.result() << "\n";
    }
    std::cout << "Status: " << toString(task.status()) << "\n";
}

int parsePriority(const std::string& text) {
    try {
        std::size_t parsed = 0;
        long value = std::stol(text, &parsed);
        if (parsed != text.size()) {
            throw std::invalid_argument(text);
        }
        return static_cast<int>(value);
    } catch (const std::exception&) {
        throw InvalidParametersException(
            "priority must be an integer, got '" + text + "'");
    }
}

}

void ConsoleUI::run() {
    std::cout << "Task Scheduler and Executor \n"
              << "Type 'help' for the command list.\n";
    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (line == "quit" || line == "exit") {
            break;
        }
        try {
            if (!handleCommand(line)) {
                std::cout << "Unknown command. Type 'help'.\n";
            }
        } catch (const TaskSchedulerException& e) {
            printError(e);
        }
    }
    std::cout << "Bye.\n";
}

bool ConsoleUI::handleCommand(const std::string& line) {
    std::vector<std::string> tokens = tokenize(line);
    if (tokens.empty()) {
        return true;
    }
    const std::string& command = tokens[0];

    if (command == "add")                { cmdAdd(tokens); }
    else if (command == "execute_next")  { cmdExecuteNext(); }
    else if (command == "execute_all")   { cmdExecuteAll(); }
    else if (command == "cancel")        { cmdCancel(tokens); }
    else if (command == "status")        { cmdStatus(tokens); }
    else if (command == "list")          { cmdList(); }
    else if (command == "history")       { cmdHistory(); }
    else if (command == "counts")        { cmdCounts(); }
    else if (command == "help")          { printHelp(); }
    else {
        return false;
    }
    return true;
}

void ConsoleUI::cmdAdd(const std::vector<std::string>& tokens) {
    if (tokens.size() < 4) {
        throw InvalidParametersException("usage: add <type> <id> <priority> <params...>");
    }
    int priority = parsePriority(tokens[3]);
    std::vector<std::string> params(tokens.begin() + 4, tokens.end());

    std::unique_ptr<Task> task = TaskFactory::create(tokens[1], tokens[2], priority, params);
    m_scheduler.registerTask(std::move(task));
    std::cout << "Task " << tokens[2] << " registered successfully.\n";
}

void ConsoleUI::cmdExecuteNext() {
    const Task* next = m_scheduler.peekNext();
    if (next == nullptr) {
        std::cout << "No pending tasks available.\n";
        return;
    }
    std::cout << "Executing " << next->id()
              << " (priority " << next->priority() << ")\n";
    m_scheduler.executeNext();
    printOutcome(*next);
}

void ConsoleUI::cmdExecuteAll() {
    if (m_scheduler.peekNext() == nullptr) {
        std::cout << "No pending tasks available.\n";
        return;
    }
    while (m_scheduler.peekNext() != nullptr) {
        cmdExecuteNext();
    }
    std::cout << "All pending tasks processed.\n";
}

void ConsoleUI::cmdCancel(const std::vector<std::string>& tokens) {
    if (tokens.size() != 2) {
        throw InvalidParametersException("usage: cancel <id>");
    }
    m_scheduler.cancelTask(tokens[1]);
    std::cout << "Task " << tokens[1] << " cancelled successfully.\n";
}

void ConsoleUI::cmdStatus(const std::vector<std::string>& tokens) {
    if (tokens.size() != 2) {
        throw InvalidParametersException("usage: status <id>");
    }
    const Task* task = m_scheduler.findTask(tokens[1]);
    if (task == nullptr) {
        throw TaskNotFoundException(tokens[1]);
    }
    std::cout << "Task " << task->id() << " | " << toString(task->status()) << "\n";
}

void ConsoleUI::cmdList() {
    for (const Task* task : m_scheduler.allTasks()) {
        std::cout << task->id() << " | " << task->type()
                  << " | priority " << task->priority()
                  << " | " << toString(task->status());
        if (!task->result().empty()) {
            std::cout << " | " << task->result();
        }
        std::cout << "\n";
    }
}

void ConsoleUI::cmdHistory() {
    for (const Task* task : m_scheduler.history()) {
        std::cout << task->id() << " | " << task->type()
                  << " | priority " << task->priority()
                  << " | " << toString(task->status())
                  << " | " << (task->status() == TaskStatus::Failed ? "Error: " : "Result: ")
                  << task->result() << "\n";
    }
}

void ConsoleUI::cmdCounts() {
    for (const auto& [status, count] : m_scheduler.statusCounts()) {
        std::cout << toString(status) << ": " << count << "\n";
    }
}

void ConsoleUI::printError(const TaskSchedulerException& e) {
    std::cout << "Error: " << e.what() << "\n";
}

std::vector<std::string> ConsoleUI::tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current;
    bool quot = false;
    for (char c : line) {
        if (c == '"') {
            quot = !quot;
        } else if ((c == ' ' || c == '\t') && !quot) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}
