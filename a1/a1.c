#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#define SUCCESS 1
#define FAIL 0
#define ERROR -1
#define MAX_PATH_LEN 1024

int list(char *dir_path, int size, int exec)
{
    struct dirent *entry;
    DIR *dir = opendir(dir_path);
    struct stat stats;
    char path[MAX_PATH_LEN];
    if (dir == NULL)
    {
        printf("Directory could not be open or not found\n");
        return ERROR;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        snprintf(path, MAX_PATH_LEN, "%s/%s", dir_path, entry->d_name); // building the path
        if (stat(path, &stats) == -1)
        {
            continue;
        }
        if (exec == 1)
        {
            if (access(path, X_OK) == 0)
                printf("%s\n", path);
        }
        if (size > 0)
        {
            if (stats.st_size < size)
                printf("%s\n", path);
        }
        if (size > 0 && exec == 1)
        {
            if (access(path, X_OK) == 0 && stats.st_size < size)
                printf("%s\n", path);
        }
        if (size == 0 && exec == 0)
            printf("%s\n", path);
    }
    closedir(dir);
    return SUCCESS;
}

int list_recursive(char *dir_path, int size, int exec)
{
    struct dirent *entry;
    struct stat stats;
    char path[MAX_PATH_LEN];
    DIR *dir = opendir(dir_path);
    if (dir == NULL)
    {
        printf("Directory could not be open or not found\n");
        return ERROR;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        snprintf(path, MAX_PATH_LEN, "%s/%s", dir_path, entry->d_name); // building the path
        if (stat(path, &stats) == -1)
        {
            continue;
        }
        if (S_ISDIR(stats.st_mode))
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            if (list_recursive(path, size, exec) == SUCCESS)
            {
                if (exec == 1)
                {
                    if (access(path, X_OK) == 0)
                        printf("%s\n", path);
                }
                else
                printf("%s\n", path);
                closedir(dir);
                return SUCCESS;
            }
        }
        else if (S_ISREG(stats.st_mode))
        {
            if (exec == 1)
            {
                if (access(path, X_OK) == 0)
                    printf("%s\n", path);
            }
            if (size > 0)
            {
                if (stats.st_size < size)
                    printf("%s\n", path);
            }
            if (size > 0 && exec == 1)
            {
                if (access(path, X_OK) == 0 && stats.st_size < size)
                    printf("%s\n", path);
            }
            if (size == 0 && exec == 0)
                printf("%s\n", path);
        }
    }
    closedir(dir);
    return SUCCESS;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("No input command given\n");
        exit(-1);
    }
    else
    {
        char *cmd = argv[1];
        if (strcmp(cmd, "variant") == 0)
            printf("Variant number: 39249\n");
        if (strcmp(cmd, "list") == 0)
        {
            int size = 0, exec = 0, recursive = 0;
            char *path = strchr(argv[argc - 1], '=') + 1;
            if (argc > 3)
            {
                for (int i = 2; i < argc - 1; i++)
                {
                    if (strstr(argv[i], "has_perm_execute") != NULL)
                        exec = 1;
                    if (strstr(argv[i], "size_smaller=") != NULL)
                    {
                        char *nr = strchr(argv[i], '=') + 1;
                        size = atoi(nr);
                    }
                    if (strstr(argv[i], "recursive") != NULL)
                        recursive = 1;
                }
                if (recursive == 1)
                {
                    if (list_recursive(path, size, exec) == SUCCESS)
                        printf("Success\n");
                    else
                        printf("Unsuccessful\n");
                }
                if(recursive == 0)
                {
                    if (list(path, size, exec) == SUCCESS)
                        printf("Success\n");
                    else
                        printf("Unsuccessful\n");
                }
            }
        }
    }
    return 1;
}