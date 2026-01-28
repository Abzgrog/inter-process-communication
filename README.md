PROJECT DOCUMENTATION: IPC Communication System
1. Assignment Description
The goal of this project is to design and implement a system of cooperating processes within the Linux OS environment. These processes demonstrate various Inter-Process Communication (IPC) mechanisms. The task creates a data processing "pipeline" where the input consists of text files, and the output is a single aggregated file.
The system consists of a process chain:
P1, P2: Read words from input files.
Pr: (Provided binary) Processes inputs and forwards them.
T: Transfers data from a Pipe to Shared Memory.
S: (Provided binary) Processes data within Shared Memory.
D: Retrieves data from Shared Memory and sends it via TCP.
Serv1: (Provided binary) TCP server / UDP client; forwards data.
Serv2: UDP server; writes the final data into a file.
2. Specification of Constraints
The implementation adheres to the following technical constraints and specifications:
Limited Transfer Size: All data buffers are limited by the constant MAX_BUFFER (152 bytes).
Fixed IPC Identifiers: Shared memory and semaphores must use keys defined in common.h (1234, 5678, 1111, 2222) to guarantee compatibility with the provided binary modules (proc_s).
Synchronization: Processes P1 and P2 must not read data arbitrarily; they must be controlled by the SIGUSR1 signal (required by the Pr module).
Network Communication: Ports for TCP and UDP communication are not hardcoded. They are passed as command-line arguments when launching the main process.
Architecture: The solution is designed as a single control process (zadanie) that launches and manages all subprocesses.
3. Data Structures and Types
To ensure consistency across the entire project, the common.h header file is used. It defines:
shared_data_t: A structure for data transfer via shared memory. It contains a single array char text[MAX_BUFFER]. This ensures all processes access memory uniformly.
union semun: A necessary union for initializing semaphores using the semctl call (System V IPC standard).
IPC Keys: SHM_KEY_1/2 and SEM_KEY_1/2 serve to uniquely identify resources in the system.
Filenames: Constants FILE_P1, FILE_P2, and FILE_OUT define input and output paths.
4. Problem Analysis
The assignment presents a classic Producer-Consumer problem applied in a series (chain). The main challenge is the diversity of communication channels.
Data Flow:
File -> Pipe -> Shared Memory (SHM) -> TCP Socket -> UDP Socket -> File
Analysis of SHM Synchronization Options:
Option A (Busy Waiting): The process would constantly check a flag in memory.
Minus: Extreme CPU load.
Option B (Semaphores): Using System V semaphores to block the process until data is ready.
Plus: Efficient CPU usage; the process sleeps until it is its turn. This solution was chosen.
Analysis of Process Execution:
A process hierarchy was necessary.
Solution: One main process (zadanie.c) creates all IPC resources and subsequently launches workers using fork() and exec(). This ensures resources exist before processes attempt to connect to them.
5. Proposed Solution Description
a. Design
I chose a combination of pipes for local parent-child communication (between P1/P2, Pr, and T) and System V IPC (SHM + SEM) for communication between independent processes (T, S, D). Standard BSD sockets are used for the network section. The reasoning is robustness and the fact that semaphores provide atomic operations needed for safe synchronization of reading and writing in shared memory.
b. Data Structures
Semaphore Arrays: Each shared memory segment is protected by an array of two semaphores:
Index 0: Indicates "Free for Write" (Write Mutex).
Index 1: Indicates "Data Ready for Read" (Read Mutex).
Buffer: All processes use a statically allocated array char buffer[MAX_BUFFER] for string manipulation before transmission.
c. Synchronization Method
Synchronization is crucial to prevent data from being overwritten before it is read.
P1/P2: Synchronization via SIGUSR1. The process sleeps (pause()) and wakes up only after receiving a signal from process Pr.
T -> S -> D: Synchronization via Semaphores.
Producer (e.g., T): Waits for semaphore 0 (Wait for Write). Writes data. Signals semaphore 1 (Signal Read).
Consumer (e.g., S): Waits for semaphore 1 (Wait for Read). Reads data. Signals semaphore 0 (Signal Write).
Initialization of semaphores in zadanie.c is set to {1, 0}, meaning writing is allowed, reading is blocked.
d. Algorithms
Main Process Algorithm (zadanie.c):
Create IPC resources (SHM, SEM).
Initialize semaphores to values {1, 0}.
Create pipes for P1/P2->Pr and Pr->T.
Sequentially fork and launch processes (execl). Crucially, redirect stdin/stdout using dup2 for processes utilizing pipes.
Wait for child processes to finish (wait).
On exit (including Ctrl+C), clean up resources (shmctl, semctl with IPC_RMID).
Process T Algorithm (Writer to SHM):
Connect to SHM1 and SEM1.
Read stdin (redirected pipe from Pr).
semop(WAIT, index 0) -> Lock for write.
strcpy data to SHM.
semop(SIGNAL, index 1) -> Unlock for read.
Process D Algorithm (Reader from SHM + TCP Client):
Connect to SHM2 and SEM2.
Create TCP socket and connect to localhost:port1.
Loop:
semop(WAIT, index 1) -> Wait for data.
Read data, send via send().
semop(SIGNAL, index 0) -> Free space for next write.
Process Serv2 Algorithm (UDP Server):
Create UDP socket and bind to port2.
Infinite loop: recvfrom().
Append received data to file serv2.txt.
6. Conclusion and Evaluation
Time Complexity:
Implementation and debugging took approximately 8 hours. Most time was spent correctly configuring IPC keys and understanding the behavior of the binary modules (Pr, S), which required specific semaphore operations.
Evaluation and Limitations:
The resulting system is functional and stable. Data passes through the entire chain without loss and in the correct order.
Limitation: The use of INADDR_ANY and localhost in the code means the distributed part is simulated on a single machine. For actual network deployment, IP addresses would need to be parameterized.
Limitation: Absence of sophisticated error handling (e.g., if the network fails, processes simply terminate).
Result:
The output file serv2.txt contains correctly assembled strings, e.g., one-pr-s-d-serv1, confirming the successful synchronization of all 7 processes.
7. References
W. Richard Stevens: Advanced Programming in the UNIX Environment.
Linux Man pages: shmget(2), semop(2), pipe(2), socket(7).
Lecture materials on Operating Systems.
APPENDICES
8. System Manual (For Programmers)
The project is divided into modules, with common.h acting as the connecting element.
Module common.h
Defines data structures. Key is shared_data_t, which ensures binary compatibility of the memory layout between my processes and the provided binaries.
Module zadanie.c (Main)
This is the "Orchestrator".
void cleanup(): Function called upon termination (including SIGINT). Ensures IPC objects are removed from the system to prevent kernel memory leaks.
main(): Handles fork, dup2 (for pipes), and exec. The process creation logic is sequential, with short sleep(1) intervals to stabilize servers before clients start.
Modules proc_p1.c / proc_p2.c
Implement passive reading.
void my_handler(int): SIGUSR1 signal handler. Sets the global variable volatile sig_atomic_t ready_to_send, which is the only safe way to communicate with a signal handler.
Module proc_t.c
Translates the data stream into shared memory blocks. Uses sembuf structures for operations -1 (wait) and +1 (signal).
Module proc_serv2.c
A simple UDP server.
Opens the file in "a" (append) mode for every received packet. While less efficient than keeping the file open, it is safer regarding data consistency in case of a crash.
9. User Manual
Purpose:
The program demonstrates data flow through various IPC types. It processes the content of files p1.txt and p2.txt, saving the result to serv2.txt.
Dependencies:
OS Linux/Unix
GCC Compiler
Input files p1.txt, p2.txt in the same directory.
Executable files proc_pr, proc_s, proc_serv1 (provided in the assignment).
Compilation:
To build all modules, use the following commands (or make if a Makefile is present):
code
Bash
gcc zadanie.c -o zadanie
gcc proc_p1.c -o proc_p1
gcc proc_p2.c -o proc_p2
gcc proc_t.c -o proc_t
gcc proc_d.c -o proc_d
gcc proc_serv2.c -o proc_serv2
Execution:
The program requires two parameters: a TCP port and a UDP port.
code
Bash
./zadanie <port_tcp> <port_udp>
Example:
code
Bash
./zadanie 8080 9090
Operation Description:
Upon launch, the program automatically starts all subprocesses. You will see console output regarding server startup and data saving. The program runs until all inputs are processed or until terminated by the user.
Termination:
Press Ctrl+C in the terminal running zadanie. The program captures the signal, correctly terminates all subprocesses, and cleans up shared memory.
Output:
Check the serv2.txt file. It should contain the processed words.
10. Source Code
(Note: Source codes are attached in separate files as per the assignment instructions. Key files are listed below.)
common.h
zadanie.c
proc_p1.c / proc_p2.c
proc_t.c
proc_d.c
proc_serv2.c