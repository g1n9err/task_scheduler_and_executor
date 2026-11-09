#ifndef UI_HPP
#define UI_HPP

#include "exceptions.hpp"
#include "scheduler.hpp"

#include <string>
#include <vector>

class ConsoleUI {
    Scheduler m_scheduler;

public:
    void run();

private:
    bool handleCommand(const std::string& line);

    void cmdAdd(const std::vector<std::string>& tokens);
    void cmdExecuteNext();
    void cmdExecuteAll();
    void cmdCancel(const std::vector<std::string>& tokens);
    void cmdStatus(const std::vector<std::string>& tokens);
    void cmdList();
    void cmdHistory();
    void cmdCounts();

    void printError(const TaskSchedulerException& e);
    
    static std::vector<std::string> tokenize(const std::string& line);
};
#endif
