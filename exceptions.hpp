#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include "task.hpp"

#include <stdexcept>
#include <string>

class TaskSchedulerException : public std::runtime_error {
public:
    explicit TaskSchedulerException(const std::string& message)
        : std::runtime_error(message) {}
};

class DuplicateTaskIdException : public TaskSchedulerException {
public:
    explicit DuplicateTaskIdException(const std::string& id)
        : TaskSchedulerException("Task ID already exists: " + id) {}
};

class TaskNotFoundException : public TaskSchedulerException {
public:
    explicit TaskNotFoundException(const std::string& id)
        : TaskSchedulerException("Task not found: " + id) {}
};

class InvalidTaskStateException : public TaskSchedulerException {
public:
    InvalidTaskStateException(const std::string& id, TaskStatus current)
        : TaskSchedulerException("Only pending tasks can be cancelled. Task "
                                 + id + " is currently " + toString(current)) {}
};

class InvalidParametersException : public TaskSchedulerException {
public:
    explicit InvalidParametersException(const std::string& reason)
        : TaskSchedulerException("Invalid task parameters: " + reason) {}
};

class DivisionByZeroException : public TaskSchedulerException {
public:
    DivisionByZeroException()
        : TaskSchedulerException("Division by zero") {}
};
#endif
