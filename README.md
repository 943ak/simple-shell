# MiniShell – A Simple UNIX Shell in C

MiniShell is a basic UNIX shell implementation written in C.
It supports command execution, pipelines, and basic I/O redirection
using low-level POSIX system calls.

This project was built to understand how shells work internally,
including process creation, pipes, and file descriptor manipulation.

---

## ✨ Features

- Execute external commands (`ls`, `grep`, `cat`, etc.)
- Command pipelines (`|`)
- Input redirection (`<`)
- Output redirection (`>`)
- Multiple command chaining via pipes
- Custom interactive prompt

---

## 🛠️ Concepts Used

- `fork()`
- `execvp()`
- `pipe()`
- `dup2()`
- `wait()`
- File descriptors
- Tokenization and command parsing

---

## 🚀 Build & Run

### Build
```bash
make

### Run
./minishell

## Example Use Case

- ls -l | grep ".c"
- cat file.txt | wc -l
- sort < input.txt > output.txt
