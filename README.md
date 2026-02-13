# async-runtime (C++20)

A minimal async runtime built to showcase a classic architecture: *event loop + worker thread pool + tasks*.

- A task’s `execute()` runs on a worker thread.
- Task completion and the user callback always run on the **event loop thread** (serialized in one place).
- Each task exposes a `TaskHandle` for `cancel()` and `wait()`.

This project is intended for learning/portfolio use and is covered by GoogleTest unit tests.

## Features

- Dedicated event loop thread
- Fixed-size worker pool (configured via `AsyncRuntime` constructor)
- Priority-based task queue (higher `priority` starts first)
- Cancellation of queued tasks: calling `cancel()` before a task starts guarantees `execute()` won’t run
- Error propagation: exceptions from `execute()` become `error` for the callback
- `cleanup()` is executed even on errors

## Quick start

### Requirements

- CMake 3.14+
- C++20 compiler (Clang/GCC)
- POSIX threads

### Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -- -j 8
```

Debug builds enable AddressSanitizer and extra debug flags (see `CMakeLists.txt`).

### Run demo

```bash
./build/main
```

### Tests

```bash
ctest --test-dir build --output-on-failure
```

## Usage examples

### 1) Run background work (`launch`) + wait

```cpp
#include "event_loop/runtime/runtime.h"

AsyncRuntime runtime(2);

TaskHandle handle = runtime.launch([] {
  // runs on a worker thread
  do_some_work();
});

handle.wait();     // wait for this task
runtime.blockOn(); // wait until the loop processes all completions/callbacks
```

### 2) Read a file (`readFile`) and handle the result

```cpp
#include <iostream>
#include "event_loop/runtime/runtime.h"

AsyncRuntime runtime(1);

runtime.readFile("/path/to/file.txt",
  [&](std::optional<std::string> error,
      std::optional<std::string> text) {
    // callback runs on the event loop thread
    if (error) {
      std::cerr << "readFile failed: " << *error << "\n";
      return;
    }
    std::cout << "bytes: " << text->size() << "\n";
  });

runtime.blockOn();
```

### 3) Cancel a queued task

If you call `cancel()` before the task is picked up by a worker, `execute()` will not run and
the callback receives `error = "Cancelled"`.

```cpp
AsyncRuntime runtime(1);

TaskHandle handle = runtime.readFile("big.txt", [](auto err, auto value) {
  // err == "Cancelled", value is empty
});

handle.cancel();
runtime.blockOn();
```

## Architecture

### Building blocks

- `AsyncRuntime` — a thin facade over the event loop.
  - `launch(fn)` — schedules a `RunTask`.
  - `readFile(path, cb)` — schedules a `ReadFileTask`.
  - `blockOn()` — waits until all scheduled tasks are finished and processed.

- `EventLoop`
  - Runs on a **dedicated thread**.
  - Owns two priority queues:
    - `queue` — tasks waiting to start
    - `result` — completed tasks (success/error/cancelled)
  - Ensures `finish()` (and therefore user callbacks) runs on the loop thread.

- `ThreadsPool`
  - A set of workers that execute `BaseTask::execute()`.
  - After execution it triggers `$complete`, which the `EventLoop` subscribes to.

- `Task` / `BaseTask`
  - `Task<Result>` stores `value` and `Callback<Result>`.
  - `TaskHandle` supports:
    - `cancel()` — request cancellation
    - `wait()` — wait for a specific task to complete

### Execution flow (high-level)

```mermaid
sequenceDiagram
  participant U as User thread
  participant RT as AsyncRuntime
  participant L as EventLoop thread
  participant P as ThreadsPool workers

  U->>RT: launch/readFile
  RT->>L: push(task)
  L->>P: schedule(task)
  P->>P: task.execute()
  P-->>L: $complete(task)
  L->>L: task.finish() (callback)
  L->>L: task.cleanup(); markDone()
  U->>RT: blockOn()
  RT->>L: wait pending==0
```

### Threading guarantees

- `execute()` never runs on the event loop thread (it runs on a worker).
- Callbacks (via `Task::finish()`) always run on the event loop thread.

These guarantees are asserted by unit tests:
- callback runs on the loop thread
- queued cancellation prevents `execute()`
- `cleanup()` runs even when `execute()` throws

### Cancellation semantics

- `TaskHandle::cancel()` sets `cancelRequested`.
- If the task is still queued, it is marked as cancelled and delivered to `result`.
- If the task is already running on a worker, it is not forcibly interrupted (cooperative cancellation requires checking `isCancelRequested()` inside `execute()`).

## Adding a new task

1) Derive from `Task<Result>`
2) Produce `value` in `execute()` (or throw to signal an error)
3) Release resources in `cleanup()`
4) Optionally provide a callback (or pass `std::nullopt` if no callback is needed)

See `ReadFileTask` and `RunTask` in `src/event_loop/task/...`.

## Formatting

```bash
./scripts/format.sh
```
