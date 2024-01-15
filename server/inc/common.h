#ifndef __COMMON_H__
#define __COMMON_H__

#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <zlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#ifndef TERMINAL_TEXT_COLORS
#define TERMINAL_TEXT_COLORS
    #define KDEF  "\x1B[0m"     // Default
    #define KRED  "\x1B[31m"    // Red
    #define KGRN  "\x1B[32m"    // Green
    #define KYEL  "\x1B[33m"    // Yellow
    #define KBLU  "\x1B[34m"    // Blue
    #define KMAG  "\x1B[35m"    // Magenta
    #define KCYN  "\x1B[36m"    // Cyan
    #define KWHT  "\x1B[37m"    // White
#endif

#define UNUSED(x) (void)(x)

#define IPV4_SOCKET_PORT 5000

typedef enum 
{
    SUCCESS = 0,                    // Success
    ERROR_SOCKET_CREATION = -1,     // Socket creation failed
    ERROR_SOCKET_BIND = -2,         // Socket bind failed
    ERROR_SOCKET_LISTEN = -3,       // Socket listen failed
    ERROR_SOCKET_ACCEPT = -4,       // Socket accept failed
    ERROR_SOCKET_CONNECTION = -5,   // Socket connection failed
    ERROR_SOCKET_SEND = -6,         // Socket send failed
    ERROR_SOCKET_RECEIVE = -7,      // Socket receive failed
    ERROR_THREAD_FAILED = -8,       // Thread creation failed
    ERROR_SOCKET_DISCONNECT = -9,   // Socket lost connection
    END_SIGNAL = -10                // End signal received
} error_code;

#endif // __COMMON_H__