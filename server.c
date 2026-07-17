#include <unistd.h>    
#include <stdio.h>      
#include <string.h>     

//Hardcoded kernel definitions
#define SYS_CLOSE   3   // Linux x86_64 close syscall
#define SYS_SOCKET  41  // Linux x86_64 socket syscall
#define SYS_SENDTO  44  // Linux x86_64 sendto syscall

#define AF_INET     2   // Internet IP protocol
#define SOCK_DGRAM  2   // UDP layout

#define PORT 5353
#define BUFFER_SIZE 65507

//The exact nested structure format
struct in_addr {
    unsigned int s_addr;
};

struct sockaddr_in {
    unsigned short sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;  
    char           sin_zero[8];
};

int main() {
    //Request a socket straight from the kernel
    long sd = syscall(SYS_SOCKET, AF_INET, SOCK_DGRAM, 0);
    if (sd < 0) {
        printf("The socket is unable to open\n");
        return 1;
    }

    //Set up the target using your nested structure and manual byte swaps
    struct sockaddr_in target;
    __builtin_memset(&target, 0, sizeof(target)); // Zero out padding properly
    
    target.sin_family = AF_INET;
    target.sin_port = 0xE914;              // 5353 in Big-Endian
    target.sin_addr.s_addr = 0x0100007F;   // 127.0.0.1 in Big-Endian

    char buffer[BUFFER_SIZE];

    printf("The program started at port %d. Enter \"exit\" to stop.\n", PORT);

    while (1) {
        printf("You: ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;

        // Strip trailing newline character
        buffer[strcspn(buffer, "\n")] = '\0';
         
        if (strcmp("exit", buffer) == 0) break;

        //Send raw message using Syscall 44
        // Note: I cast target to a generic pointer just like standard C does
        long send_byte = syscall(SYS_SENDTO, sd, buffer, strlen(buffer), 0, 
                                 (void *)&target, sizeof(target));

        if (send_byte < 0) {
            printf("Failed to send the message\n");
        }
    }

    // Tell the kernel to release the file descriptor directly
    syscall(SYS_CLOSE, sd);
    return 0;
}