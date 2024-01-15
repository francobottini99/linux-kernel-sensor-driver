#include "utils.h"

char* get_sensors_list(void)
{
    int sensors = 0;
    struct dirent *entry;
    DIR *dir;
    char* err = calloc(strlen("error:\"") + 1, sizeof(char));
    char* content = calloc(strlen("content:[") + 1, sizeof(char));
    char* type = calloc(strlen("type:\"string_list\"") + 1, sizeof(char));
    char* result = NULL;

    strcpy(err, "error:\"");
    strcpy(content, "content:[");
    strcpy(type, "type:\"string_list\"");
    
    dir = opendir("/dev");

    if (dir == NULL) 
    {
        err = realloc(err, strlen(err) + strlen("Failed attemp get list of sensors\"") + 1);
        content = realloc(content, strlen(content) + strlen("]") + 1);

        strcat(err, "Failed attemp get list of sensors\"");
        strcat(content, "]");

        perror("\nopendir");

        goto finish;
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        if(strstr(entry->d_name, "DHT11") || strstr(entry->d_name, "TFMINI"))
        {
            content = realloc(content, strlen(content) + strlen(entry->d_name) + strlen("\",\"") + 1);

            strcat(content, "\"");
            strcat(content, entry->d_name);
            strcat(content, "\"");
            strcat(content, ",");
            
            sensors = 1;
        }
    }

    if(sensors)
    {
        content[strlen(content) - 1] = '\0';

        content = realloc(content, strlen(content) + strlen("]") + 1);
        err = realloc(err, strlen(err) + strlen("\"") + 1);

        strcat(content, "]");
        strcat(err, "\"");

        goto finish;
    }
    else
    {
        err = realloc(err, strlen(err) + strlen("No sensors found\"") + 1);
        content = realloc(content, strlen(content) + strlen("]") + 1);

        strcat(err, "No sensors found\"");
        strcat(content, "]");

        goto finish;
    }

    finish:

    result = calloc(strlen(err) + strlen(type) + strlen(content) + strlen("{,,}") + 1, sizeof(char));

    strcpy(result, "{");
    strcat(result, err);
    strcat(result, ",");
    strcat(result, type);
    strcat(result, ",");
    strcat(result, content);
    strcat(result, "}");

    closedir(dir);

    free(err);
    free(content);
    free(type);

    return result;
}

char* get_sensor_read(char* path)
{
    char dev_path[PATH_MAX];
    char read_buffer[256];

    ssize_t bytes_read = 0;

    char* err = calloc(strlen("error:\"") + 1, sizeof(char));
    char* type = calloc(strlen("type:\"string\"") + 1, sizeof(char));
    char* content = calloc(strlen("content:\"") + 1, sizeof(char));
    char* result = NULL;

    strcpy(err, "error:\"");
    strcpy(content, "content:\"");
    strcpy(type, "type:\"string\"");

    strcpy(dev_path, "/dev/");
    strcat(dev_path, path);

    int fd = open(dev_path, O_RDONLY);

    if (fd == -1) 
    {
        err = realloc(err, strlen(err) + strlen("Failed attemp get sensor read\"") + 1);
        content = realloc(content, strlen(content) + strlen("\"") + 1);

        strcat(err, "Failed attemp get sensor read\"");
        strcat(content, "\"");

        perror("\nopen");

        goto finish;
    }

    if((bytes_read = read(fd, read_buffer, sizeof(read_buffer))) > 0)
    {
        read_buffer[bytes_read - 2] = '\0';

        content = realloc(content, strlen(content) + strlen(read_buffer) + strlen("\"") + 1);
        err = realloc(err, strlen(err) + strlen("\"") + 1);

        strcat(content, read_buffer);
        strcat(content, "\"");
        strcat(err, "\"");

        goto finish;
    }
    else
    {
        err = realloc(err, strlen(err) + strlen("Failed attemp get sensor read\"") + 1);
        content = realloc(content, strlen(content) + strlen("\"") + 1);

        strcat(err, "Failed attemp get sensor read\"");
        strcat(content, "\"");

        perror("\nread");

        goto finish;
    }
    
    finish:

    result = calloc(strlen(err) + strlen(type) + strlen(content) + strlen("{,,}") + 1, sizeof(char));

    strcpy(result, "{");
    strcat(result, err);
    strcat(result, ",");
    strcat(result, type);
    strcat(result, ",");
    strcat(result, content);
    strcat(result, "}");

    close(fd);

    free(err);
    free(content);
    free(type);
    
    return result;
}