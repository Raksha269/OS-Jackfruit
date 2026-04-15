# Mini Container Runtime (OS Jackfruit Project)

## 👩‍💻 Overview
This project implements a minimal container runtime in C along with a Linux kernel module for memory monitoring.

The runtime (engine) allows users to create and manage lightweight containers using Linux namespaces, while the kernel module enforces memory limits on containers.

---

## 🏗️ Architecture

### 🔹 User-space (engine)
- Handles container lifecycle (start, run, stop)
- Uses `clone()` for namespace isolation
- Communicates via UNIX domain sockets

### 🔹 Kernel-space (monitor module)
- Tracks memory usage (RSS)
- Enforces soft and hard limits
- Uses `ioctl` for communication

---

## ⚙️ Setup

```bash
make
sudo ./environment-check.sh
sudo insmod monitor.ko
```

## 📦 Root Filesystem Setup

```bash
mkdir rootfs
cd rootfs
wget https://dl-cdn.alpinelinux.org/alpine/latest-stable/releases/x86_64/alpine-minirootfs-latest-x86_64.tar.gz
tar -xvf alpine-minirootfs-latest-x86_64.tar.gz
cd ..
sudo cp cpu_hog memory_hog io_pulse rootfs/
```

## 🚀 Running the System

Start supervisor:
```bash
sudo ./engine supervisor rootfs
```

---

## ✅ Task 1: Multi-container Runtime

**Description:** Multiple containers are created using Linux namespaces.

### Commands
```bash
sudo ./engine start c1 rootfs "/cpu_hog 30"
sudo ./engine start c2 rootfs "/cpu_hog 30"
sudo ./engine ps
```

**Output:**
- Containers c1 and c2 created successfully
- Process list displays active containers

---

## ✅ Task 2: CLI Commands

**Description:** Core command-line interface for container management.

### Implemented Commands
- `start` - Create and start a container
- `run` - Run a container in foreground
- `ps` - List running containers
- `logs` - View container logs
- `stop` - Stop a running container

### Examples
```bash
sudo ./engine ps
sudo ./engine logs c1
sudo ./engine stop c1
```

---

## ✅ Task 3: Logging System

**Description:** Producer-consumer model for container output logging.

### Features
- Container output stored in log files
- Asynchronous log collection
- Persistent logging system

### Example
```bash
sudo ./engine run c3 rootfs "/cpu_hog 5"
cat logs/c3.log
```

---

## ✅ Task 4: Kernel Memory Monitor

**Description:** Kernel module tracks and enforces memory limits on containers.

### Features
- Tracks memory usage (RSS)
- Soft limit → warning message
- Hard limit → SIGKILL

### Example
```bash
sudo ./engine run m1 rootfs "/memory_hog 8 500"
sudo dmesg | tail
```

---

## ✅ Task 5: CPU Scheduling (nice)

**Description:** CPU priority scheduling using the nice flag.

### Features
- `--nice` flag adjusts process priority
- Higher nice value = lower priority
- Negative nice values require root privileges

### Examples
```bash
sudo nice -n -5 ./cpu
sudo nice -n 10 ./cpu
top
```


## ✅ Task 6: Clean Shutdown
Supervisor gracefully stops containers

Press:

Ctrl + C

## 📌 Conclusion

### This project demonstrates:
-Process isolation using namespaces
-Kernel-user communication via ioctl
-Resource control using kernel module
-Multi-container execution

---

## 🧠 Engineering Analysis

### 🔹 Design Decisions

1. **Use of Linux Namespaces**
   - Containers are implemented using `clone()` with PID, UTS, and mount namespaces.
   - This provides process isolation similar to lightweight containers.

2. **Separation of User-space and Kernel-space**
   - The runtime (`engine`) operates in user space.
   - Memory monitoring is handled in kernel space using a custom module.
   - This separation improves modularity and system safety.

3. **UNIX Domain Socket Communication**
   - The CLI communicates with the supervisor using UNIX domain sockets.
   - This enables efficient inter-process communication on the same machine.

4. **Producer-Consumer Logging System**
   - A bounded buffer is used between container output and log writer thread.
   - This avoids blocking and ensures efficient logging.

---

### 🔹 Trade-offs

1. **Static Binaries**
   - Workloads are compiled statically to run inside minimal rootfs.
   - Trade-off: larger binary size vs portability.

2. **Polling-based Memory Monitoring**
   - Kernel module periodically checks memory usage.
   - Trade-off: slight overhead vs simplicity of implementation.

3. **Simplified Container Features**
   - No full filesystem isolation or networking.
   - Trade-off: reduced complexity vs fewer features.

---

### 🔹 Challenges Faced

1. **Root Filesystem Issues**
   - Ensuring correct binaries inside rootfs.
   - Fix: used Alpine minirootfs.

2. **Permission Errors**
   - Supervisor and client required `sudo` due to namespace and socket permissions.

3. **Command Execution Failures**
   - Relative paths failed inside container.
   - Fix: used absolute paths like `/cpu_hog`.

4. **Kernel Logging Access**
   - `dmesg` required root privileges.

---

### 🔹 Performance Observations

1. **CPU Scheduling**
   - Containers with lower nice values (-5) received more CPU time.
   - Higher nice values (10) resulted in reduced CPU share.

2. **Memory Enforcement**
   - Memory hog processes were terminated when exceeding hard limits.
   - Demonstrates effective kernel-level resource control.

---

### 🔹 Limitations

- No container networking
- Limited filesystem isolation
- No cgroups integration
- Basic scheduling control (nice only)

---

### 🔹 Future Improvements

- Add cgroups for better resource control
- Implement container networking
- Add container image support
- Improve CLI with more commands

Authors
Nandhitha, Raksha, Priyanka
# Multi-Container Runtime

A lightweight Linux container runtime in C with a long-running supervisor and a kernel-space memory monitor.

Read [`project-guide.md`](project-guide.md) for the full project specification.

---

## Getting Started

### 1. Fork the Repository

1. Go to [github.com/shivangjhalani/OS-Jackfruit](https://github.com/shivangjhalani/OS-Jackfruit)
2. Click **Fork** (top-right)
3. Clone your fork:

```bash
git clone https://github.com/<your-username>/OS-Jackfruit.git
cd OS-Jackfruit
```

### 2. Set Up Your VM

You need an **Ubuntu 22.04 or 24.04** VM with **Secure Boot OFF**. WSL will not work.

Install dependencies:

```bash
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r)
```

### 3. Run the Environment Check

```bash
cd boilerplate
chmod +x environment-check.sh
sudo ./environment-check.sh
```

Fix any issues reported before moving on.

### 4. Prepare the Root Filesystem

```bash
mkdir rootfs-base
wget https://dl-cdn.alpinelinux.org/alpine/v3.20/releases/x86_64/alpine-minirootfs-3.20.3-x86_64.tar.gz
tar -xzf alpine-minirootfs-3.20.3-x86_64.tar.gz -C rootfs-base

# Make one writable copy per container you plan to run
cp -a ./rootfs-base ./rootfs-alpha
cp -a ./rootfs-base ./rootfs-beta
```

Do not commit `rootfs-base/` or `rootfs-*` directories to your repository.

### 5. Understand the Boilerplate

The `boilerplate/` folder contains starter files:

| File                   | Purpose                                             |
| ---------------------- | --------------------------------------------------- |
| `engine.c`             | User-space runtime and supervisor skeleton          |
| `monitor.c`            | Kernel module skeleton                              |
| `monitor_ioctl.h`      | Shared ioctl command definitions                    |
| `Makefile`             | Build targets for both user-space and kernel module |
| `cpu_hog.c`            | CPU-bound test workload                             |
| `io_pulse.c`           | I/O-bound test workload                             |
| `memory_hog.c`         | Memory-consuming test workload                      |
| `environment-check.sh` | VM environment preflight check                      |

Use these as your starting point. You are free to restructure the repository however you want — the submission requirements are listed in the project guide.

### 6. Build and Verify

```bash
cd boilerplate
make
```

If this compiles without errors, your environment is ready.

### 7. GitHub Actions Smoke Check

Your fork will inherit a minimal GitHub Actions workflow from this repository.

That workflow only performs CI-safe checks:

- `make -C boilerplate ci`
- user-space binary compilation (`engine`, `memory_hog`, `cpu_hog`, `io_pulse`)
- `./boilerplate/engine` with no arguments must print usage and exit with a non-zero status

The CI-safe build command is:

```bash
make -C boilerplate ci
```

This smoke check does not test kernel-module loading, supervisor runtime behavior, or container execution.

---

## What to Do Next

Read [`project-guide.md`](project-guide.md) end to end. It contains:

- The six implementation tasks (multi-container runtime, CLI, logging, kernel monitor, scheduling experiments, cleanup)
- The engineering analysis you must write
- The exact submission requirements, including what your `README.md` must contain (screenshots, analysis, design decisions)

Your fork's `README.md` should be replaced with your own project documentation as described in the submission package section of the project guide. (As in get rid of all the above content and replace with your README.md)
