#ifndef TASK_HPP
#define TASK_HPP

#include <string>

enum class TaskStatus {
    Pending,
    Completed,
    Failed,
    Cancelled
};

std::string toString(TaskStatus status);

class Task {
    std::string m_id;
    int m_priority;
    TaskStatus m_status;
    std::string m_result;

public:
    Task(std::string id, int priority);
    virtual ~Task() noexcept = default;

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    virtual void execute() = 0;

    virtual std::string type() const = 0;
    virtual std::string parameters() const = 0;

    void markFailed(const std::string& reason);
    void markCancelled() noexcept;

    const std::string& id() const noexcept { return m_id; }
    int priority() const noexcept { return m_priority; }
    TaskStatus status() const noexcept { return m_status; }
    const std::string& result() const noexcept { return m_result; }

protected:
    void setStatus(TaskStatus status) noexcept;
    void setResult(std::string result) noexcept;
};

class CalculationTask : public Task {
    std::string m_operation;
    double m_ch1;
    double m_ch2;

public:
    CalculationTask(std::string id, int priority,
                    std::string operation, double ch1, double ch2);

    void execute() override;
    std::string type() const override;
    std::string parameters() const override;
};

class MessageTask : public Task {
    std::string m_message;

public:
    MessageTask(std::string id, int priority, std::string message);

    void execute() override;
    std::string type() const override;
    std::string parameters() const override;
};

class DelayTask : public Task {
    long long m_durationMs;

public:
    DelayTask(std::string id, int priority, long long durationMs);

    void execute() override;
    std::string type() const override;
    std::string parameters() const override;
};
#endif
