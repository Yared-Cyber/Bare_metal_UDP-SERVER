
# Bare-Metal UDP Echo Server

A high-performance, raw Linux UDP server written in C. This project bypasses high-level application frameworks to interface directly with the Linux kernel's socket API (`socket()`, `bind()`, `recvfrom()`, `sendto()`). It processes raw network packets, reverses the payload byte-by-byte in memory without using high-level string libraries, and mirrors the data back to the client.

---

## Features

* **Direct Kernel Interface:** Built using low-level POSIX/Linux socket APIs.
* **In-Memory Payload Reversal:** Reverses arbitrary byte streams directly in memory without relying on standard string libraries (safe for binary data and non-null-terminated payloads).
* **Zero-Allocation Echo:** Instantly routes responses back to the sender's exact IPv4 address and ephemeral port.
* **Minimal Footprint:** No external dependencies—just standard C library headers and pure Linux system calls.

---
