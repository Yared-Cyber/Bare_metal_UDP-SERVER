/*
 * Freestanding UDP Client in C (x86_64 Linux)
 * Build with: gcc -nostdlib -ffreestanding -fno-stack-protector -no-pie udp_baremetal.c -o udp_baremetal
 */

// --- x86_64 Linux System Call Numbers ---
#define SYS_READ    0
#define SYS_WRITE   1
#define SYS_CLOSE   3
#define SYS_SOCKET  41
#define SYS_SENDTO  44
#define SYS_EXIT    60

// --- Socket Constants ---
#define AF_INET     2
#define SOCK_DGRAM  2

#define STDIN_FILENO  0
#define STDOUT_FILENO 1

#define BUFFER_SIZE 1024

// --- Custom Minimal Network Structures ---
struct in_addr {
    unsigned int s_addr;
};

struct sockaddr_in {
    unsigned short sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;
    char           sin_zero[8];
};

// --- Raw Inline Assembly Syscall Wrapper (x86_64 ABI) ---
static inline long raw_syscall(long num, long a1, long a2, long a3, long a4, long a5, long a6) {
    long ret;
    register long r10 __asm__("r10") = a4;
    register long r8  __asm__("r8")  = a5;
    register long r9  __asm__("r9")  = a6;

    __asm__ volatile (
        "syscall"
        : "=a" (ret)
        : "a" (num), "D" (a1), "S" (a2), "d" (a3), "r" (r10), "r" (r8), "r" (r9)
        : "rcx", "r11", "memory"
    );
    return ret;
}

// --- Freestanding Helper Functions (Replaces libc) ---
static size_t raw_strlen(const char *str) {
    const char *s = str;
    while (*s) s++;
    return s - str;
}

static void raw_memset(void *b, int c, size_t len) {
    unsigned char *p = (unsigned char *)b;
    while (len--) *p++ = (unsigned char)c;
}

static int raw_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

static void raw_print(const char *str) {
    raw_syscall(SYS_WRITE, STDOUT_FILENO, (long)str, raw_strlen(str), 0, 0, 0);
}

static void raw_exit(int code) {
    raw_syscall(SYS_EXIT, (long)code, 0, 0, 0, 0, 0);
}

// --- Custom Execution Entry Point (No main/crt1.o) ---
void _start(void) {
    // 1. Create UDP socket using inline syscall 41 (SYS_socket)
    long sd = raw_syscall(SYS_SOCKET, AF_INET, SOCK_DGRAM, 0, 0, 0, 0);

    if (sd < 0) {
        raw_print("The socket is not opened!\n");
        raw_exit(1);
    }

    struct sockaddr_in target;
    raw_memset(&target, 0, sizeof(target));

    target.sin_family = AF_INET;
    target.sin_port   = 0xE914;            // Target port
    target.sin_addr.s_addr = 0x0100007F;    // 127.0.0.1 (Loopback)

    raw_print("\n###### The UDP client is begin ######\n");

    char buffer[BUFFER_SIZE];

    while (1) {
        raw_print("You: ");

        // 2. Read stdin directly via inline syscall 0 (SYS_read)
        long bytes_read = raw_syscall(SYS_READ, STDIN_FILENO, (long)buffer, BUFFER_SIZE - 1, 0, 0, 0);
        if (bytes_read <= 0) break;

        // Strip trailing newline character
        buffer[bytes_read] = '\0';
        for (long i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\r') {
                buffer[i] = '\0';
                bytes_read = i;
                break;
            }
        }

        if (raw_strcmp("exit", buffer) == 0) break;

        // 3. Send packet via inline syscall 44 (SYS_sendto)
        long send_byte = raw_syscall(SYS_SENDTO, sd, (long)buffer, bytes_read, 0, (long)&target, sizeof(target));

        if (send_byte < 0) {
            raw_print("Failed to send the message\n");
        }
    }

    // 4. Close socket via inline syscall 3 (SYS_close)
    raw_syscall(SYS_CLOSE, sd, 0, 0, 0, 0, 0);

    // 5. Clean exit via inline syscall 60 (SYS_exit)
    raw_exit(0);
}