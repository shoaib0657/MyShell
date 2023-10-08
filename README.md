# MyShell

## Introduction
This is a custom shell implementation in C. It allows users to run various shell commands, including sequential and parallel execution, redirection, and piped commands.

## Features
- **Basic Shell Commands:** Execute common shell commands like **`ls`**, **`cd`**, **`mkdir`** and more.
- **Sequential Execution:** Run multiple commands sequentially using the **`##`** delimiter.
- **Parallel Execution:** Run multiple commands in parallel using the **`&&`** delimiter.
- **Output Redirection:** Redirect the output of a single command to a specified file using **`>`**.
- **Command Piping:** Execute a pipeline of commands separated by **`|`** for complex operations.
- **Signal Handling:** Handle Ctrl+C (SIGINT) and Ctrl+Z (SIGTSTP) signals for graceful termination and process suspension.

## Code Explanation
Here's a brief explanation of the key components and functions in the custom shell code:

## `parseInput`
  The `parseInput` function is crucial for breaking down user input into executable commands and their respective arguments. Here's how it works:
  - This function takes a user input string and splits it into individual commands or arguments based on spaces (' ') as a delimiter.
  - It utilizes the **`strsep`** function to perform this parsing and stores the resulting commands in the **`commands`** array.
  - The **`commands`** array is **essential** for organizing and executing user-entered commands correctly.

## `executeCommand`
  The `executeCommand` function is responsible for executing a single command. Here's a detailed breakdown:
  - This function forks a new process using **`fork()`**, creating a child process.
  - In the child process, it sets up signal handling using **`signal(SIGINT, SIG_DFL)`** and **`signal(SIGTSTP, SIG_DFL)`** to allow the default behavior of Ctrl+C and Ctrl+Z.
  - It uses the **`execvp`** system call to execute the specified command with its arguments. If the execution fails, it prints an error message.
  - The child process exits when the command execution is complete.
  - In the parent process, it waits for the child process to finish using **`wait(NULL)`**. This ensures proper synchronization and prevents zombie processes.
  - This function also includes a **special case** for the **`cd`** command, allowing users to change the current working directory. It checks if the first command is **`cd`** and handles directory changes accordingly.

## `executeParallelCommands`
  The **`executeParallelCommands`** function is used when the user wants to run multiple commands in parallel, separated by the `&&` delimiter. Here's how it works:
  - It splits the input into separate commands and arguments, then forks and executes each command as a separate child process.
  - The parent process waits for all child processes to complete, ensuring proper execution and synchronization.

## `executeSequentialCommands`
  The **`executeSequentialCommands`** function is invoked when the user wants to run multiple commands sequentially, separated by the `##` delimiter. Here's how it works:
  - It parses the input into individual commands and arguments.
  - It then executes each command one after the other, ensuring that each command completes before the next one starts. This sequential execution is useful for scenarios where the order of commands is crucial.

## `executeCommandRedirection`

The `executeCommandRedirection` function allows the redirection of the output of a single command to a user-specified file. It ensures that the command's output is saved to the designated file, facilitating data capture and analysis. Below is an overview of how this function works:

- **Output File Validation**:
  - The function begins by checking if the specified output file name is empty. If it is, the function returns without performing any redirection, ensuring that output redirection is optional.

- **Separating Output Redirection**:
  - Next, the function modifies the `commands` array to separate the output redirection part (represented by `>` symbol) from the actual command and its arguments. This separation is achieved by setting the element at index 1 (corresponding to the `>` symbol) to `NULL`. This ensures that the `execvp` function considers only the command and its arguments and does not include the `>` symbol or the output file name as part of the command's arguments.

- **Child Process Creation**:
  - The function then forks a new child process using `fork()`. This child process is responsible for executing the command with output redirection.

- **Signal Handling**:
  - In the child process, signal handling is set up using `signal(SIGINT, SIG_DFL)` and `signal(SIGTSTP, SIG_DFL)`. These signal handlers allow the default behavior of Ctrl+C (SIGINT) and Ctrl+Z (SIGTSTP), ensuring that the child process responds appropriately to these signals.

- **Output File Handling**:
  - The specified output file is opened using the `open` system call. The `open` call opens the file in write-only mode, creating it if it doesn't exist (`O_WRONLY | O_CREAT | O_TRUNC`). This step ensures that the output file is ready for writing.

- **Output Redirection**:
  - After opening the file, the `dup2` system call is used to duplicate the file descriptor of the opened file onto the standard output file descriptor (`STDOUT_FILENO`). This redirection ensures that any output generated by the command will be written to the specified file instead of being displayed on the terminal.

- **Command Execution**:
  - Once the file redirection is set up, the function executes the command using `execvp`. If the execution of the command fails, the function prints an error message to the terminal, providing feedback to the user.

- **Child Process Termination**:
  - Finally, the child process exits, and control returns to the parent process.

## `executePipedCommands`

The `executePipedCommands` function is responsible for parsing and executing a command line that may contain multiple commands separated by the `|` (pipe) operator. It also handles optional output redirection using `>` symbol. Here's how it works:

- **Variables**:
  - `commands`: An array of command arguments.
  - `outfile`: A string representing the output file name, if specified.
  - `n_commands`: An integer representing the number of individual commands in the pipeline.

- **Checking Output File**:
  - The function checks if an output file is specified in the command line. If so, it extracts and stores the output file name in the `outfile` variable.

- **Splitting into Commands**:
  - It splits the command line into individual commands using the `|` (pipe) symbol as the delimiter and stores each command in the `commands` array.

- **Execution**:
  - If there is at least one command in the pipeline (i.e., `n_commands > 0`), the `executePipedCommands` function calls the `pipe_execute` function, passing the array of commands, the number of commands (`n_commands`), and the output file name (`outfile`) as arguments.

## `pipe_execute`

### Recommended videos
- [Communicating between processes (using pipes) in C](https://youtu.be/Mqb2dVRe0uo?si=XbW6Z5_lT0NmKbwC)
- [Simulating the pipe "|" operator in C](https://youtu.be/Mqb2dVRe0uo?si=XbW6Z5_lT0NmKbwC)

The `pipe_execute` function is responsible for executing a pipeline of commands, where the output of one command is used as the input for the next command. It handles the setup of pipes, forking of child processes, and redirection of input and output between commands. Below is a detailed breakdown of this function:

- **Variables**: 
  - `fd_input` and `fd_output`: These variables are used to manage file descriptors for input and output.
  - `fd[2]`: An array of file descriptors used for the pipe. It has two elements: `fd[0]` for reading (input) and `fd[1]` for writing (output).
  - `pid`: Process ID for the child processes.
  - `buffer`: A character array used for reading and writing data between commands.
  - `bytes_read`: Keeps track of the number of bytes read from the pipe.

- **Initialization**:
  - `fd_input` is initially set to 0, indicating that the initial input comes from standard input (stdin).

- **Loop Through Commands**:
  - The function iterates through each command in the pipeline using a loop (`for` loop with `n_commands` iterations).

- **Pipe Creation**:
  - Inside the loop, it checks if creating a pipe (`pipe(fd)`) fails, and if so, it prints an error message and exits.

- **Forking Child Process**:
  - It forks a new child process using `fork()`. This child process will execute the current command.

- **Child Process Actions**:
  - In the child process:
    - It closes the read end of the pipe (`fd[0]`) as it is not needed for writing to the pipe.
    - It redirects the standard input (`stdin`) of the child process to the previous pipe's read end (`fd_input`) if it is not the first command in the pipeline. This ensures that the output of the previous command becomes the input for the current command.
    - It redirects the standard output (`stdout`) of the child process to the current pipe's write end (`fd[1]`) if it is not the last command in the pipeline. This enables the current command to write its output to the pipe, which will be read by the next command.
    - It splits the command string into individual arguments, storing them in the `args` array.
    - It uses the `execvp` system call to execute the command with its arguments. If execution fails, it prints an error message and exits the child process.

- **Closing Write End of Pipe**:
  - After setting up the redirections, the function closes the write end of the pipe (`fd[1]`) to ensure proper data flow.

- **Updating `fd_input`**:
  - `fd_input` is updated to the read end of the current pipe (`fd[0]`) so that it can be used as input for the next command.

- **Waiting for Child Process**:
  - The parent process waits for the child process to complete using `waitpid`.

- **Output Redirection (Optional)**:
  - If an `outfile` is specified (not `NULL`), it opens the specified output file for writing using the `open` system call and performs error handling.

- **Redirecting Output to File**:
  - It uses `dup2` to redirect the standard output of the last command in the pipeline to the specified output file descriptor (`STDOUT_FILENO`). This step ensures that the final output is saved to the designated file, if required.

- **Reading and Writing Data**:
  - The function reads any remaining output from the last command in the pipeline using `read` and writes it to the terminal or the specified output file, ensuring that all data is appropriately captured and displayed.

## Signal Handlers (`handle_sigint` and `handle_sigtstp`)
  Signal handlers are functions that deal with specific signals sent to the program. In this code, two signal handlers are defined:
  - **`handle_sigint`**: This handler manages the Ctrl+C (SIGINT) signal. It sets the **`flag`** variable to 0, effectively blocking the handling of the Ctrl+C signal. This allows users to interrupt long-running commands by pressing Ctrl+C, but it can be customized to handle interruption differently if needed.
  - **`handle_sigtstp`**: This handler manages the Ctrl+Z (SIGTSTP) signal in a similar way as **`handle_sigint`**. It sets the **`flag`** variable to 0, blocking the handling of Ctrl+Z. This signal is typically used to suspend a running process, and its behavior can be customized according to the desired functionality.
