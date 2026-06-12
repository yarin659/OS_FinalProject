# OS_FinalProject
Final Project in OS course.


# Graph Movement Simulation - OS Project
**Authors:** Yarin Cohen, Abigail Weill, Niv Cohen, Itay Ono

## IPC Mechanism
We use unnamed pipes and interprocess semaphores.
Communication relies on the ipc_msg_t struct.

Semaphores are stored in a shared memory region so all forked
subprocesses share the same memory for them.

## Compilation and Execution
To compile and run the project, use the following commands:
(If using the makefile, you must have Raylib 5.5 or another
compatible version installed).

### Milestones 1, 2, 3, 4, 5
As per the relevant post on the course's forum we are no longer
required to host the relevant milestones in a uniform makefile.
As a result of that, to switch to a previous milestone, please
utilize the relevant git tag.

This is important - do **not** attempt to build previous milestones
without switching to the relevant git tag first.

### Milestone 6
```bash
make milestone6
./sim [file]
```
