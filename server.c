typedef unsigned long size_t;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

// x86_64 Linux System Call Numbers
#define SYS_READ    0
#define SYS_WRITE   1
#define SYS_CLOSE   3
#define SYS_SOCKET  41
#define SYS_SENDTO  44
#define SYS_EXIT    60

// Socket Constants
#define AF_INET     2
#define SOCK_DGRAM  2

#define STDIN_FILENO  0
#define STDOUT_FILENO 1

#define BUFFER_SIZE 1024

// Custom Minimal Network Structures
struct in_addr {
    unsigned int s_addr;
};

struct sockaddr_in {
    unsigned short sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;
    char           sin_zero[8];
};

// Raw Inline Assembly Syscall Wrapper (x86_64 ABI)
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

// Freestanding Helper Functions
static size_t raw_strlen(const char *str) {
    const char *s = str;
    while (*s) s++;
    return s - str;
}

static void raw_memset(void *b, int c, size_t len) {
    unsigned char *p = (unsigned char *)b;
    while (len--) *p++ = (unsigned char)c;
}

static void raw_memcpy(void *dest, const void *src, size_t len) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (len--) *d++ = *s++;
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


static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static const uint8_t Rcon[11] = { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };

static void aes_key_expansion(const uint8_t *key, uint8_t *w) {
    for (int i = 0; i < 16; i++) w[i] = key[i];
    for (int i = 4; i < 44; i++) {
        uint8_t temp[4];
        raw_memcpy(temp, &w[(i - 1) * 4], 4);
        if (i % 4 == 0) {
            uint8_t t = temp[0];
            temp[0] = sbox[temp[1]] ^ Rcon[i / 4];
            temp[1] = sbox[temp[2]];
            temp[2] = sbox[temp[3]];
            temp[3] = sbox[t];
        }
        for (int j = 0; j < 4; j++) {
            w[i * 4 + j] = w[(i - 4) * 4 + j] ^ temp[j];
        }
    }
}

static uint8_t xtime(uint8_t x) {
    return (x << 1) ^ ((x >> 7) * 0x1b);
}

static void aes_encrypt_block(const uint8_t *in, uint8_t *out, const uint8_t *w) {
    uint8_t state[16];
    raw_memcpy(state, in, 16);

    for (int i = 0; i < 16; i++) state[i] ^= w[i];

    for (int round = 1; round <= 10; round++) {
        uint8_t tmp[16];
        // SubBytes & ShiftRows
        tmp[0]  = sbox[state[0]];  tmp[4]  = sbox[state[4]];  tmp[8]  = sbox[state[8]];  tmp[12] = sbox[state[12]];
        tmp[1]  = sbox[state[5]];  tmp[5]  = sbox[state[9]];  tmp[9]  = sbox[state[13]]; tmp[13] = sbox[state[1]];
        tmp[2]  = sbox[state[10]]; tmp[6]  = sbox[state[14]]; tmp[10] = sbox[state[2]];  tmp[14] = sbox[state[6]];
        tmp[3]  = sbox[state[15]]; tmp[7]  = sbox[state[3]];  tmp[11] = sbox[state[7]];  tmp[15] = sbox[state[11]];

        if (round < 10) {
            // MixColumns
            for (int i = 0; i < 4; i++) {
                uint8_t a = tmp[i*4], b = tmp[i*4+1], c = tmp[i*4+2], d = tmp[i*4+3];
                uint8_t e = a ^ b ^ c ^ d;
                state[i*4]   = a ^ e ^ xtime(a ^ b);
                state[i*4+1] = b ^ e ^ xtime(b ^ c);
                state[i*4+2] = c ^ e ^ xtime(c ^ d);
                state[i*4+3] = d ^ e ^ xtime(d ^ a);
            }
        } else {
            raw_memcpy(state, tmp, 16);
        }

        for (int i = 0; i < 16; i++) state[i] ^= w[round * 16 + i];
    }
    raw_memcpy(out, state, 16);
}

// AES CTR Mode (Encrypts/Decrypts in-place)
static void aes_ctr_crypt(uint8_t *data, size_t len, const uint8_t *key, uint8_t counter[16]) {
    uint8_t round_keys[176];
    uint8_t keystream[16];
    uint8_t ctr_copy[16];
    
    aes_key_expansion(key, round_keys);
    raw_memcpy(ctr_copy, counter, 16);

    for (size_t i = 0; i < len; i++) {
        if (i % 16 == 0) {
            aes_encrypt_block(ctr_copy, keystream, round_keys);
            // Increment 128-bit Big-Endian counter
            for (int j = 15; j >= 0; j--) {
                if (++ctr_copy[j] != 0) break;
            }
        }
        data[i] ^= keystream[i % 16];
    }
}

void _start(void) {
    long sd = raw_syscall(SYS_SOCKET, AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
    if (sd < 0) {
        raw_print("The socket is not opened!\n");
        raw_exit(1);
    }

    struct sockaddr_in target;
    raw_memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port   = 0xE914;               // Port 5353 (in Network Byte Order / Big-Endian)
    target.sin_addr.s_addr = 0x0100007F;       // 127.0.0.1 (Loopback)

    // Shared 128-bit (16-byte) Key
    uint8_t secret_key[16] = "SecretAESKey1234";

    // 16-byte Initialization Vector (Nonce/Counter base)
    uint8_t nonce[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
                         0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    raw_print("\n###### Encrypted UDP Client Active ######\n");

    char input_buffer[BUFFER_SIZE];
    uint8_t packet_buffer[BUFFER_SIZE + 16];

    while (1) {
        raw_print("You (Encrypted): ");

        long bytes_read = raw_syscall(SYS_READ, STDIN_FILENO, (long)input_buffer, BUFFER_SIZE - 1, 0, 0, 0);
        if (bytes_read <= 0) break;

        // Strip trailing newline character
        for (long i = 0; i < bytes_read; i++) {
            if (input_buffer[i] == '\n' || input_buffer[i] == '\r') {
                input_buffer[i] = '\0';
                bytes_read = i;
                break;
            }
        }

        if (raw_strcmp("exit", input_buffer) == 0) break;

        // Prep Packet: [ 16 Bytes IV ] + [ Encrypted Payload ]
        raw_memcpy(packet_buffer, nonce, 16);
        raw_memcpy(packet_buffer + 16, input_buffer, bytes_read);

        // Encrypt the payload in-place (starting at index 16)
        aes_ctr_crypt(packet_buffer + 16, bytes_read, secret_key, nonce);

        long total_send_len = bytes_read + 16;
        long send_byte = raw_syscall(SYS_SENDTO, sd, (long)packet_buffer, total_send_len, 0, (long)&target, sizeof(target));

        if (send_byte < 0) {
            raw_print("Failed to send encrypted message\n");
        }
    }

    raw_syscall(SYS_CLOSE, sd, 0, 0, 0, 0, 0);
    raw_exit(0);
}