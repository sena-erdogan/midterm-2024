#include <sys/stat.h>

#include <sys/types.h>

#include <sys/mman.h>

#include <string.h>

#include <stdlib.h>

#include <stdio.h>

#include <fcntl.h>

#include <errno.h>

#include <sys/wait.h>

#include <signal.h>

#include <stdbool.h>

#include <ctype.h>

#include <semaphore.h>

#include <unistd.h>

#include <dirent.h>

#include <sys/file.h>

#include <time.h>

#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_CREAT | O_APPEND)
#define PERMS (S_IRUSR | S_IWUSR | S_IWGRP)

#define LOG_FILE "log%ld.txt"
#define SERVERFIFO "/tmp/serverFifo."
#define CLIENTFIFO "/tmp/clientFifo."
#define FIFO_NAME_LEN (sizeof(SERVERFIFO) + 20)

typedef struct Node {
        int data;
        struct Node * next;
}
Node;

typedef struct {
        Node * front;
        Node * rear;
        int size;
}
Queue;

Queue * create() {
        Queue * queue = malloc(sizeof(Queue));
        queue -> front = NULL;
        queue -> rear = NULL;
        queue -> size = 0;
        return queue;
}

void destroy(Queue * queue) {
        Node * current = queue -> front;
        while (current != NULL) {
                Node * temp = current;
                current = current -> next;
                free(temp);
        }
        free(queue);
}

int is_empty(Queue * queue) {
        return queue -> size == 0;
}

void push(Queue * queue, int value) {
        Node * new_node = malloc(sizeof(Node));
        new_node -> data = value;
        new_node -> next = NULL;

        if (is_empty(queue)) {
                queue -> front = new_node;
                queue -> rear = new_node;
        } else {
                queue -> rear -> next = new_node;
                queue -> rear = new_node;
        }

        queue -> size++;
}

int pop(Queue * queue) {
        if (is_empty(queue)) {
                printf("Error: Queue is empty\n");
                return -1;
        } else {
                int value = queue -> front -> data;
                Node * temp = queue -> front;
                queue -> front = queue -> front -> next;
                free(temp);
                queue -> size--;
                return value;
        }
}

int size(Queue * queue) {
        return queue -> size;
}

int remove_value(Queue * queue, int target) {
        if (is_empty(queue)) {
                printf("Error: Queue is empty\n");
                return -1;
        } else {
                Node * current = queue -> front;
                Node * previous = NULL;

                while (current != NULL) {
                        if (current -> data == target) {
                                int value = current -> data;

                                if (previous == NULL) {
                                        queue -> front = current -> next;
                                } else {
                                        previous -> next = current -> next;
                                }

                                if (current == queue -> rear) {
                                        queue -> rear = previous;
                                }

                                free(current);
                                queue -> size--;
                                return value;
                        }

                        previous = current;
                        current = current -> next;
                }

                printf("Error: Data value not found in the queue\n");
                return -1;
        }
}
