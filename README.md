# Task Scheduler and Executor (C++17)

Console application for creating, organizing, scheduling, executing and
managing tasks of different types. Single-threaded and sequential by design:
no threads, no asynchronous execution, no real-time scheduling.

## Build

```bash
g++ -std=c++17 -Wall -Wextra -o scheduler main.cpp task.cpp scheduler.cpp ui.cpp
```

(clang++ works with the same command. Compiles with zero warnings.)

## Run

```bash
./scheduler
```

## Project structure

| File | Role |
|---|---|
| `task.hpp` / `task.cpp` | `TaskStatus` enum, abstract `Task` base class (Command pattern) and the three concrete tasks: `CalculationTask`, `MessageTask`, `DelayTask` |
| `exceptions.hpp` | Exception hierarchy rooted at `TaskSchedulerException`: `DuplicateTaskIdException`, `TaskNotFoundException`, `InvalidTaskStateException`, `InvalidParametersException`, `DivisionByZeroException` |
| `scheduler.hpp` / `scheduler.cpp` | `TaskFactory` (Factory pattern) + `Scheduler`: registration, priority ordering, execution, cancellation, execution history |
| `ui.hpp` / `ui.cpp` | `ConsoleUI` — the only component that performs console input/output |
| `main.cpp` | Entry point (`ConsoleUI ui; ui.run();`) |

## Commands

| Command | Description |
|---|---|
| `add calculation <id> <priority> <add\|sub\|mul\|div> <ch1> <ch2>` | Register an arithmetic task |
| `add message <id> <priority> "<text>"` | Register a message task (quoted text may contain spaces) |
| `add delay <id> <priority> <duration_ms>` | Register a simulated delay task (no sleeping occurs) |
| `execute_next` | Execute the next pending task by scheduling rules |
| `execute_all` | Execute all remaining pending tasks sequentially |
| `cancel <id>` | Cancel a pending task |
| `status <id>` | Show the current status of a task |
| `list` | Show all registered tasks (including completed/failed/cancelled) |
| `history` | Show executed tasks in real execution order |
| `counts` | Show the number of tasks in each state |
| `help` / `quit` | Command list / exit |

## Demonstration scenarios

### Scenario 1 — registration, priorities, failure isolation

```
> add calculation T001 5 add 10 20
Task T001 registered successfully.
> add message T002 3 "Hello from C++"
Task T002 registered successfully.
> add delay T003 2 2000
Task T003 registered successfully.
> add calculation T004 4 div 10 0
Task T004 registered successfully.
> add message T001 1 "Duplicate"
Error: Task ID already exists: T001
> execute_all
Executing T001 (priority 5)
Result: 30
Status: Completed
Executing T004 (priority 4)
Error: Division by zero
Status: Failed
Executing T002 (priority 3)
Result: Hello from C++
Status: Completed
Executing T003 (priority 2)
Result: Simulated delay of 2000 ms
Status: Completed
All pending tasks processed.
```

Notes: higher priority executes first; `T004 div 10 0` registers
successfully but transitions to **Failed** at execution time; the scheduler
continues with the remaining tasks (failure isolation).

### Scenario 2 — equal priorities keep registration order (§2.4)

```
> add delay T001 2 100
> add delay T002 5 200
> add delay T003 3 300
> add delay T004 5 400
> add delay T005 1 500
> execute_all
Executing T002 (priority 5) ... Completed
Executing T004 (priority 5) ... Completed
Executing T003 (priority 3) ... Completed
Executing T001 (priority 2) ... Completed
Executing T005 (priority 1) ... Completed
> history
T002 | ... T004 | ... T003 | ... T001 | ... T005 | ...
```

Expected order: T002, T004, T003, T001, T005 — priority first, and among
equal priorities the earlier registered task runs first (FIFO, via
`std::stable_sort`). History reflects the real execution order, not the
registration order.

### Scenario 3 — cancellation rules

```
> add message T005 8 "Pending task"
Task T005 registered successfully.
> cancel T005
Task T005 cancelled successfully.
> execute_all
No pending tasks available.
> cancel T001
Error: Only pending tasks can be cancelled. Task T001 is currently Completed
> status T005
Task T005 | Cancelled
```

Notes: only `Pending` tasks can be cancelled; completed/failed tasks and
nonexistent IDs produce errors; cancelled tasks stay visible.

## Design summary

- **Command pattern** — `Task` encapsulates everything needed to perform an
  operation (parameters + `execute()`); the scheduler only orders and invokes.
- **Factory pattern** — `TaskFactory::create()` maps a type string to a
  concrete task and validates everything before construction.
- **Runtime polymorphism** — the scheduler stores `std::unique_ptr<Task>` and
  calls `task->execute()`; the concrete implementation is chosen at run time.
- **Ownership** — the scheduler owns all tasks via
  `std::vector<std::unique_ptr<Task>>`; ownership travels Factory → UI →
  Scheduler with move semantics; copying of tasks is deleted.
- **Encapsulated state machine** — `TaskStatus` changes only through `Task`
  methods (`setStatus` is protected; `markFailed`/`markCancelled` are the
  scheduler's entry points); terminal states never re-execute.
- **Exceptions** — one hierarchy separates registration failures from
  execution failures; the UI catches `TaskSchedulerException` in one place.
- **Separation of concerns** — `Task`/`TaskFactory`/`Scheduler` contain no
  I/O; all console input/output lives in `ConsoleUI` (SOLID).
