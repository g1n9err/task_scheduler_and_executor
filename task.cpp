#include "task.hpp"

#include "exceptions.hpp"

#include <utility>

std::string toString(TaskStatus status) {
    switch (status) {
        case TaskStatus::Pending:   return "Pending";
        case TaskStatus::Completed: return "Completed";
        case TaskStatus::Failed:    return "Failed";
        case TaskStatus::Cancelled: return "Cancelled";
    }
    return "Unknown";
}

Task::Task(std::string id, int priority)
    : m_id(std::move(id)),
      m_priority(priority),
      m_status(TaskStatus::Pending) {}

void Task::setStatus(TaskStatus status) { m_status = status; }

void Task::setResult(std::string result) { m_result = std::move(result); }

void Task::markFailed(const std::string& reason) {
    setResult(reason);
    setStatus(TaskStatus::Failed);
}

void Task::markCancelled() {
    setStatus(TaskStatus::Cancelled);
}

namespace {

std::string formatNumber(double value) {
    if (value == static_cast<long long>(value)) {
        return std::to_string(static_cast<long long>(value));
    }
    return std::to_string(value);
}

}

CalculationTask::CalculationTask(std::string id, int priority,
                                 std::string operation, double ch1, double ch2)
    : Task(std::move(id), priority),
      m_operation(std::move(operation)),
      m_ch1(ch1),
      m_ch2(ch2) {
    if (m_operation != "add" && m_operation != "sub" &&
        m_operation != "mul" && m_operation != "div") {
        throw InvalidParametersException("unknown operation '" + m_operation + "'");
    }
}

void CalculationTask::execute() {
    double value = 0.0;
    if (m_operation == "add") {
        value = m_ch1 + m_ch2;
    } else if (m_operation == "sub") {
        value = m_ch1 - m_ch2;
    } else if (m_operation == "mul") {
        value = m_ch1 * m_ch2;
    } else if (m_operation == "div") {
        if (m_ch2 == 0.0) {
            throw DivisionByZeroException();
        }
        value = m_ch1 / m_ch2;
    }
    setResult(formatNumber(value));
    setStatus(TaskStatus::Completed);
}

std::string CalculationTask::type() const { return "Calculation"; }

std::string CalculationTask::parameters() const {
    return m_operation + " " + formatNumber(m_ch1) + " " + formatNumber(m_ch2);
}

MessageTask::MessageTask(std::string id, int priority, std::string message)
    : Task(std::move(id), priority),
      m_message(std::move(message)) {
    if (m_message.empty()) {
        throw InvalidParametersException("message must not be empty");
    }
}

void MessageTask::execute() {
    setResult(m_message);
    setStatus(TaskStatus::Completed);
}

std::string MessageTask::type() const { return "Message"; }

std::string MessageTask::parameters() const { return m_message; }

DelayTask::DelayTask(std::string id, int priority, long long durationMs)
    : Task(std::move(id), priority),m_durationMs(durationMs) {
    if (m_durationMs < 0) {
        throw InvalidParametersException("duration must be nonnegative");
    }
}

void DelayTask::execute() {
    setResult("Simulated delay of " + std::to_string(m_durationMs) + " ms");
    setStatus(TaskStatus::Completed);
}

std::string DelayTask::type() const { return "Delay"; }

std::string DelayTask::parameters() const {
    return std::to_string(m_durationMs) + " ms";
}
