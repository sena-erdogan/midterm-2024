#include "neHosLib.h"

Queue * queue;
Queue * wait_queue;
Queue * children;

sem_t log_sem;

char logName[FIFO_NAME_LEN];

char * get_timestamp() {
        time_t now = time(NULL);
        return asctime(localtime( & now));
}

void write_to_log(char * message) {

        sem_wait( & log_sem);
        int logfile = open(logName, WRITE_FLAGS, PERMS);
        if (logfile == -1) {
                perror("Failed to open serverFifo");
                sem_post( & log_sem);
                exit(EXIT_FAILURE);
        }

        char * time = get_timestamp();

        write(logfile, time, strlen(time));
        write(logfile, message, strlen(message));
        write(logfile, "\n", 1);
        if (close(logfile) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

        sem_post( & log_sem);
}

void handler(int signum) {

        int status;
        char log[200];

        log[0] = '\0';

        pid_t pid = getpid();

        if (signum == SIGINT) {
                Node * current = children -> front;
                while (current != NULL) {
                        kill(current -> data, SIGKILL);
                        printf("\n\nProcess with PID %d terminating\n\n", current -> data);
                        sprintf(log, "Process with PID %d terminating", current -> data);
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));
                        current = current -> next;
                }
                printf("\n\nAll child processes killed. Exiting...\n\n");
                sprintf(log, "All child processes killed. Server exiting...");
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                exit(1);
        }

        while ((pid = waitpid(-1, & status, WNOHANG | WUNTRACED)) > 0) {
                if (WIFEXITED(status)) {
                        printf("Process with PID %d exited normally with status %d\n", pid, WEXITSTATUS(status));
                        sprintf(log, "Process with PID %d exited normally with status %d\n", pid, WEXITSTATUS(status));
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));
                } else if (WIFSIGNALED(status)) {
                        printf("Process with PID %d terminated by signal %d\n", pid, WTERMSIG(status));
                        sprintf(log, "Process with PID %d terminated by signal %d\n", pid, WTERMSIG(status));
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));
                } else if (WIFSTOPPED(status)) {
                        printf("Process with PID %d stopped by signal %d\n", pid, WSTOPSIG(status));
                        sprintf(log, "Process with PID %d stopped by signal %d\n", pid, WSTOPSIG(status));
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));
                }
        }
}

int list(char * serverdir, int clientFifo) {

        char log[200];

        log[0] = '\0';

        sprintf(log, "Child %d: List of directory %s", getpid(), serverdir);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        int directory_lock = open(serverdir, O_RDONLY);
        if (directory_lock == -1) {
                perror("Unable to lock directory");
                return 1;
        }

        if (flock(directory_lock, LOCK_EX) == -1) {
                perror("Error locking directory");
                return 1;
        }

        DIR * directory;
        struct dirent * entry;

        directory = opendir(serverdir);

        if (directory == NULL) {
                perror("Unable to open directory");
                return 1;
        }

        while ((entry = readdir(directory)) != NULL) {
                if (strcmp(entry -> d_name, ".") == 0 || strcmp(entry -> d_name, "..") == 0) {
                        continue;
                }
                if (write(clientFifo, entry -> d_name, strlen(entry -> d_name)) == -1) {
                        perror("Failed to write to clientFifo");
                        exit(EXIT_FAILURE);
                }
                write(clientFifo, "\n", strlen("\n"));
        }

        closedir(directory);

        if (flock(directory_lock, LOCK_UN) == -1) {
                perror("Error unlocking directory");
                return 1;
        }

        close(directory_lock);

        sprintf(log, "Child %d: List of directory %s was SUCCESSFUL", getpid(), serverdir);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        return 0;

}

void readF(char * serverdir, int clientFifo, char * buffer) {

        char * token;
        char lineNumber[4];
        char fileName[50];
        char fileAddress[100];

        char log[200];

        log[0] = '\0';

        token = strtok(buffer, " ");

        token = strtok(NULL, " ");
        token = strtok(NULL, " ");
        strcpy(fileName, token);

        token = strtok(NULL, " ");
        if (token != NULL) {
                strcpy(lineNumber, token);
        }

        strcpy(fileAddress, serverdir);
        strcat(fileAddress, "/");
        strcat(fileAddress, fileName);

        sprintf(log, "Child %d: readF for %s", getpid(), fileAddress);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        FILE * file = fopen(fileAddress, "r");
        if (file == NULL) {
                perror("Error opening file");
                sprintf(log, "Child %d: ReadF of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                return;
        }

        int fd = fileno(file);
        if (fd == -1) {
                perror("Error obtaining file descriptor");
                fclose(file);
                sprintf(log, "Child %d: ReadF of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                return;
        }

        if (flock(fd, LOCK_EX) == -1) {
                perror("Error locking file");
                fclose(file);
                sprintf(log, "Child %d: ReadF of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                return;
        }

        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        int line_number = 0;
        int readFlag = 0;

        if (strcmp(lineNumber, "0") == 0) {
                readFlag = 1;
                while ((read = getline( & line, & len, file)) != -1) {
                        if (write(clientFifo, line, strlen(line)) == -1) {
                                perror("Failed to write to clientFifo");
                                sprintf(log, "Child %d: ReadF of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                                write_to_log(log);
                                memset(log, '\0', sizeof(log));
                                exit(EXIT_FAILURE);
                        }

                        memset(line, '\0', strlen(line));
                }
        } else {
                while ((read = getline( & line, & len, file)) != -1) {
                        line_number++;
                        if (line_number == atoi(lineNumber)) {
                                readFlag = 1;
                                int oyku = write(clientFifo, line, strlen(line));
                                if (oyku > 0) {
                                        write(STDOUT_FILENO, "\n", sizeof("\n"));
                                        write(STDOUT_FILENO, line, strlen(line));
                                        write(STDOUT_FILENO, "\n", sizeof("\n"));
                                } else {
                                        perror("Failed to write to clientFifo");
                                        sprintf(log, "Child %d: ReadF of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                                        write_to_log(log);
                                        memset(log, '\0', sizeof(log));
                                        exit(EXIT_FAILURE);
                                }
                                break;
                        }
                }
        }

        if (readFlag == 0) {

                if (fseek(file, 0, SEEK_SET) == -1) {
                        perror("Failed to rewind file pointer");
                        sprintf(log, "Child %d: ReadF of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));
                        exit(EXIT_FAILURE);
                }

                while ((read = getline( & line, & len, file)) != -1) {
                        if (write(clientFifo, line, strlen(line)) == -1) {
                                perror("Failed to write to clientFifo");
                                sprintf(log, "Child %d: ReadF of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                                write_to_log(log);
                                memset(log, '\0', sizeof(log));
                                exit(EXIT_FAILURE);
                        }

                        memset(line, '\0', strlen(line));
                }
        }

        flock(fd, LOCK_UN);

        fclose(file);

        sprintf(log, "Child %d: ReadF of file %s was SUCCESSFUL", getpid(), fileAddress);
        write_to_log(log);
        memset(log, '\0', sizeof(log));
        memset(fileAddress, '\0', strlen(fileAddress));

}

void writeT(char * serverdir, int clientFifo, char * buffer) {

        char * token;
        char lineNumber[4];
        char fileName[50];
        char fileAddress[100];
        char stringToWrite[100];
        char log[200];

        log[0] = '\0';

        token = strtok(buffer, " ");

        token = strtok(NULL, " ");
        token = strtok(NULL, " ");
        strcpy(fileName, token);

        token = strtok(NULL, " ");
        if (token != NULL) {
                if (isdigit( * token)) {
                        strcpy(lineNumber, token);
                        token = strtok(NULL, "");
                        if (token != NULL) {
                                strcpy(stringToWrite, token);
                        } else {
                                strcpy(stringToWrite, "");
                        }
                } else {
                        strcpy(lineNumber, "0");
                        strcpy(stringToWrite, token);
                        token = strtok(NULL, "\n");
                        strcat(stringToWrite, " ");
                        strcat(stringToWrite, token);
                }
        }

        strcpy(fileAddress, serverdir);
        strcat(fileAddress, "/");
        strcat(fileAddress, fileName);

        sprintf(log, "Child %d: writeT for %s", getpid(), fileAddress);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        FILE * file = fopen(fileAddress, "rb+");
        if (file == NULL) {
                perror("Error opening file");
                sprintf(log, "Child %d: WriteT of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                return;
        }

        int fd = fileno(file);
        if (fd == -1) {
                perror("Error obtaining file descriptor");
                fclose(file);
                sprintf(log, "Child %d: WriteT of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                return;
        }

        if (flock(fd, LOCK_EX) == -1) {
                perror("Error locking file");
                fclose(file);
                sprintf(log, "Child %d: WriteT of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                return;
        }
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        int line_number = 0;
        int writeFlag = 0;

        if (atoi(lineNumber) != 0) {

                while ((read = getline( & line, & len, file)) != -1) {
                        line_number++;
                        if (line_number == atoi(lineNumber) - 1) {
                                fprintf(file, "%s", stringToWrite);
                                writeFlag = 1;
                                break;
                        }
                }
        }

        if (writeFlag == 0 || atoi(lineNumber) == 0) {
                if (writeFlag == 0) {
                        printf("Line %d not found in file %s\nWriting to the end of the file.\n\n", atoi(lineNumber), fileAddress);
                }
                if (fseek(file, 0, SEEK_END) == -1) {
                        if (write(clientFifo, "0\n", sizeof("0\n")) == -1) {
                                perror("Failed to write to clientFifo");
                                sprintf(log, "Child %d: WriteT of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                                write_to_log(log);
                                memset(log, '\0', sizeof(log));
                                exit(EXIT_FAILURE);
                        }
                        fclose(file);
                        sprintf(log, "Child %d: WriteT of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));
                        return;
                }
                fprintf(file, "%s\n", stringToWrite);

        }

        if (write(clientFifo, "1\n", sizeof("1\n")) == -1) {
                perror("Failed to write to clientFifo");
                exit(EXIT_FAILURE);
        }

        flock(fd, LOCK_UN);

        fclose(file);

        sprintf(log, "Child %d: WriteT of file %s was SUCCESSFUL", getpid(), fileAddress);
        write_to_log(log);
        memset(log, '\0', sizeof(log));
        memset(fileAddress, '\0', strlen(fileAddress));

}

int upload(char * serverdir, char * clientFifoName, char * buffer) {

        char * token;
        char fileName[50];
        char fileAddress[100];
        char input[100];
        char log[200];

        log[0] = '\0';

        sprintf(log, "Child %d: Upload for %s", getpid(), clientFifoName);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        token = strtok(buffer, " ");

        token = strtok(NULL, " ");
        token = strtok(NULL, " ");
        strcpy(fileName, token);

        strcpy(fileAddress, serverdir);
        strcat(fileAddress, "/");
        strcat(fileAddress, fileName);

        printf("\nfileAddress: %s\n", fileAddress);

        int fd = open(fileAddress, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd == -1) {
                perror("Error opening file");
                sprintf(log, "Child %d: Upload of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                return EXIT_FAILURE;
        }

        if (flock(fd, LOCK_EX) == -1) {
                perror("Error locking file");
                close(fd);
                sprintf(log, "Child %d: Upload of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                return -1;
        }

        int clientFifo = open(clientFifoName, READ_FLAGS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                sprintf(log, "Child %d: Upload of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                exit(EXIT_FAILURE);
        }

        memset(input, '\0', sizeof(input));

        ssize_t bytes_written;

        while (1) {
                ssize_t bytes_read = read(clientFifo, input, sizeof(input) - 1);
                if (bytes_read == -1) {
                        perror("Failed to read from clientFifo");
                        sprintf(log, "Child %d: Upload of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));
                        exit(EXIT_FAILURE);
                } else if (bytes_read == 0) {
                        break;
                }

                input[bytes_read] = '\0';

                bytes_written = write(fd, input, sizeof(input));
                if (bytes_written == -1) {
                        perror("Error writing to file");
                        close(fd);
                        sprintf(log, "Child %d: Upload of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));
                        return EXIT_FAILURE;
                }

                bytes_written = write(fd, "\n", sizeof("\n"));
                if (bytes_written == -1) {
                        perror("Error writing to file");
                        close(fd);
                        sprintf(log, "Child %d: Upload of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));
                        return EXIT_FAILURE;
                }

                memset(input, '\0', sizeof(input));
        }

        flock(fd, LOCK_UN);

        close(fd);

        sprintf(log, "Child %d: Upload of file %s was SUCCESSFUL", getpid(), fileAddress);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

        memset(fileAddress, '\0', strlen(fileAddress));
        
        return 0;

}

void download(char * serverdir, int clientFifo, char * buffer) {

        char * token;
        char fileName[50];
        char fileAddress[100];

        char log[200];

        log[0] = '\0';

        sprintf(log, "Child %d: Download from server", getpid());
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        token = strtok(buffer, " ");

        token = strtok(NULL, " ");
        token = strtok(NULL, " ");
        strcpy(fileName, token);

        strcpy(fileAddress, serverdir);
        strcat(fileAddress, "/");
        strcat(fileAddress, fileName);

        FILE * file = fopen(fileAddress, "r");
        if (file == NULL) {
                perror("Error opening file");
                sprintf(log, "Child %d: Download of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                return;
        }

        int fd = fileno(file);
        if (fd == -1) {
                perror("Error obtaining file descriptor");
                fclose(file);
                sprintf(log, "Child %d: Download of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                return;
        }

        if (flock(fd, LOCK_EX) == -1) {
                perror("Error locking file");
                fclose(file);
                sprintf(log, "Child %d: Download of file %s was UNSUCCESSFUL", getpid(), fileAddress);
                write_to_log(log);
                memset(log, '\0', sizeof(log));
                return;
        }
        char * line = NULL;
        size_t len = 0;
        ssize_t read;

        while ((read = getline( & line, & len, file)) != -1) {
                if (write(clientFifo, line, strlen(line)) == -1) {
                        perror("Failed to write to clientFifo");
                        exit(EXIT_FAILURE);
                }
                memset(line, '\0', strlen(line));
        }
        flock(fd, LOCK_UN);

        fclose(file);

        sprintf(log, "Child %d: Download of file %s was SUCCESSFUL", getpid(), fileAddress);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        memset(fileAddress, '\0', strlen(fileAddress));

}

void archServer() {

}
void killServer(int serverPid, int serverFifo, char * serverFifoName, int clientFifo, char * clientFifoName) {

        char log[200];

        log[0] = '\0';

        sprintf(log, "Child %d: KillServer for %s", getpid(), clientFifoName);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        Node * current = queue -> front;
        while (current != NULL) {
                if (kill(current -> data, SIGKILL) == 0) {
                        sprintf(log, "Child %d: Kill signal sent to PID: %d", getpid(), current -> data);
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));
                } else {
                        perror("Failed to send kill signal");
                }
                current = current -> next;
        }

        current = wait_queue -> front;
        while (current != NULL) {
                if (kill(current -> data, SIGKILL) == 0) {
                        sprintf(log, "Child %d: Kill signal sent to PID: %d", getpid(), current -> data);
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));
                } else {
                        perror("Failed to send kill signal");
                }
                current = current -> next;
        }

        if (close(serverFifo) == -1) {
                perror("\nserverFifo close error\n");
                exit(EXIT_FAILURE);
        }

        if (unlink(serverFifoName) == -1) {
                exit(EXIT_FAILURE);
        }

        if (kill(serverPid, SIGTERM) == -1) {
                perror("Error sending signal");
                exit(EXIT_FAILURE);
        }

}

void quit(int clientFifo, char * buffer) {

        char * token;
        char pid_str[10];

        token = strtok(buffer, " ");

        token = strtok(NULL, " ");
        token = strtok(NULL, " ");
        strcpy(pid_str, token);

        int pid = atoi(pid_str);

        char log[200];

        log[0] = '\0';

        sprintf(log, "Child %d: Quit for %d", getpid(), pid);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        sprintf(log, "Child %d: Pid of quit client: %d", getpid(), pid);
        write_to_log(log);
        memset(log, '\0', sizeof(log));

        int ret = remove_value(queue, pid);

        if (ret == 1) {
                sprintf(log, "Child %d: Remove from queue was SUCCESSFUL", getpid());
                write_to_log(log);
                memset(log, '\0', sizeof(log));

        } else {
                sprintf(log, "Child %d: Remove from queue was UNSUCCESSFUL", getpid());
                write_to_log(log);
                memset(log, '\0', sizeof(log));
        }

        if (size(wait_queue) > 0) {

                push(queue, wait_queue -> front -> data);

                ret = pop(wait_queue);

                if (ret == 1) {
                        sprintf(log, "Child %d: Remove from wait_queue was SUCCESSFUL", getpid());
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));

                } else {
                        sprintf(log, "Child %d: Remove from wait_queue was UNSUCCESSFUL", getpid());
                        write_to_log(log);
                        memset(log, '\0', sizeof(log));
                }
        }

        if (write(clientFifo, "1\n", sizeof("1\n")) == -1) {
                perror("Failed to write to clientFifo");
                exit(EXIT_FAILURE);
        }
}

int fifoParse(char * buffer, char * serverdir, int serverFifo, char * serverFifoName, int clientFifo, char * clientFifoName, int serverPid) {

        char command[20];

        if (sscanf(buffer, "%s", command) != 1) {
                perror("Clientfifo command parse:");
                return 1;
        }

        if (strcmp(command, "req") != 0) {
                return 1;
        }

        if (sscanf(buffer, "%*s %s", command) != 1) {
                perror("Clientfifo command parse:");
                return 1;
        }

        if (strcmp(command, "list") == 0) {
                return list(serverdir, clientFifo);
        } else if (strcmp(command, "readF") == 0) {
                readF(serverdir, clientFifo, buffer);
        } else if (strcmp(command, "writeT") == 0) {
                writeT(serverdir, clientFifo, buffer);
        } else if (strcmp(command, "upload") == 0) {
                upload(serverdir, clientFifoName, buffer);
        } else if (strcmp(command, "download") == 0) {
                download(serverdir, clientFifo, buffer);
        } else if (strcmp(command, "archServer") == 0) {
                archServer();
        } else if (strcmp(command, "killServer") == 0) {
                killServer(serverPid, serverFifo, serverFifoName, clientFifo, clientFifoName);
        } else if (strcmp(command, "quit") == 0) {
                printf("\nCommand is quit\n");
                quit(clientFifo, buffer);
        }

        return 1;

}

int server(int serverPid, char * serverdir, int maxClient) {

        int clientFifo, serverFifo;
        char clientFifoName[FIFO_NAME_LEN];
        char serverFifoName[FIFO_NAME_LEN];
        char buffer[100];
        char control[3];
        int con;
        pid_t cli;

        printf("\nServer started PID %d\n", getpid());

        struct sigaction sa;
        memset( & sa, 0, sizeof(sa));
        sa.sa_handler = handler;
        sigaction(SIGINT, & sa, NULL);

        sprintf(serverFifoName, "%s%ld", SERVERFIFO, (long) getpid());

        printf("serverFifo is: %s\n", serverFifoName);

        umask(0);

        if (mkfifo(serverFifoName, PERMS) == -1 && errno != EEXIST) {
                perror("\nserverFifo Error\n");
                exit(EXIT_FAILURE);
        }

        serverFifo = open(serverFifoName, READ_FLAGS, PERMS);
        if (serverFifo == -1) {
                perror("Failed to open serverFifo");
                exit(EXIT_FAILURE);
        }

        memset(buffer, '\0', sizeof(buffer));
        while (1) {

                while (read(serverFifo, buffer, sizeof(buffer)) <= 0);

                char input[200];
                input[0] = '\0';
                sprintf(input, "Read from serverfifo: %s", buffer);
                write_to_log(input);

                if (sscanf(buffer, "%3s", control) != 1) {
                        perror("Error parsing input");
                        exit(EXIT_FAILURE);
                }

                if (strcmp(control, "pid") == 0) {
                        if (sscanf(buffer, "%*s %d", & cli) != 1) {
                                perror("Input read from serverfifo is not valid");

                                exit(EXIT_FAILURE);
                        }

                        if (sscanf(buffer, "%*s %*s %s %d", control, & con) != 2) {
                                perror("Input read from serverfifo is not valid");

                                exit(EXIT_FAILURE);
                        }

                        sprintf(clientFifoName, "%s%ld", CLIENTFIFO, (long) cli);

                        memset(input, '\0', sizeof(input));
                        sprintf(input, "ClientFifo is: %s", clientFifoName);
                        write_to_log(input);

                        memset(buffer, '\0', sizeof(buffer));

                        umask(0);

                        if (mkfifo(clientFifoName, PERMS) == -1 && errno != EEXIST) {
                                perror("\nclientFifo Error\n");
                                exit(EXIT_FAILURE);
                        }

                        clientFifo = open(clientFifoName, WRITE_FLAGS, PERMS);
                        if (clientFifo == -1) {
                                perror("Failed to open clientFifo");
                                exit(EXIT_FAILURE);
                        }

                        if (con == 0) {
                                if (size(queue) < maxClient) {
                                        push(queue, cli);

                                        memset(input, '\0', sizeof(input));
                                        sprintf(input, "Client %d is pushed to the queue", cli);
                                        write_to_log(input);

                                        if (write(clientFifo, "con 1", strlen("con 1")) == -1) {
                                                perror("Failed to write to clientFifo");
                                                exit(EXIT_FAILURE);
                                        }
                                } else {
                                        memset(input, '\0', sizeof(input));
                                        sprintf(input, "Queue is full, client %d terminating", cli);
                                        write_to_log(input);
                                }

                                memset(input, '\0', sizeof(input));
                                sprintf(input, "Queue size: %d", size(queue));
                                write_to_log(input);
                                memset(input, '\0', sizeof(input));
                                sprintf(input, "Waiting queue size: %d", size(wait_queue));
                                write_to_log(input);

                        } else if (con == 1) {
                                if (size(queue) < maxClient) {
                                        push(queue, cli);

                                        memset(input, '\0', sizeof(input));
                                        sprintf(input, "Client %d is pushed to the queue", cli);
                                        write_to_log(input);

                                        if (write(clientFifo, "con 1", strlen("con 1")) == -1) {
                                                perror("Failed to write to clientFifo");
                                                exit(EXIT_FAILURE);
                                        }

                                        if (close(clientFifo) == -1) {
                                                perror("\nclientFifo close error\n");
                                                exit(EXIT_FAILURE);
                                        }

                                } else {
                                        push(wait_queue, cli);
                                        memset(input, '\0', sizeof(input));
                                        sprintf(input, "Client %d is pushed to the waiting queue", cli);
                                        write_to_log(input);
                                }

                                memset(input, '\0', sizeof(input));
                                sprintf(input, "Queue size: %d", size(queue));
                                write_to_log(input);
                                memset(input, '\0', sizeof(input));
                                sprintf(input, "Waiting queue size: %d", size(wait_queue));
                                write_to_log(input);

                        } else {
                                perror("Input read from serverfifo is not valid");

                                exit(EXIT_FAILURE);
                        }
                } else {
                        continue;
                }

                if (size(queue) < maxClient) {

                        pid_t childPid = fork();
                        if (childPid == 0) {
                                push(children, getpid());

                                while (1) {

                                        memset(input, '\0', sizeof(input));
                                        sprintf(input, "Child %d is responsible for %s", getpid(), clientFifoName);
                                        write_to_log(input);

                                        memset(buffer, '\0', sizeof(buffer));

                                        clientFifo = open(clientFifoName, READ_FLAGS, PERMS);
                                        if (clientFifo == -1) {
                                                perror("Failed to open clientFifo");
                                                exit(EXIT_FAILURE);
                                        }

                                        while (read(clientFifo, buffer, sizeof(buffer)) <= 0);

                                        memset(input, '\0', sizeof(input));
                                        sprintf(input, "Read from clientfifo %s: %s", clientFifoName, buffer);
                                        write_to_log(input);

                                        if (close(clientFifo) == -1) {
                                                perror("\nclientFifo close error\n");
                                                exit(EXIT_FAILURE);
                                        }

                                        clientFifo = open(clientFifoName, WRITE_FLAGS, PERMS);
                                        if (clientFifo == -1) {
                                                perror("Failed to open clientFifo");
                                                exit(EXIT_FAILURE);
                                        }

                                        fifoParse(buffer, serverdir, serverFifo, serverFifoName, clientFifo, clientFifoName, serverPid);

                                        if (close(clientFifo) == -1) {
                                                perror("\nclientFifo close error\n");
                                                exit(EXIT_FAILURE);
                                        }
                                }

                        } else if (childPid > 0) {
                                printf("\nServer parent\n");

                        } else {
                                perror("Fork failed");
                                exit(EXIT_FAILURE);
                        }
                }

                memset(buffer, '\0', sizeof(buffer));

        }

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

        if (unlink(clientFifoName) == -1) {
                exit(EXIT_FAILURE);
        }

        if (close(serverFifo) == -1) {
                perror("\nserverFifo close error\n");
                exit(EXIT_FAILURE);
        }

        if (unlink(serverFifoName) == -1) {
                exit(EXIT_FAILURE);
        }

        return 0;
}

int main(int argc, char * argv[]) {

        setbuf(stdout, NULL);

        char * serverdir;
        int maxclient;

        if (argc != 3) {
                perror("\nFormat: ./neHosServer <dirname> <max. #ofClients>");
                return 1;
        }

        serverdir = (char * ) malloc(strlen(argv[1]) + 4);

        if (serverdir == NULL) {
                perror("Memory allocation failed");
                return 1;
        }

        strcat(serverdir, argv[1]);

        if (argv[2]) {
                maxclient = atoi(argv[2]);
                if (maxclient < 1) {
                        perror("Max number of clients must be greater than 0");
                        return 1;
                }
        }

        sprintf(logName, LOG_FILE, (long) getpid());

        DIR * directory = opendir(serverdir);
        if (directory) {
                closedir(directory);
        } else {
                if (mkdir(serverdir, 0777) == -1) {
                        perror("Error creating directory");
                        exit(EXIT_FAILURE);
                }
        }

        queue = create();
        wait_queue = create();
        children = create();

        FILE * logFile = fopen(logName, "a");
        if (logFile == NULL) {
                perror("Failed to open log file");
                return 1;
        }

        fclose(logFile);

        if (sem_init( & log_sem, 0, 1) != 0) {
                perror("Semaphore initialization failed");
                return 1;
        }

        char input[150];
        input[0] = '\0';
        sprintf(input, "Server started with PID %d, logfile %s, directory %s, and max number of clients %d", getpid(), logName, serverdir, maxclient);

        write_to_log(input);

        server(getpid(), serverdir, maxclient);

        sem_destroy( & log_sem);

        return 0;

}
