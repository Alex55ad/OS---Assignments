#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#define REQUEST_PIPE_NAME "REQ_PIPE_39249"
#define RESPONSE_PIPE_NAME "RESP_PIPE_39249"
#define BUFFER_SIZE 1024
#define SHM_NAME "/BR9Zl2"
void* mapping=NULL;
int main()
{
    if (mkfifo(RESPONSE_PIPE_NAME, 0600) == -1)
    {
        printf("ERROR\nCannot create the response pipe\n");
        perror("fifo: ");
        exit(1);
    }
    int req_pipe = open(REQUEST_PIPE_NAME, O_RDONLY);
    if (req_pipe == -1)
    {
        printf("ERROR\nCannot open the request pipe\n");
        perror("req_pipe: ");
        exit(1);
    }
    int resp_pipe = open(RESPONSE_PIPE_NAME, O_WRONLY);
    if (resp_pipe == -1)
    {
        printf("ERROR\nCannot open the response pipe\n");
        perror("resp_pipe: ");
        exit(1);
    }
    char *startMSG = "START";
    int size = strlen("START");
    write(resp_pipe, &size, 1);
    if (write(resp_pipe, startMSG, size) == -1)
    {
        perror("ERROR WRITING START MSG: ");
        exit(1);
    }
    printf("SUCCESS\n");
    while (1)
    {
        int SHMsize;
        int size = 0;
        long bytes_read = read(req_pipe, &size, 1);
        if (bytes_read <= 0)
        {
            printf("ERROR\n");
            perror("read from req_pipe");
            exit(1);
        }
        char *request = (char *)malloc(sizeof(char) * size + 1);
        bytes_read = read(req_pipe, request, size);
        if (bytes_read <= 0)
        {
            perror("read from req_pipe err: ");
            exit(1);
        }
        request[size] = '\0';
        if (strcmp(request, "ECHO") == 0)
        {
            char *echo = "ECHO";
            char *variant = "VARIANT";
            int nr = 39249;
            int responseSize = strlen("ECHO");
            write(resp_pipe, &responseSize, 1);
            write(resp_pipe, echo, responseSize);
            responseSize = strlen("VARIANT");
            write(resp_pipe, &responseSize, 1);
            write(resp_pipe, variant, responseSize);
            write(resp_pipe, &nr, sizeof(int));
        }
        else if (strcmp(request, "CREATE_SHM") == 0)
        {
            int boolean = 1;
            read(req_pipe, &SHMsize, sizeof(int));
            int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0664);

            if (shm_fd == -1)
            {
                perror("SHM_FD ERR: ");
                boolean = 0;
            }

            if (ftruncate(shm_fd, SHMsize) == -1)
            {
                perror("SHM SIZE ERR: ");
                boolean = 0;
            }
            mapping = mmap(NULL, SHMsize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
            if (mapping == MAP_FAILED)
            {
                perror("MAPPING ERR: ");
                boolean = 0;
            }
            char *create = "CREATE_SHM";
            int createSize = strlen("CREATE_SHM");
            write(resp_pipe, &createSize, 1);
            write(resp_pipe, create, createSize);
            char *fail = "ERROR";
            int failSize = strlen("ERROR");
            char *success = "SUCCESS";
            int successSize = strlen("SUCCESS");
            if (boolean == 0)
            {
                write(resp_pipe, &failSize, 1);
                write(resp_pipe, fail, failSize);
            }
            else
            {
                write(resp_pipe, &successSize, 1);
                write(resp_pipe, success, successSize);
            }
            close(shm_fd);
        }
        else if (strcmp(request, "WRITE_TO_SHM") == 0)
        {
            if (mapping != MAP_FAILED)
            {
                unsigned int nr;
                unsigned int offset;
                read(req_pipe, &offset, sizeof(unsigned int));
                read(req_pipe, &nr, sizeof(unsigned int));
                if (offset >= 0 && offset <= SHMsize && offset + sizeof(unsigned int) <= SHMsize)
                {
                    *(unsigned int *)(mapping + offset) = nr;
                    char *writeMsg = "WRITE_TO_SHM";
                    int writeSize = strlen("WRITE_TO_SHM");
                    write(resp_pipe, &writeSize, 1);
                    write(resp_pipe, writeMsg, writeSize);
                    char *success = "SUCCESS";
                    int successSize = strlen("SUCCESS");
                    write(resp_pipe, &successSize, 1);
                    write(resp_pipe, success, successSize);
                }
                else
                {
                    char *writeMsg = "WRITE_TO_SHM";
                    int writeSize = strlen("WRITE_TO_SHM");
                    write(resp_pipe, &writeSize, 1);
                    write(resp_pipe, writeMsg, writeSize);
                    char *fail = "ERROR";
                    int failSize = strlen("ERROR");
                    write(resp_pipe, &failSize, 1);
                    write(resp_pipe, fail, failSize);
                }
            }
        }
        else if (strcmp(request, "MAP_FILE") == 0)
        {
            int fileNameSize = 0;
            read(req_pipe, &fileNameSize, sizeof(int));

            char *fileName = (char *)malloc(sizeof(char) * (fileNameSize + 1));
            read(req_pipe, fileName, fileNameSize);
            fileName[fileNameSize] = '\0';
            char *fullFileName = (char *)malloc(sizeof(char) * (fileNameSize + 5));
            strcpy(fullFileName, "tes");
            strcat(fullFileName, fileName);
            printf("%s\n", fullFileName);
            int filefd = open(fullFileName, O_RDONLY);
            if (filefd == -1)
            {
                perror("File fd error: ");
                char *writeMsg = "MAP_FILE";
                int writeSize = strlen("MAP_FILE");
                write(resp_pipe, &writeSize, 1);
                write(resp_pipe, writeMsg, writeSize);
                char *fail = "ERROR";
                int failSize = strlen("ERROR");
                write(resp_pipe, &failSize, 1);
                write(resp_pipe, fail, failSize);
            }
            else
            {
                struct stat filestat;
                if (fstat(filefd, &filestat) == -1)
                {
                    perror("File fd error: ");
                    char *writeMsg = "MAP_FILE";
                    int writeSize = strlen("MAP_FILE");
                    write(resp_pipe, &writeSize, 1);
                    write(resp_pipe, writeMsg, writeSize);
                    char *fail = "ERROR";
                    int failSize = strlen("ERROR");
                    write(resp_pipe, &failSize, 1);
                    write(resp_pipe, fail, failSize);
                }
                else
                {
                    off_t fileSize = filestat.st_size;
                    char *map = (char *)mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, filefd, 0);
                    if (map == MAP_FAILED)
                    {
                        perror("MAP FAIL ERROR: ");
                        // map[0]=fileName;
                        char *writeMsg = "MAP_FILE";
                        int writeSize = strlen("MAP_FILE");
                        write(resp_pipe, &writeSize, 1);
                        write(resp_pipe, writeMsg, writeSize);
                        char *fail = "ERROR";
                        int failSize = strlen("ERROR");
                        write(resp_pipe, &failSize, 1);
                        write(resp_pipe, fail, failSize);
                    }
                    else
                    {
                        char *writeMsg = "MAP_FILE";
                        int writeSize = strlen("MAP_FILE");
                        write(resp_pipe, &writeSize, 1);
                        write(resp_pipe, writeMsg, writeSize);
                        char *success = "SUCCESS";
                        int successSize = strlen("SUCCESS");
                        write(resp_pipe, &successSize, 1);
                        write(resp_pipe, success, successSize);
                    }
                }
                close(filefd);
                free(fileName);
                free(fullFileName);
            }
        }
        else if (strcmp(request, "EXIT") == 0)
        {
            close(resp_pipe);
            close(req_pipe);
            free(request);
            unlink(RESPONSE_PIPE_NAME);
            break;
        }
        else
        {
            close(resp_pipe);
            close(req_pipe);
            free(request);
            unlink(RESPONSE_PIPE_NAME);
            break;
        }
        free(request);
    }
    return 0;
}