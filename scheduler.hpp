#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "task.hpp"

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class TaskFactory {
public:
    static std::unique_ptr<Task> create(const std::string& type,
                                        const std::string& id,
                                        int priority,
                                        const std::vector<std::string>& params);

private:
    static double parseNumber(const std::string& text, const std::string& what);
    static long long parseDuration(const std::string& text);
};

class Scheduler {
    using TaskPtr = std::unique_ptr<Task>;

    std::vector<TaskPtr> m_tasks;
    std::unordered_map<std::string, std::size_t> m_indexById;
    std::vector<std::size_t> m_history;

public:
    void registerTask(TaskPtr task);

    bool executeNext();

    std::size_t executeAll();

    void cancelTask(const std::string& id);

    const Task* peekNext() const;

    const Task* findTask(const std::string& id) const;
    std::vector<const Task*> allTasks() const;
    std::vector<const Task*> history() const;
    std::map<TaskStatus, std::size_t> statusCounts() const;

private:
    std::vector<Task*> pendingByPriority() const;
};
#endif
