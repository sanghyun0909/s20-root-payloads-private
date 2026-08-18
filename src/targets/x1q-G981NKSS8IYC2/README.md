# Galaxy S20 (x1q) - Target Profile

## Device Details

- **Model**: SM-G981N
- **Codename**: x1q
- **Processor**: Qualcomm Snapdragon 865
- **RAM**: 8 GB
- **Storage**: 128 GB (typical)
- **Android Version**: 13 (Tiramisu)
- **Kernel**: 4.19.113-27166950
- **Build**: TP1A.220624.014
- **Region**: Korea (KOO)

## Build Instructions

### Prerequisites

- Android NDK 21 or newer
- CMake 3.22.1+
- A recent Linux kernel (for tools) or macOS with appropriate toolchain
- Python 3.7+

### Compilation

```bash
# From Root-My-Galaxy-Payloads directory
make TARGET=x1q-G981NKSS8IYC2 ANDROID_NDK_HOME=/path/to/android-ndk
```

This produces three artifacts:

1. **cve-2026-43499-app.so** - App-space exploit payload
2. **cve-2026-43499-root** - Root privilege daemon
3. **kernelsu.ko** - KernelSU kernel module (4.19 variant)

### Build Output

```
build/x1q-G981NKSS8IYC2/
├── cve-2026-43499-app.so
├── cve-2026-43499-root
├── kernelsu.ko
└── [other build artifacts]
```

## Offset Validation

The offsets in `target.h` were calculated from:

- Live device memory analysis (4.19.113 kernel)
- Samsung firmware documentation
- Kernel structure analysis from vmlinux-to-elf recovery

**Status**: These offsets are baseline estimates. They should be validated against extracted firmware before production use. See [SM-G981N-G981NKSS8IYC2.md](../../docs/SM-G981N-G981NKSS8IYC2.md) for detailed validation procedures.

## Known Issues & Limitations

1. **p0_fingerprint.h is incomplete** - Contains placeholder values. Must be extracted from actual kernel binary for robust KASLR bypasses.

2. **Kernel Variants** - Other regional builds of S20 (XEF, ATU, TMB, etc.) may have different kernel offsets. A separate target profile is recommended for each major variant.

3. **Thermal Considerations** - 4.19 kernel on Snapdragon 865 can thermally throttle, affecting exploit timing. May require thermal management during exploitation.

## Testing

### On Device

1. **Prerequisites**: Shizuku installed and running, or existing root access
2. **Build app payload**
3. **Deploy to device**: `adb push cve-2026-43499-app.so /data/local/tmp/`
4. **Execute**:
   ```bash
   adb shell /data/local/tmp/cve-2026-43499-app.so
   ```
5. **Verify**: `adb shell id` should show uid=0

### Troubleshooting

- **Crash/Segfault**: Offset mismatch - verify target.h offsets
- **Timeout**: KASLR bypass failed - increase SLIDE_PSELECT_TIMEOUT_NSEC
- **Device Reboot**: Memory corruption error - verify MM_STRUCT_SZ and task structure offsets
- **Performance**: Thermal throttling - allow device to cool between attempts

## Future Maintenance

- This profile is specific to build **TP1A.220624.014**
- Future S20 firmware updates will require new profiles
- Monitor Samsung security patches - CVE-2026-43499 may be patched
- Kernel upgrades (4.19 → 5.10/5.15) require separate profiles

## Contributing

If you discover more accurate offsets through firmware analysis:

1. Document the extraction method
2. Provide kernel binary SHA256
3. Submit offsets with validation results
4. Include device test results

See root [PORTING.md](../../PORTING.md) for detailed procedure.
