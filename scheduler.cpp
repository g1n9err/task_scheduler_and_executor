#include "scheduler.hpp"

#include "exceptions.hpp"

#include <algorithm>
#include <utility>

namespace {

bool hasHigherPriority(const Task* a, const Task* b) noexcept {
    return a->priority() > b->priority();
}

}

std::unique_ptr<Task> TaskFactory::create(const std::string& type,
                                          const std::string& id,
                                          int priority,
                                          const std::vector<std::string>& params) {
    if (type == "calculation") {
        if (params.size() != 3) {
            throw InvalidParametersException(
                "calculation expects: <operation> <lhs> <rhs>");
        }
        const std::string& operation = params[0];
        double ch1 = parseNumber(params[1], "left operand");
        double ch2 = parseNumber(params[2], "right operand");
        return std::make_unique<CalculationTask>(id, priority, operation, ch1, ch2);
    }
    if (type == "message") {
        if (params.size() != 1) {
            throw InvalidParametersException("message expects: \"<text>\"");
        }
        return std::make_unique<MessageTask>(id, priority, params[0]);
    }
    if (type == "delay") {
        if (params.size() != 1) {
            throw InvalidParametersException("delay expects: <duration_ms>");
        }
        return std::make_unique<DelayTask>(id, priority, parseDuration(params[0]));
    }
    throw InvalidParametersException("unknown task type '" + type + "'");
}

double TaskFactory::parseNumber(const std::string& text, const std::string& what) {
    try {
        std::size_t parsed = 0;
        double value = std::stod(text, &parsed);
        if (parsed != text.size()) {
            throw std::invalid_argument(text);
        }
        return value;
    } catch (const std::exception&) {
        throw InvalidParametersException(
            what + " must be a number, got '" + text + "'");
    }
}

long long TaskFactory::parseDuration(const std::string& text) {
    try {
        std::size_t parsed = 0;
        long long value = std::stoll(text, &parsed);
        if (parsed != text.size()) {
            throw std::invalid_argument(text);
        }
        if (value < 0) {
            throw InvalidParametersException(
                "duration must be nonnegative, got '" + text + "'");
        }
        return value;
    } catch (const InvalidParametersException&) {
        throw;
    } catch (const std::exception&) {
        throw InvalidParametersException(
            "duration must be an integer, got '" + text + "'");
    }
}

void Scheduler::registerTask(TaskPtr task) {
    const std::string& id = task->id();
    if (m_indexById.find(id) != m_indexById.end()) {
        throw DuplicateTaskIdException(id);
    }
    m_indexById.emplace(id, m_tasks.size());
    m_tasks.push_back(std::move(task));
}

bool Scheduler::executeNext() {
    std::vector<Task*> pending = pendingByPriority();
    if (pending.empty()) {
        return false;
    }

    Task* task = pending.front();
    try {
        task->execute();
    } catch (const TaskSchedulerException& e) {
        task->markFailed(e.what());
    } catch (const std::exception& e) {
        task->markFailed(e.what());
    }
    m_history.push_back(m_indexById[task->id()]);
    return true;
}

std::size_t Scheduler::executeAll() {
    std::size_t executed = 0;
    while (executeNext()) {
        ++executed;
    }
    return executed;
}

void Scheduler::cancelTask(const std::string& id) {
    auto it = m_indexById.find(id);
    if (it == m_indexById.end()) {
        throw TaskNotFoundException(id);
    }
    Task& task = *m_tasks[it->second];
    if (task.status() != TaskStatus::Pending) {
        throw InvalidTaskStateException(id, task.status());
    }
    task.markCancelled();
}

const Task* Scheduler::peekNext() const {
    std::vector<Task*> pending = pendingByPriority();
    return pending.empty() ? nullptr : pending.front();
}

const Task* Scheduler::findTask(const std::string& id) const {
    auto it = m_indexById.find(id);
    return it == m_indexById.end() ? nullptr : m_tasks[it->second].get();
}

std::vector<const Task*> Scheduler::allTasks() const {
    std::vector<const Task*> out;
    out.reserve(m_tasks.size());
    for (const TaskPtr& task : m_tasks) {
        out.push_back(task.get());
    }
    return out;
}

std::vector<const Task*> Scheduler::history() const {
    std::vector<const Task*> out;
    out.reserve(m_history.size());
    for (std::size_t index : m_history) {
        out.push_back(m_tasks[index].get());
    }
    return out;
}

std::map<TaskStatus, std::size_t> Scheduler::statusCounts() const {
    std::map<TaskStatus, std::size_t> counts;
    for (const TaskPtr& task : m_tasks) {
        ++counts[task->status()];
    }
    return counts;
}

std::vector<Task*> Scheduler::pendingByPriority() const {
    std::vector<Task*> pending;
    for (const TaskPtr& task : m_tasks) {
        if (task->status() == TaskStatus::Pending) {
            pending.push_back(task.get());
        }
    }
    std::stable_sort(pending.begin(), pending.end(), hasHigherPriority);
    return pending;
}
