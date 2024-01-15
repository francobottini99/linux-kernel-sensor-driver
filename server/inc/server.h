#ifndef __SERVER_H__
#define __SERVER_H__

#include "threads_handle.h"
#include "common.h"
#include "utils.h"

int ipv4_socket_fd;

extern volatile sig_atomic_t finished;

void signal_handler(int sig, siginfo_t *info, void* context);

void signal_handler_init(void);

void *connection_handler(void *args);

void connection_end(int *client_fd);

error_code create_ipv4_socket(const uint16_t socket_port);

error_code accept_connection(int *client_fd);

void init(void);

void end(void);

#endif // __SERVER_H__