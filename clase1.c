#include <stdio.h>
#include <unistd.h>

int main() {

    //FORK
    int pid = fork(); // Create a new process

    if (pid == 0) {
        // This block runs in the child process
        printf("I am the child process with PID %d\n", getpid());
    } else {
        // This block runs in the parent process
        printf("I am the parent process with PID %d\n", getpid());
        printf("My child process has PID %d\n", pid);
    }


    //PIPE
    int pipe_fds[2];
    pipe(pipe_fds); // Create a pipe

    int pid = fork(); // Fork a new process

    if (pid == 0) {
        // Child process
        close(pipe_fds[1]); // Close the write end of the pipe
        char buffer[100];
        read(pipe_fds[0], buffer, sizeof(buffer)); // Read data from the pipe
        printf("Child process received: %s\n", buffer);
        close(pipe_fds[0]); // Close the read end of the pipe
    } else {
        // Parent process
        close(pipe_fds[0]); // Close the read end of the pipe
        char *message = "Hello from parent!";
        write(pipe_fds[1], message, sizeof(message)); // Write data to the pipe
        close(pipe_fds[1]); // Close the write end of the pipe
    }

    //What happens:
    // A pipe is created using pipe(pipe_fds). This creates two file descriptors: one for reading (pipe_fds[0]) and one for writing (pipe_fds[1]).
    // After the fork:
    // The child process closes the write end of the pipe and reads from the pipe.
    // The parent process closes the read end of the pipe and writes to the pipe.
    // The child process receives and prints the message sent by the parent.

    exit(0);
}
