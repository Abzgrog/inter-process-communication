IPC Data Processing Pipeline
![alt text](https://img.shields.io/badge/language-C-blue.svg)

![alt text](https://img.shields.io/badge/platform-Linux-lightgrey.svg)

![alt text](https://img.shields.io/badge/concepts-IPC%20%7C%20Sockets%20%7C%20Shared%20Memory-orange.svg)
📖 Overview
This project implements a complex system of cooperating processes within the Linux OS environment. It demonstrates various Inter-Process Communication (IPC) mechanisms by creating a data processing "pipeline".
The system reads text from input files, passes data through a chain of independent processes using different communication channels (Pipes, Shared Memory, TCP, UDP), and aggregates the result into a single output file.
Data Flow Architecture
The data travels through the following chain:
File → Pipe → Pr → Pipe → T → Shared Memory → S → Shared Memory → D → TCP → Serv1 → UDP → Serv2 → File
Process Roles
P1, P2: Read words from input files (p1.txt, p2.txt).
Pr: (Provided binary) Processes inputs and forwards them.
T: Transfers data from a Pipe to Shared Memory.
S: (Provided binary) Processes data within Shared Memory.
D: Retrieves data from Shared Memory and sends it via TCP.
Serv1: (Provided binary) TCP server / UDP client; forwards data.
Serv2: UDP server; writes the final data into serv2.txt.
🛠️ Technical Specifications
Constraints & Standards
The implementation adheres to strict technical constraints to ensure compatibility with provided binary modules:
Buffer Size: Fixed MAX_BUFFER constant (152 bytes).
IPC Identifiers: Fixed keys defined in common.h (1234, 5678, 1111, 2222) for Shared Memory and Semaphores.
Synchronization:
P1/P2: Controlled by SIGUSR1 signal.
Shared Memory: Controlled by System V Semaphores to prevent race conditions.
Network: Ports are dynamic and passed as CLI arguments.
Data Structures (common.h)
shared_data_t: Ensures uniform memory access. Contains a single array char text[MAX_BUFFER].
union semun: Used for semctl initialization (System V IPC standard).
🚀 implementation Details
Synchronization Logic (Producer-Consumer)
The project solves the Producer-Consumer problem in a chain using System V Semaphores.
Each Shared Memory segment is protected by an array of two semaphores:
Index 0 (Write Mutex): Indicates "Free for Write".
Index 1 (Read Mutex): Indicates "Data Ready for Read".
Algorithm:
Producer (e.g., T): WAIT(0) → Write Data → SIGNAL(1)
Consumer (e.g., S): WAIT(1) → Read Data → SIGNAL(0)
Initial state: {1, 0} (Write allowed, Read blocked).
Process Algorithms
Main Orchestrator (zadanie.c)
Creates IPC resources (SHM, SEM).
Initializes semaphores.
Creates Pipes.
Forks and executes all worker processes using dup2 for pipe redirection.
Cleans up resources (SHM/SEM) upon termination (SIGINT or completion).
Process T (Pipe → SHM)
Reads from stdin (redirected pipe).
Locks Write Mutex.
Writes to Shared Memory.
Unlocks Read Mutex.
Process D (SHM → TCP)
Connects to Shared Memory.
Establishes TCP connection to localhost.
Waits for Read Mutex.
Reads data and sends via send().
Unlocks Write Mutex.
Process Serv2 (UDP → File)
Binds UDP socket.
Receives data via recvfrom().
Appends data to serv2.txt.
💻 Build & Usage
Dependencies
Linux/Unix OS
GCC Compiler
Provided binaries: proc_pr, proc_s, proc_serv1
Input files: p1.txt, p2.txt
Compilation
Compile all modules manually or use a Makefile:
code
Bash
gcc zadanie.c -o zadanie
gcc proc_p1.c -o proc_p1
gcc proc_p2.c -o proc_p2
gcc proc_t.c -o proc_t
gcc proc_d.c -o proc_d
gcc proc_serv2.c -o proc_serv2
Execution
Run the main orchestrator with TCP and UDP ports:
code
Bash
./zadanie <port_tcp> <port_udp>
Example:
code
Bash
./zadanie 8080 9090
To stop the program, press Ctrl+C. The handler will automatically clean up shared memory and semaphores.
📂 Project Structure
File	Description
common.h	Shared header with IPC keys, structs, and constants.
zadanie.c	Main controller. Handles fork, exec, pipe creation, and cleanup.
proc_p1.c/p2.c	Input readers. synchronized via SIGUSR1.
proc_t.c	Translator: Reads from Pipe, writes to Shared Memory.
proc_d.c	Dispatcher: Reads from Shared Memory, sends via TCP.
proc_serv2.c	Final server: Receives UDP, writes to file.
proc_pr	Binary: Processor module.
proc_s	Binary: SHM Processor module.
proc_serv1	Binary: Network Bridge (TCP->UDP).
📝 Evaluation & Limitations
Status: Functional and Stable.
Result: The output serv2.txt demonstrates correct ordering of data (e.g., one-pr-s-d-serv1), confirming successful synchronization of 7 concurrent processes.
Limitations:
Network execution is simulated on localhost (uses INADDR_ANY).
Error handling focuses on resource cleanup rather than network recovery.
Complexity: Required deep understanding of System V IPC, signal handling, and file descriptor manipulation (dup2).
📚 References
Advanced Programming in the UNIX Environment (W. Richard Stevens)

Linux Man pages: shmget(2), semop(2), pipe(2), socket(7)
