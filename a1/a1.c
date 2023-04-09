#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

typedef struct header
{
    char sect_name[13];
    int sect_type;
    int sect_offset;
    int sect_size;
} sect_header;

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
        printf("ERROR\n");
        return ERROR;
    }
    else
        printf("SUCCESS\n");
    while ((entry = readdir(dir)) != NULL)
    {
        snprintf(path, MAX_PATH_LEN, "%s/%s", dir_path, entry->d_name); // building the path
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
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

int list_recursive(char *dir_path, int size, int exec, int recursive)
{
    struct dirent *entry;
    struct stat stats;
    char path[MAX_PATH_LEN];
    DIR *dir = opendir(dir_path);
    if (dir == NULL)
    {
        printf("ERROR\n");
        return ERROR;
    }
    else if (recursive == 0)
        printf("SUCCESS\n");

    char **stack = NULL;
    int stack_size = 0;
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
            if (list_recursive(path, size, exec, 1) == SUCCESS)
            {
                if (exec == 1)
                {
                    if (access(path, X_OK) == 0)
                        stack = realloc(stack, sizeof(char *) * (stack_size + 1)), stack[stack_size++] = strdup(path);
                }
                else
                    stack = realloc(stack, sizeof(char *) * (stack_size + 1)), stack[stack_size++] = strdup(path);
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
    for (int i = stack_size - 1; i >= 0; i--)
    {
        printf("%s\n", stack[i]);
    }
    free(stack);
    return SUCCESS;
}

int parse(char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        printf("ERROR\n");
        return ERROR;
    }
    char magic[5];
    long file_size = lseek(fd, 0, SEEK_END);
    if (file_size < 0)
    {
        printf("ERROR\n");
        return ERROR;
    }
    long offset = file_size - 4;
    long read_bytes = 0;
    if ((read_bytes = pread(fd, magic, 4, offset)) < 0)
    {
        printf("ERROR\n");
        return ERROR;
    }
    if (strcmp(magic, "PYQe") != 0)
    {
        printf("ERROR\n");
        printf("wrong magic\n");
        return ERROR;
    }
    offset = offset - 2;
    if ((read_bytes = pread(fd, &file_size, 2, offset)) < 0)
    {
        return ERROR;
    }
    offset = offset + 6 - file_size;
    int version;
    if ((read_bytes = pread(fd, &version, 2, offset) < 0))
    {
        printf("ERROR\n");
        return ERROR;
    }
    if (version < 102 || version > 129)
    {
        printf("ERROR\n");
        printf("wrong version\n");
        return ERROR;
    }
    int nr_of_sections = 0;
    offset = offset + 2;
    if ((read_bytes = pread(fd, &nr_of_sections, 1, offset)) < 0)
    {
        printf("ERROR\n");
        return ERROR;
    }
    if (nr_of_sections < 2 || nr_of_sections > 20)
    {
        printf("ERROR\n");
        printf("wrong sect_nr\n");
        return ERROR;
    }
    offset = offset + 1;
    sect_header **section_headers = malloc(nr_of_sections * sizeof(sect_header *));
    for (int i = 0; i < nr_of_sections; i++)
    {
        section_headers[i] = malloc(sizeof(sect_header));
        // sect_header *section = malloc(sizeof(sect_header));
        int type = 0, ofset = 0, size = 0;
        char name[12];
        if ((read_bytes = pread(fd, &name, 12, offset)) < 0)
        {
            printf("ERROR\n");
            return ERROR;
        }
        offset = offset + 12;
        if ((read_bytes = pread(fd, &type, 1, offset)) < 0)
        {
            printf("ERROR\n");
            return ERROR;
        }
        if (type != 93 && type != 34 && type != 31 && type != 79)
        {
            printf("ERROR\n");
            printf("wrong sect_types\n");
            return ERROR;
        }
        offset = offset + 1;
        if ((read_bytes = pread(fd, &ofset, 4, offset)) < 0)
        {
            printf("ERROR\n");
            return ERROR;
        }
        offset = offset + 4;
        if ((read_bytes = pread(fd, &size, 4, offset)) < 0)
        {
            printf("ERROR\n");
            return ERROR;
        }
        offset = offset + 4;
        int namesize = sizeof(name);
        for (int j = 0; j < namesize; j++)
        {
            section_headers[i]->sect_name[j] = name[j];
        }
        section_headers[i]->sect_name[12] ='\0';
        section_headers[i]->sect_offset = ofset;
        section_headers[i]->sect_type = type;
        section_headers[i]->sect_size = size;
        // free(section);
    }
    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", nr_of_sections);
    for (int i = 0; i < nr_of_sections; i++)
    {
        printf("section%d: ", i + 1);
        printf("%s ", section_headers[i]->sect_name);
        printf("%d ",section_headers[i]->sect_type);
        printf("%d\n",section_headers[i]->sect_size);
    }
    for (int i = 0; i < nr_of_sections; i++)
    {
        free(section_headers[i]);
    }
    free(section_headers);
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
            printf("39249\n");
        if (strcmp(cmd, "list") == 0)
        {
            int size = 0, exec = 0, recursive = 0;
            char *path = strchr(argv[argc - 1], '=') + 1;
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
                list_recursive(path, size, exec, 0);
            }
            if (recursive == 0)
            {
                list(path, size, exec);
            }
        }
        if (strcmp(cmd, "parse") == 0)
        {
            char *path = strchr(argv[argc - 1], '=') + 1;
            parse(path);
        }
    }
    return 1;
}