#include "neHosLib.h"

void handler(int signum) {

        int status;
        pid_t pid = getpid();

        if (signum == SIGINT) {
                printf("\n\nProcess with PID %d terminating\n\n", pid);
                exit(1);
        }

        while ((pid = waitpid(-1, & status, WNOHANG | WUNTRACED)) > 0) {
                if (WIFEXITED(status)) {
                        printf("Process with PID %d exited normally with status %d\n", pid, WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                        printf("Process with PID %d terminated by signal %d\n", pid, WTERMSIG(status));
                } else if (WIFSTOPPED(status)) {
                        printf("Process with PID %d stopped by signal %d\n", pid, WSTOPSIG(status));
                }
        }
}

void help_list() {
        printf("\nlist");
        printf("\nsends a request to display the list of files in Servers directory(also displays the list received from the Server)\n");
}

void help_readF() {
        printf("\nreadF <file> <line #>");
        printf("\nrequests to display the # line of the <file>, if no line number is given  the whole contents of the file is requested (and displayed on the client side) \n");
}

void help_writeT() {
        printf("\nwriteT <file> <line #> <string>");
        printf("\nrequest to write the  content of “string” to the  #th  line the <file>, if the line # is not given writes to the end of file. If the file does not exists in Servers directory creates and edits the file at the same time\n");
}

void help_upload() {
        printf("\nupload <file>");
        printf("\nuploads the file from the current working directory of client to the Servers directory\n");
}

void help_download() {
        printf("\ndownload <file>");
        printf("\nrequest to receive <file> from Servers directory to client side\n");
}

void help_archServer() {
        printf("\narchServer <fileName>.tar");
        printf("\nUsing fork, exec and tar utilities create a child process that will collect all the files currently available on the Server side and store them in the <filename>.tar archive\n");
}

void help_killServer() {
        printf("\nkillServer");
        printf("\nSends a kill request to the Server\n");
}

void help_quit() {
        printf("\nquit");
        printf("\nSend write request to Server side log file and quits \n");
}

void help() {
        printf("\n\nList of Requests:\n\n");
        help_list();
        help_readF();
        help_writeT();
        help_upload();
        help_download();
        help_archServer();
        help_killServer();
        help_quit();
}

int isnum(char * lineNumber) {
        while ( * lineNumber != '\0') {
                if (!isdigit( * lineNumber)) {
                        return 0;
                }

                lineNumber++;
        }
        return 1;
}

void list(char * clientFifoName) {

        int clientFifo;

        umask(0);

        clientFifo = open(clientFifoName, WRITE_FLAGS, PERMS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        if (write(clientFifo, "req list", strlen("req list")) == -1) {
                perror("Failed to write to clientFifo");
                exit(EXIT_FAILURE);
        }

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }
        
        clientFifo = open(clientFifoName, READ_FLAGS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        char input[100];
        input[0] = '\0';
        
        printf("\n");

        while (1) {
                ssize_t bytes_read = read(clientFifo, input, sizeof(input) - 1);
                if (bytes_read == -1) {
                        perror("Failed to read from clientFifo");
                        exit(EXIT_FAILURE);
                } else if (bytes_read == 0) {
                        break;
                }

                input[bytes_read] = '\0';

                printf("%s", input);

                memset(input, '\0', sizeof(input));
        }
        
        printf("\n");

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

}

void readF(char * clientFifoName, char * fileName, char * lineNumber) {

        int clientFifo;

        umask(0);

        clientFifo = open(clientFifoName, WRITE_FLAGS, PERMS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        char input[100];

        input[0] = '\0';

        strcat(input, "req readF ");
        strcat(input, fileName);
        strcat(input, " ");
        strcat(input, lineNumber);

        if (write(clientFifo, input, strlen(input)) == -1) {
                perror("Failed to write to clientFifo");
                exit(EXIT_FAILURE);
        }

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

        clientFifo = open(clientFifoName, READ_FLAGS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        memset(input, '\0', sizeof(input));
        
        printf("\n");

        while (1) {
                ssize_t bytes_read = read(clientFifo, input, sizeof(input) - 1);
                if (bytes_read == -1) {
                        perror("Failed to read from clientFifo");
                        exit(EXIT_FAILURE);
                } else if (bytes_read == 0) {
                        break;
                }

                input[bytes_read] = '\0';

                printf("%s", input);

                memset(input, '\0', sizeof(input));
        }
        
        printf("\n");

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

}

void writeT(char * clientFifoName, char * fileName, char * lineNumber, char * stringToWrite) {

        int clientFifo;

        umask(0);

        clientFifo = open(clientFifoName, WRITE_FLAGS, PERMS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        char input[100];

        input[0] = '\0';

        strcat(input, "req writeT ");
        strcat(input, fileName);
        strcat(input, " ");
        strcat(input, lineNumber);
        strcat(input, " ");
        strcat(input, stringToWrite);

        if (write(clientFifo, input, strlen(input)) == -1) {
                perror("Failed to write to clientFifo");
                exit(EXIT_FAILURE);
        }

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

        clientFifo = open(clientFifoName, READ_FLAGS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        memset(input, '\0', sizeof(input));

        while (1) {
                ssize_t bytes_read = read(clientFifo, input, sizeof(input) - 1);
                if (bytes_read == -1) {
                        perror("Failed to read from clientFifo");
                        exit(EXIT_FAILURE);
                } else if (bytes_read == 0) {
                        break;
                }

                input[bytes_read] = '\0';

                if (input[0] == '1') {
                        printf("\nString written to file succesfully\n");
                } else {
                        printf("\nString couldn't be written to file\n");
                }

                memset(input, '\0', sizeof(input));
        }

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

}

void upload(char * clientFifoName, char * fileName) {

        int clientFifo;

        umask(0);

        clientFifo = open(clientFifoName, WRITE_FLAGS, PERMS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        char input[100];

        input[0] = '\0';

        strcat(input, "req upload ");
        strcat(input, fileName);

        if (write(clientFifo, input, strlen(input)) == -1) {
                perror("Failed to write to clientFifo");
                exit(EXIT_FAILURE);
        }

        FILE * file = fopen(fileName, "r");
        if (file == NULL) {
                perror("Error opening file");
                return;
        }

        int fd = fileno(file);
        if (fd == -1) {
                perror("Error obtaining file descriptor");
                fclose(file);
                return;
        }

        if (flock(fd, LOCK_EX) == -1) {
                perror("Error locking file");
                fclose(file);
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

        memset(input, '\0', sizeof(input));

}

int download(char * clientFifoName, char * fileName) {

        int clientFifo;

        umask(0);

        clientFifo = open(clientFifoName, WRITE_FLAGS, PERMS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        char input[100];

        input[0] = '\0';

        strcat(input, "req download ");
        strcat(input, fileName);

        if (write(clientFifo, input, strlen(input)) == -1) {
                perror("Failed to write to clientFifo");
                exit(EXIT_FAILURE);
        }
        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

        clientFifo = open(clientFifoName, READ_FLAGS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        memset(input, '\0', sizeof(input));

        int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

        if (fd == -1) {
                perror("Error opening file");
                return EXIT_FAILURE;
        }

        ssize_t bytes_written;

        while (1) {
                ssize_t bytes_read = read(clientFifo, input, sizeof(input) - 1);
                if (bytes_read == -1) {
                        perror("Failed to read from clientFifo");
                        exit(EXIT_FAILURE);
                } else if (bytes_read == 0) {
                        break;
                }

                input[bytes_read] = '\0';

                bytes_written = write(fd, input, sizeof(input));
                if (bytes_written == -1) {
                        perror("Error writing to file");
                        close(fd);
                        return EXIT_FAILURE;
                }

                bytes_written = write(fd, "\n", sizeof("\n"));
                if (bytes_written == -1) {
                        perror("Error writing to file");
                        close(fd);
                        return EXIT_FAILURE;
                }

                memset(input, '\0', sizeof(input));
        }

        close(fd);

        printf("Data written to %s\n", fileName);

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

        return 0;

}

void archServer() {

}
void killServer(int serverFifo, char * serverFifoName, char * clientFifoName) {

        int clientFifo;

        umask(0);

        clientFifo = open(clientFifoName, WRITE_FLAGS, PERMS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        char input[100];

        input[0] = '\0';

        strcat(input, "req killServer ");

        if (write(clientFifo, input, strlen(input)) == -1) {
                perror("Failed to write to clientFifo");
                exit(EXIT_FAILURE);
        }

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

        clientFifo = open(clientFifoName, READ_FLAGS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        memset(input, '\0', sizeof(input));

        while (1) {
                ssize_t bytes_read = read(clientFifo, input, sizeof(input) - 1);
                if (bytes_read == -1) {
                        perror("Failed to read from clientFifo");
                        exit(EXIT_FAILURE);
                } else if (bytes_read == 0) {
                        break;
                }

                input[bytes_read] = '\0';

                if (input[0] == '1') {
                        printf("\nQuit permitted from server. Terminating\n");
                } else {
                        printf("\nQuit wasn't permitted from server. Could not be terminated\n");
                }

                memset(input, '\0', sizeof(input));
        }

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

        if (close(serverFifo) == -1) {
                perror("\nserverFifo close error\n");
                exit(EXIT_FAILURE);
        }

        if (unlink(serverFifoName) == -1) {
                exit(1);
        }

        if (kill(getpid(), SIGTERM) == -1) {
                perror("Error sending signal");
                exit(EXIT_FAILURE);
        }
}

void quit(int serverFifo, char * serverFifoName, char * clientFifoName) {

        int clientFifo;

        umask(0);

        clientFifo = open(clientFifoName, WRITE_FLAGS, PERMS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        char input[100];
        char pid[10];

        input[0] = '\0';

        strcat(input, "req quit ");
        sprintf(pid, "%d", getpid());
        strcat(input, pid);

        if (write(clientFifo, input, strlen(input)) == -1) {
                perror("Failed to write to clientFifo");
                exit(EXIT_FAILURE);
        }

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

        clientFifo = open(clientFifoName, READ_FLAGS);
        if (clientFifo == -1) {
                perror("Failed to open clientFifo");
                exit(EXIT_FAILURE);
        }

        memset(input, '\0', sizeof(input));

        while (1) {
                ssize_t bytes_read = read(clientFifo, input, sizeof(input) - 1);
                if (bytes_read == -1) {
                        perror("Failed to read from clientFifo");
                        exit(EXIT_FAILURE);
                } else if (bytes_read == 0) {
                        break;
                }

                input[bytes_read] = '\0';

                if (input[0] == '1') {
                        printf("\nQuit permitted from server. Terminating\n");
                } else {
                        printf("\nQuit wasn't permitted from server. Could not be terminated\n");
                }

                memset(input, '\0', sizeof(input));
        }

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

        if (close(serverFifo) == -1) {
                perror("\nserverFifo close error\n");
                exit(EXIT_FAILURE);
        }

        if (unlink(serverFifoName) == -1) {
                exit(1);
        }

        if (kill(getpid(), SIGTERM) == -1) {
                perror("Error sending signal");
                exit(EXIT_FAILURE);
        }

}

int operation(char * input, int serverFifo, char * serverFifoName, char * clientFifoName, char * buffer) {

        ssize_t bytes_read;
        int clientFifo;
        char lineNumber[4];
        char command[20];
        char fileName[50];
        char stringToWrite[100];
        char temp[100];

        memset(command, '\0', strlen(command));
        memset(fileName, '\0', strlen(fileName));
        memset(temp, '\0', strlen(temp));

        if (sscanf(input, "%s", command) != 1) {
                help();
        }

        strtok(input, "\n");

        if (strcmp(input, "help") == 0) {
                help();
        } else if (strcmp(command, "list") == 0) {
                list(clientFifoName);

                umask(0);

                clientFifo = open(clientFifoName, READ_FLAGS, PERMS);
                if (clientFifo == -1) {
                        perror("Failed to open clientFifo");
                        exit(EXIT_FAILURE);
                }

                memset(buffer, '\0', strlen(buffer));

                while (1) {

                        bytes_read = read(clientFifo, buffer, 100);

                        if (bytes_read == -1) {
                                perror("Error reading from clientFifo");
                                exit(EXIT_FAILURE);
                        } else if (bytes_read == 0) {
                                break;
                        }

                        memset(buffer, '\0', 100);

                }

                printf("\n");

                if (close(clientFifo) == -1) {
                        perror("\nclientFifo close error\n");
                        exit(EXIT_FAILURE);
                }

        } else if (strcmp(command, "readF") == 0) {
                memset(fileName, '\0', sizeof(fileName));
                memset(lineNumber, '\0', sizeof(lineNumber));

                strtok(input, "");

                char * token;

                token = strtok(input, " ");

                if (strcmp(token, "readF") == 0) {
                        token = strtok(NULL, " ");
                        strcpy(fileName, token);

                        token = strtok(NULL, " ");
                        if (token != NULL) {
                                strcpy(lineNumber, token);
                        } else {
                                strcpy(lineNumber, "0");
                        }

                        if (!isnum(lineNumber)) {
                                printf("\nThis command is not valid.\n");
                                help();
                        } else {
                                readF(clientFifoName, fileName, lineNumber);
                        }

                } else {
                        printf("\nThis command is not valid.\n");
                        help();
                }

        } else if (strcmp(command, "writeT") == 0) {

                memset(fileName, '\0', sizeof(fileName));
                memset(lineNumber, '\0', sizeof(lineNumber));
                memset(stringToWrite, '\0', sizeof(stringToWrite));

                char * token = strtok(input, " ");

                if (strcmp(token, "writeT") == 0) {
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
                        } else {
                                strcpy(lineNumber, "0");
                                strcpy(stringToWrite, "");
                        }

                        if (!isnum(lineNumber)) {
                                printf("\nThis command is not valid.\n");
                                help();
                        } else {
                                writeT(clientFifoName, fileName, lineNumber, stringToWrite);
                        }

                } else {
                        printf("\nThis command is not valid.\n");
                        help();
                }

        } else if (strcmp(command, "upload") == 0) {

                memset(fileName, '\0', sizeof(fileName));

                char * token = strtok(input, " ");
                token = strtok(NULL, " ");
                strcpy(fileName, token);

                upload(clientFifoName, fileName);

                if (sizeof(fileName) == 0) {
                        printf("\nThis command is not valid.\n");
                        help();
                }

        } else if (strcmp(command, "download") == 0) {

                memset(fileName, '\0', sizeof(fileName));

                char * token = strtok(input, " ");
                token = strtok(NULL, " ");
                strcpy(fileName, token);

                download(clientFifoName, fileName);

                if (sizeof(fileName) == 0) {
                        printf("\nThis command is not valid.\n");
                        help();
                }

        } else if (strcmp(command, "archServer") == 0) {
                archServer();
        } else if (strcmp(command, "killServer") == 0) {
                killServer(serverFifo, serverFifoName, clientFifoName);
        } else if (strcmp(command, "quit") == 0) {
                quit(serverFifo, serverFifoName, clientFifoName);
        } else if (strcmp(input, "help list") == 0) {
                help_list();
        } else if (strcmp(input, "help readF") == 0) {
                help_readF();
        } else if (strcmp(input, "help writeT") == 0) {
                help_writeT();
        } else if (strcmp(input, "help upload") == 0) {
                help_upload();
        } else if (strcmp(input, "help download") == 0) {
                help_download();
        } else if (strcmp(input, "help archServer") == 0) {
                help_archServer();
        } else if (strcmp(input, "help killServer") == 0) {
                help_killServer();
        } else if (strcmp(input, "help quit") == 0) {
                help_quit();
        } else {
                printf("\nThis command is not valid.\n");
                help();
        }

        return 0;

}

int createFifo(int serverpid, int connect) {

        int clientFifo, serverFifo;
        char clientFifoName[FIFO_NAME_LEN];
        char serverFifoName[FIFO_NAME_LEN];
        char clientpid[12];
        char buffer[100];
        char input[100];

        sprintf(clientpid, "%d", getpid());

        printf("\nClient started PID %d\n", getpid());

        struct sigaction sa;
        memset( & sa, 0, sizeof(sa));
        sa.sa_handler = handler;
        sigaction(SIGINT, & sa, NULL);

        sprintf(serverFifoName, "%s%ld", SERVERFIFO, (long) serverpid);

        printf("ServerFifo is: %s\n", serverFifoName);

        umask(0);

        if (mkfifo(serverFifoName, PERMS) == -1 && errno != EEXIST) {
                perror("\nserverFifo Error\n");
                exit(EXIT_FAILURE);
        }

        serverFifo = open(serverFifoName, WRITE_FLAGS, PERMS);
        if (serverFifo == -1) {
                perror("Failed to open serverFifo");
                exit(EXIT_FAILURE);
        }

        sprintf(buffer, "pid %ld con %d", (long) getpid(), connect);

        if (write(serverFifo, buffer, sizeof(buffer)) != sizeof(buffer)) {
                perror("\nserverFifo write error\n");
                exit(EXIT_FAILURE);
        }

        sprintf(clientFifoName, "%s%ld", CLIENTFIFO, (long) getpid());

        umask(0);

        if (mkfifo(clientFifoName, PERMS) == -1 && errno != EEXIST) {
                perror("\nclientFifo Error\n");
                exit(EXIT_FAILURE);
        }

        clientFifo = open(clientFifoName, READ_FLAGS, PERMS);
        if (clientFifo == -1) {
                perror("Failed to open serverFifo");
                exit(EXIT_FAILURE);
        }

        memset(buffer, '\0', sizeof(buffer));

        while (read(clientFifo, buffer, sizeof(buffer)) <= 0);

        memset(buffer, '\0', sizeof(buffer));

        while (1) {

                printf("Enter a command: \n");

                while (read(STDIN_FILENO, input, sizeof(input)) <= 0);

                memset(buffer, '\0', sizeof(buffer));

                operation(input, serverFifo, serverFifoName, clientFifoName, buffer);

                fflush(stdout);

                memset(input, '\0', sizeof(input));

        }

        if (close(clientFifo) == -1) {
                perror("\nclientFifo close error\n");
                exit(EXIT_FAILURE);
        }

        if (unlink(clientFifoName) == -1) {
                exit(1);
        }

        if (close(serverFifo) == -1) {
                perror("\nserverFifo close error\n");
                exit(EXIT_FAILURE);
        }

        if (unlink(serverFifoName) == -1) {
                exit(1);
        }

        exit(EXIT_SUCCESS);

        return 0;

}

int main(int argc, char * argv[]) {

        setbuf(stdout, NULL);

        int serverpid;
        int connect;

        if (argc != 3) {
                perror("\nFormat: ./neHosClient <Connect/tryConnect> ServerPID");
                return 1;
        }

        if (argv[2]) {
                serverpid = atoi(argv[2]);
        }

        if (strcmp(argv[1], "Connect") == 0) {
                connect = 1;
        } else if (strcmp(argv[1], "tryConnect") == 0) {
                connect = 0;
        } else {
                perror("\nInvalid command. Couldn't connect to a server.\n\nTerminating\n");
                return 1;
        }

        printf("Connect status is %d\n", connect);
        printf("Serverpid is %d\n", serverpid);
        printf("Clientpid is %d\n", getpid());

        createFifo(serverpid, connect);

}
