#include "server.h"

#define MESSAGE_SIZE 1024

volatile sig_atomic_t finished = 0;

pthread_mutex_t mutex;

void signal_handler(int sig, siginfo_t *info, void* context)
{
    UNUSED(info);
    UNUSED(context);

    if(sig == SIGTERM || sig == SIGINT || sig == SIGHUP || sig == SIGPIPE)
        finished = 1;
}

void signal_handler_init(void)
{
    struct sigaction sa = 
    {
        .sa_sigaction = signal_handler,
        .sa_flags = SA_SIGINFO
    };
    sigemptyset(&sa.sa_mask);

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
}

char* response_handler(char* request)
{
    char* result = NULL;

    if(strcmp(request, "GET LIST") == 0)
        result = get_sensors_list();
    else if(strstr(request, "GET DATA "))
        result = get_sensor_read(strstr(request, "GET DATA ") + strlen("GET DATA "));
    else
    {
        result = calloc(strlen("{error:\"Invalid request\",type:\"\",content:\"\"}") + 1, sizeof(char));

        strcpy(result, "{error:\"Invalid request\",type:\"\",content:\"\"}");
    }

    return result;
}

void *connection_handler(void *args) 
{    
    int client_fd = *(int*)args; 
    char* data = NULL;
    char* result = NULL;
    
    printf(KGRN"\nClient (FD: %d) connect !\n"KDEF, client_fd);

    while (1)
    {
        struct timeval timeout = {0, 100000};
        fd_set read_fds;
        ssize_t bytes_received;
        ssize_t bytes_sent;

        FD_ZERO(&read_fds);

        FD_SET(client_fd, &read_fds);

        if(finished)
            break;

        if (select(client_fd + 1, &read_fds, NULL, NULL, &timeout) > 0)
        {
            if(FD_ISSET(client_fd, &read_fds))
            {
                data = calloc(MESSAGE_SIZE, sizeof(char));

                bytes_received = recv(client_fd, data, MESSAGE_SIZE, 0);

                if(bytes_received == 0)
                {
                    free(data);
                    break;
                }
                else if (bytes_received < 0)
                    fprintf(stderr, KRED"\nError receiving data from client (FD: %d) \n"KDEF, client_fd);
                else
                {
                    printf(KYEL"\nRecibe [%d B] (FD: %d)\n"KDEF, bytes_received, client_fd);
                    printf(KYEL"\nData: %s\n"KDEF, data);

                    result = response_handler(data);

                    bytes_sent = send(client_fd, result, strlen(result) + 1, 0);
                    
                    if (bytes_sent == 0)
                    {
                        free(result);
                        free(data);
                        break;
                    }
                    else if (bytes_sent < 0)
                        fprintf(stderr, KRED"\nError sending data to client (FD: %d) \n"KDEF, client_fd);
                    else
                    {
                        printf(KCYN"\nSend [%d B] (FD: %d)\n"KDEF, bytes_sent, client_fd);
                        printf(KCYN"\nData: %s\n"KDEF, result);
                    }

                    free(result);
                }

                free(data);
            }
        }
    }

    connection_end((int*)args);

    return NULL;
}

void connection_end(int *client_fd)
{
    printf(KRED"\nClient (FD: %d) disconnect !\n"KDEF, *client_fd);
    
    close(*client_fd);
    free(client_fd);

    if(!finished)
    {
        pthread_mutex_lock(&mutex);
        handler_destroy(pthread_self());
        pthread_mutex_unlock(&mutex);
    }
}

error_code create_ipv4_socket(const uint16_t socket_port)
{
    struct sockaddr_in server_address;

    ipv4_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (ipv4_socket_fd < 0) 
    {
        perror("socket() failed");
        return ERROR_SOCKET_CREATION;
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(socket_port);

    if (bind(ipv4_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        perror("bind() failed");
        return ERROR_SOCKET_BIND;
    }

    if (listen(ipv4_socket_fd, 1) < 0) 
    {
        perror("listen() failed");
        return ERROR_SOCKET_LISTEN;
    }

    return SUCCESS;
}

error_code accept_connection(int *client_fd)
{
    *client_fd = -1;

    while (*client_fd < 0 && !finished)
    {
        struct timeval timeout = {0, 100000};
        fd_set socket_set;

        FD_ZERO(&socket_set);

        FD_SET(ipv4_socket_fd, &socket_set);

        if (select(ipv4_socket_fd + 1, &socket_set, NULL, NULL, &timeout) > 0)
        {
            if (FD_ISSET(ipv4_socket_fd, &socket_set))
                *client_fd = accept(ipv4_socket_fd, NULL, NULL);
        }
    }

    if(finished)
        return END_SIGNAL;

    return SUCCESS;
}

void init(void)
{
    error_code result;

    result = create_ipv4_socket(IPV4_SOCKET_PORT);

    if (result != SUCCESS)
    {
        fprintf(stderr, "Server creation failed with error code %d\n", result);
        exit(EXIT_FAILURE);
    }

    signal_handler_init();

    pthread_mutex_init(&mutex, NULL);

    printf(KBLU"\nServer start (FD: %d) !\n"KDEF, ipv4_socket_fd);
}

void end(void)
{
    handler_wait_all();
    handler_destroy_all();

    close(ipv4_socket_fd);

    pthread_mutex_destroy(&mutex);

    printf(KBLU"\nServer stop (FD: %d) !\n"KDEF, ipv4_socket_fd);

    exit(EXIT_SUCCESS);
}

int main(void)
{
    if(geteuid() != 0)
    {
        fprintf(stderr, KRED"Server must be run as root !\n"KDEF);
        exit(EXIT_FAILURE);
    }

    init();

    int *client_fd = calloc(1, sizeof(int));   
    
    error_code result;

    do 
    {
        result = accept_connection(client_fd);

        if(result == SUCCESS)
        {
            pthread_t* tid = handler_create();

            if (pthread_create(tid, NULL, connection_handler, (void *)client_fd) != 0)
            {
                perror("pthread_create() failed");
                close(*client_fd);
                free(client_fd);
            }

            client_fd = calloc(1, sizeof(int));
        }
    } while(result != END_SIGNAL);

    end();

    return EXIT_SUCCESS;
}