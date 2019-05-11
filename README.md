# sysfs-msrs
A Linux kernel module that allows to read and write x86 CPU MSRs (machine-specific registers) via sysfs. This was created as a 
learning project and does not necessarily serve a useful purpose. The code is somewhat messy and there are some issues, e.g., 
write-only registers are not properly exposed and registers with ambiguous addresses are not correctly handled.

Currently only "Architectural MSRs" as defined by the _Intel® 64 and IA-32 Architectures Software Developer’s Manual_ are 
supported, though it can be easily extended for microarchitecture-specific MSRs by adding them to `msrs.h`.

## Build
```bash
$ git clone https://github.com/pp3345/sysfs-msrs.git
$ cd sysfs-msrs
$ make -C /path/to/linux/kernel/source M=$(pwd)
$ # OR (if building against distribution kernel, might require installing kernel-devel or similar package)
$ make -C /usr/lib/modules/`uname -r`/build M=$(pwd)
```

## Installation
```bash
$ sudo make -C /path/to/linux/kernel/source M=$(pwd) modules_install
$ # OR (if building against distribution kernel)
$ sudo make -C /usr/lib/modules/`uname -r`/build M=$(pwd) modules_install
```

## Usage
```bash
$ sudo modprobe sysfs-msrs
$ cd /sys/devices/system/cpu/cpuN/msrs # where N is the logical ID of the CPU you're interested in
$ ls -l
total 0
-rw-rw----. 1 root root 4096 May 11 17:30 IA32_APERF
-rw-rw----. 1 root root 4096 May 11 17:30 IA32_APIC_BASE
-rw-rw----. 1 root root 4096 May 11 17:30 IA32_A_PMC0
-rw-rw----. 1 root root 4096 May 11 17:30 IA32_A_PMC1
-rw-rw----. 1 root root 4096 May 11 17:30 IA32_A_PMC2
-rw-rw----. 1 root root 4096 May 11 17:30 IA32_A_PMC3
-rw-rw----. 1 root root 4096 May 11 17:30 IA32_BIOS_SIGN_ID
-rw-rw----. 1 root root 4096 May 11 17:30 IA32_BNDCFGS
-rw-rw----. 1 root root 4096 May 11 17:30 IA32_CLOCK_MODULATION
-rw-rw----. 1 root root 4096 May 11 17:30 IA32_CPU_DCA_CAP
-rw-rw----. 1 root root 4096 May 11 17:30 IA32_CSTAR
-rw-rw----. 1 root root 4096 May 11 17:30 IA32_DEBUGCTL
# ...
$ sudo cat IA32_FEATURE_CONTROL # read MSR
$ echo 1 | sudo tee IA32_PEBS_ENABLE # write MSR
```

**Use with caution.**
