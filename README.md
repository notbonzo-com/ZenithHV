# ZenithHV: A Minimal and Undetectable Hypervisor

## 📌 Overview

ZenithHV is a lightweight, **bare-metal hypervisor** designed to be as **undetectable and minimal as possible** while allowing users to write and load custom modules to **supervise guest systems**. It fully **passes through hardware**, making it nearly invisible to the guest OS while still enabling powerful debugging and monitoring capabilities.
ZenithHV is built in **C**, uses the **Limine bootloader**, and is designed to support **OVMF for UEFI booting**. It also aims to implement **nested virtualization** for running hypervisors inside guests.

## 🚀 Features

✔ **Minimal Footprint** – No unnecessary features, just a thin passthrough layer.

✔ **Undetectable** – Aims to be invisible to guests while still providing control.

✔ **Full Hardware** Passthrough – Guests interact with real hardware with minimal interference.

✔ **Modular Supervision** – Loadable modules allow flexible guest supervision.

✔ **Nested Virtualization** Support – Run hypervisors inside the guest OS and support hypervisor based security.


## 🏗 Building & Running

### 🔧 Dependencies

- **GCC** (with cross-compilation support for x86_64 ELF)
- **Git** (for cloning the limine repository)
- **OVMF** (for UEFI booting)
- **NASM** (for assembling low-level code)
- **Make** (for automated builds)
- **QEMU** (for runing virtualised)
- **MTools and gptfdisk** (for creating the image)

### 🛠 Building ZenithHV

#### 1️⃣ Clone the repository:

```
git clone https://github.com/notbonzo-com/ZenithHV.git
cd ZenithHV
```

#### 2️⃣ Build the hypervisor:

```
make; make reset
```

### 🖥 Running in QEMU

#### 🏴 Legacy BIOS Mode

Run with **BIOS emulation** (faster testing):

```
make run
```

#### 🌍 UEFI Mode (OVMF)

Run with **OVMF UEFI firmware**:

```
qemu-system-x86_64 -cpu host -smp 4 -m 512M -enable-kvm -drive file=build/image.hdd,format=raw -debugcon stdio -bios /usr/share/ovmf/OVMF.fd
```

## 🛠 Debugging with GDB

To attach **GDB** to the hypervisor: 1️⃣ Start QEMU with debugging enabled:

```
qemu-system-x86_64 -cpu host -smp 4 -m 512M -enable-kvm -drive file=build/image.hdd,format=raw -serial stdio -s -S
```

2️⃣ In another terminal, connect **GDB**:

```
gdb build/kernel.bin
```

3️⃣ Attach to QEMU:

```
target remote :1234
```

Now you can set breakpoints and step through execution.

## 🔍 Current Issues

❌ None of the hypervisor featrues are implemented.
❌ Nested Virtualization not yet implemented (planned feature).

## 📜 License

ZenithHV is licensed under the **MIT License**. See [LICENSE](LICENSE.md) for details.

## 🤝 Contributing to ZenithHV

Thank you for your interest in contributing to **ZenithHV**! 🎉
We welcome contributions of all kinds, including code, documentation, bug reports, feature requests, and testing.

### 📌 How to Contribute

#### 🔍 Finding Issues

Check the Issues tab for **open issues** and **feature requests**.
If you have an idea, feel free to **open a new issue** before implementing it.

#### 🔀 Creating a New Branch

Always work on a new branch instead of **main**:

```
git checkout -b my-name-my-feature
```

#### ✅ Writing Code

- Follow C23 coding style.
- Keep the hypervisor minimal & undetectable.
- Use meaningful commit messages.
- Test your code in QEMU before submitting.

#### 🔃 Submitting a Pull Request (PR)

1. Commit your changes
2. Push your branch
3. Open a Pull Request on GitHub.

### 📝 Reporting Bugs

If you encounter a bug, please open an issue and include:

- A detailed description of the problem.
- Steps to reproduce the issue.
- Any error messages or logs.
- Your host system setup (Linux distro, CPU, QEMU version).

### 💡 Suggesting Features

Want a new feature? Open an issue with:

- A clear explanation of the feature.
- How it benefits ZenithHV.
- Any technical considerations.

### 📜 License

By contributing, you agree that your code will be released under the **MIT License**.

