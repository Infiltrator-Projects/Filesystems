# Independent EFI filesystem drivers

Filesystems owns the firmware filesystem adapters in this directory. Boot
contains their generic UEFI loader, but does not compile, vendor or package any
filesystem driver. Drivers implement EFI Driver Binding and EFI Simple File
System and can be installed alongside any compatible boot manager.

The `filesystems/` source and its licences were moved from Boot commit
`9043feba1b72e41969dbf26c5a65aedbea0fe0bc`. `IMPORT.json` records each original
source path and SHA-256 digest. The implementation files were preserved byte
for byte, including the dormant generic filesystem adapters. `Make.common`, the platform headers and the source credits/licences
provide the imported build support; no Boot source checkout is required.

Build the maintained x86-64 driver payload with GNU-EFI, GCC, make and binutils:

```sh
bash scripts/build-efi-drivers.sh build-efi
```

The established ext4 reader is emitted as `drivers_x64/ext4_x64.efi`.
The independent OFS implementation is built from `native/filesystems/ofs/` and
emitted as `experimental/OFS_x64.efi`. Its experimental status remains explicit.
**InfiltratorFS owns its own Format 0.19 UEFI driver** in the [InfiltratorFS](https://github.com/Infiltrator-Projects/InfiltratorFS) repository. The obsolete 0.18 InfiltratorFS firmware adapter and vendored parser were removed from Filesystems after their historical code was recovered to InfiltratorFS. Filesystems does not compile or publish that filesystem driver.

For Debian release builds, configure CMake with
`-DFILESYSTEMS_BUILD_EFI_DRIVERS=ON`. Filesystems installs its driver payload under
`/usr/lib/filesystems/efi/`. This is component storage, not an EFI installation:
neither building nor installing the package writes to an ESP, modifies NVRAM,
changes Boot configuration or installs a kernel. Setup, or an explicit local
test, selects the required drivers and places them in the chosen boot manager's
driver directory. There are no machine-specific destinations in this build.

The imported makefiles and EDK2 metadata retain their historical names and
upstream assumptions. The script above is the maintained independent build;
other adapters/toolchains require their own qualification before distribution.
