/********************************************************************************************
This is a template for assignment on writing a custom Shell. 

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Though use of any extra functions is not recommended, students may use new functions if they need to, 
but that should not make code unnecessorily complex to read.

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Students need to be careful while forking a new process (no unnecessory process creations) 
or while inserting the single handler code (should be added at the correct places).

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp, 
as you not need to use any features for this assignment that are supported by C++ but not by C).
*********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()

// Signal handlers
int flag = 1;
void handle_sigint(int signo)
{
    //   printf("\nCaught SIGINT\n");
    flag = 0;
}

void handle_sigtstp(int signo)
{
    //   printf("\nCaught SIGTSTP\n");
    flag = 0;
}

void parseInput(char* input, char *commands[])
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
    char delimit[] = " ";                                    
    int i = 0;
    char* inputPtr = input;
    char* outputPtr = input;
    while((outputPtr = strsep(&inputPtr, delimit)) != NULL)
    {
        commands[i++] = outputPtr;
    }

    commands[i] = NULL;
}

void executeCommand(char *commands[])
{
	// This function will fork a new process to execute a command

    //For cd command  
    if(strcmp(commands[0], "cd") == 0)
    {
        if(commands[1] != NULL)
        {
            int error = chdir(commands[1]);
            if(error < 0)
            {
                printf("Shell: Incorrect command\n"); 
            }
        }
        else
        {
            int error = chdir("/home/shoaib");
            if(error < 0)
            {
                printf("Shell: Incorrect command\n"); 
            }
        }
    }

    //For ls command & others
    else 
    {
        pid_t pid;    //process id
        pid = fork(); //create another process
        if(pid == 0)  //child
        {
            /* Signal handling */
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if(execvp(commands[0], commands) < 0)
            {
                printf("Shell: Incorrect command\n"); 
                exit(-1);
            }
            exit(0);
        }
        if (pid < 0)    // fail
        {
            printf("Shell: Incorrect command\n");                                 
            exit (-1);
        }
        else  //parent
        {
            wait(NULL);  //wait for child
        }
    }
}

void executeParallelCommands(char **commands)
{
	// This function will run multiple commands in parallel

    char *parsed[25][25];
    int cmdCount = 0;
    int argIndex = 0;

    // Parse input into separate commands
    for(int i = 0; i < 256; i++)
    {
        if(commands[i] == NULL)
        {
            break;
        }
        if(strcmp(commands[i], "&&") == 0)
        {
            parsed[cmdCount][argIndex] = NULL;
            cmdCount++;
            argIndex = 0;
        }
        else
        {
            parsed[cmdCount][argIndex] = commands[i];
            argIndex++;
        }
    }

    parsed[cmdCount][argIndex] = NULL;

    // Fork and execute each command
    for (int i = 0; i < cmdCount + 1; i++) 
    {
        pid_t pid = fork();

        if(pid < 0)
        {
            printf("Shell: Incorrect command\n"); 
            exit(1);
        }
        else if (pid == 0) 
        {   // Child process
            /* Signal handling */
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if(execvp(parsed[i][0], parsed[i]) < 0)
            {
                printf("Shell: Incorrect command\n"); 
                exit(1);
            }
            exit(0);
        }
    }

    // Wait for all child processes
    int status;
    while (cmdCount + 1 > 0) 
    {
        wait(&status);
        cmdCount--;
    }
}

void executeSequentialCommands(char **commands)
{	
	// This function will run multiple commands in sequence
    char **tempCommands = commands;
    int i = 0;
    while(i < 256)
    {
        if(tempCommands[i] == NULL)
        {
            break;
        }
        char *temp[10];
        for(int i = 0; i < 10; i++)
        {
            temp[i] = NULL;
        }
        int j = 0;
        while((tempCommands[i] != NULL) && (strcmp(tempCommands[i], "##") != 0))
        {
            temp[j] = tempCommands[i];
            i++;
            j++;             
        }
        temp[j] = NULL;
        executeCommand(temp);
        i++;
    }        
}

void executeCommandRedirection(char **commands)
{
	// This function will run a single command with output redirected to an output file specificed by user
    
    char **tempCommands = commands;
    if(strlen(tempCommands[2]) == 0)
    {
        return;
    }
    tempCommands[1] = NULL;
    pid_t pid = fork();
    if(pid == 0)
    {
        /*Signal handling */
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        int fd = open(tempCommands[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(fd, STDOUT_FILENO);
        close(fd);

        if(execvp(tempCommands[0], commands) < 0)
        {
            printf("Shell: Incorrect command\n"); 
        }
        exit(0);
    }
    else if(pid < 0)
    {
        printf("Shell: Incorrect command\n"); 
        return;
    }
    else
    {
        wait(NULL);
    }
}

void pipe_execute(char **commands, int n_commands, char *outfile) 
{
    // Variables
    int fd_input, fd_output;  // File descriptors for input and output
    int fd[2];  // Array of file descriptors for the pipe
    pid_t pid;  // Process ID of the child process
    char buffer[1024];
    int bytes_read;

    fd_input = 0;  // Initial input will come from stdin

    // Loop through each command in pipeline
    for (int i = 0; i < n_commands; i++) 
    {
        // Pipe Error Handling
        if (pipe(fd) < 0) {  // Create pipe
            perror("pipe error");
            exit(EXIT_FAILURE);
        }

        pid = fork();  // Create child process
        // Fork Error Handling
        if (pid < 0) 
        {
            perror("fork error");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) // Child process
        {
            close(fd[0]);  // Close read end of pipe

            printf("%d\n", fd_input);

            // Redirect input to previous pipe read end
            if (fd_input != 0) 
            {
                // dup2 Error Handling for input end
                if (dup2(fd_input, STDIN_FILENO) < 0) 
                {
                    perror("dup2 error");
                    exit(EXIT_FAILURE);
                }
                close(fd_input);
            }

            printf("%d\n", fd[1]);

            // Redirect output to current pipe write end
            if (fd[1] != 1) 
            {
                // dup2 Error Handling for input end
                if (dup2(fd[1], STDOUT_FILENO) < 0) 
                {
                    perror("dup2 error");
                    exit(EXIT_FAILURE);
                }
                close(fd[1]);
            }

            // Execute command
            char *args[10];
            int n_args = 0;
            char *token = strtok(commands[i], " ");

            while (token != NULL && n_args < 10) 
            {
                args[n_args++] = token;
                token = strtok(NULL, " ");
            }
            args[n_args] = NULL;

            if (execvp(args[0], args) < 0) // Execute command
            {
                perror("execvp error");
                exit(EXIT_FAILURE);
            }
        }

        close(fd[1]);  // Close write end of pipe

        fd_input = fd[0];  // Set input for next command to current pipe read end
        waitpid(pid, NULL, 0);  // Wait for child process to complete
    }

    // Redirect output to file if specified
    if (outfile != NULL) 
    {
        fd_output = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        
        // Output Error Handling
        if (fd_output < 0) 
        {  
            perror("open error");
            exit(EXIT_FAILURE);
        }

        // dup2 Error Handling for output end
        if (dup2(fd_output, STDOUT_FILENO) < 0) 
        {
            perror("dup2 error");
            exit(EXIT_FAILURE);
        }

        close(fd_output);
    }

    // Read remaining output from last command in pipeline
    while ((bytes_read = read(fd_input, buffer, sizeof(buffer))) > 0) 
    {
        write(STDOUT_FILENO, buffer, bytes_read);
    }

    close(fd_input);
}

void executePipedCommands(char *command) 
{
    char *commands[10];    // Array of command arguments
    char *outfile = NULL;    // Output file name, if specified
    int n_commands = 0;          // Number of individual commands

    // Check if output file is specified
    if (strstr(command, ">") != NULL) 
    {
        outfile = strtok(command, ">"); // Get the part of the command before the ">" character
        outfile = strtok(NULL, " "); // Get the next part of the command, which should be the output file name
        outfile[strcspn(outfile, " ")] = '\0'; // Remove the trailing whitespace from the output file name
    }

    // Split command into individual commands using the "|" character as the delimiter
    char *token = strtok(command, "|");
    while (token != NULL && n_commands < 10) // As long as there are still tokens left and we haven't reached the maximum number of arguments
    {
        commands[n_commands++] = token; // Add the current token to the array of command arguments
        token = strtok(NULL, "|"); // Get the next token
    }

    // If there is at least one command, execute the pipeline
    if (n_commands > 0) 
    {
        pipe_execute(commands, n_commands, outfile); // This is a placeholder function call that presumably executes the pipeline of commands
    }
}

int main()
{
	// Initial declarations
    signal(SIGINT, handle_sigint);  /* Ignore signal interrupt (Ctrl+C) */
    signal(SIGTSTP, handle_sigtstp); /* Ignore signal of suspending execution (Ctrl+Z) */
	
	while(1)	// This loop will keep your shell running until user exits.
	{
		// Print the prompt in format - currentWorkingDirectory$
        char cwd[256];
        if(getcwd(cwd, sizeof(cwd)) == NULL)
        {
            perror("getcwd() error");
            exit(1);
        }
        else
        {
            printf("%s$", cwd);
        }
        		
		// accept input with 'getline()'    
        char* input;
        size_t inputSize = 32;
        size_t characters;

        input = (char*)malloc(sizeof(char) * inputSize);
        if( input == NULL)
        {
            perror("Unable to allocate memory");
            exit(1);
        }
        
        characters = getline(&input, &inputSize, stdin);
        if (characters == -1)
        {
            perror("Error");
            exit(1);
        }

        if(characters > 0 && input[characters-1] == '\n') {
            input[characters-1] = '\0';
            characters--;
            // special care for windows line endings:
            if(characters > 0 && input[characters-1] == '\r') 
            {
                input[characters-1] = '\0';
                characters--;
            }
        }

		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.

        char *commands[256];
        for(int i = 0; i < 256; i++)
        {
            commands[i] = NULL;
        }

        char command[100];       //for pipes
        strcpy(command, input);
        if(command == NULL)
        {
            continue;
        }
        
		parseInput(input, commands); 		
		
		if(strcmp(commands[0], "exit") == 0)	// When user uses exit command.
		{
            printf("Exiting shell...\n");
			break;
		}

        int parallel = 0;
        int sequential = 0;
        int redirection = 0;
        int isPipe = 0;

        for(int i = 0; i < 256; i++)
        {
            if(commands[i] == NULL)
            {
                break;
            }
            if(strcmp(commands[i], "&&") == 0)
            {
                parallel = 1;
                break;
            }
            else if(strcmp(commands[i], "##") == 0)
            {
                sequential = 1;
                break;
            }
            else if(strcmp(commands[i], ">") == 0)
            {
                redirection = 1;
                break;
            }
            else if(strcmp(commands[i], "|") == 0)
            {
                isPipe = 1;
                break;
            }
        }

        if(flag == 0)
        {
            flag = 1;
            continue;
        }
        		
		if(parallel)
			executeParallelCommands(commands);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		else if(sequential)
			executeSequentialCommands(commands);	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		else if(redirection)
			executeCommandRedirection(commands);	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
		else if(isPipe)
            executePipedCommands(command);		// This function is invoked when user wants to run multiple commands using piping
        else
			executeCommand(commands);		// This function is invoked when user wants to run a single commands
				
	}
	
	return 0;
}