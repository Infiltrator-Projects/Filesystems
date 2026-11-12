# Independent filesystem Common V2 migration — released binary evidence

This is an execution-backed status ledger, **not** a claim that all filesystems
have working Common V2 defragmentation. The authoritative audit is the [all
23-provider GitHub workflow](https://github.com/Infiltrator-Projects/Filesystems/actions/workflows/independent-v2-audit.yml),
which unpacks actual **published** filesystem DEBs and inspects each exported
ELF symbol and Common V2 group table in an isolated C executable. It never
opens, writes, formats or mutates any user device.

## Verified public ABI state, 13 November 2026

| Independent repository | Published SO ABI at initial audit | Common V2 capabilities demonstrated by descriptor | Blocking work |
|---|---|---|---|
| HFS | V2 only | Probe, read-only volume query | Files and V2 defrag |
| ZFS | V2 + V1 | Probe | Native V2 volume, map and mutation recovery |
| XFS | V2 + V1 | Probe, read-only volume query | Source-owned extent map, V2 defrag, retire V1 |
| NTFS | V2 + V1 | Probe, read-only volume query | Native extent map, V2 defrag and recover |
| SFS | V2 only | Probe, read-only volume query | V2 map and transactional defrag |
| FFS | V2 + V1 | Core operations, volume, read-only defrag map | V2 plan, execute, verify, recover; retire V1 |
| Minix | V2 + V1 | Core operations, volume, read-only defrag map | V2 relocation/recovery; retire V1 |
| HFSPlus | V2 only | Probe, read-only volume query | Native V2 file map and defrag |
| Btrfs | V2 + V1 | Probe, read-only volume query | Source-owned CoW-safe map/writer/recover; retire V1 |
| Swap | V2 only | Probe, volume formatter | No mounted file namespace; preserve correct SWAP semantics |
| APFS | V2 + V1 | Probe, read-only volume query | APFS checkpoint/snapshot-safe map, writer/recover |
| OFS | V2 + V1 | Core, volume operations | V2 defrag group; retire V1 after complete parity |
| exFAT | Missing V2 in **old published** 0.1.12 | Current main contains V2 core and volume code | Qualify and publish 0.1.13 package, then audit actual SO |
| EXT4 | V2 + V1 | Probe, read-only volume query | Journal/extent-aware V2 defrag and recovery |
| FAT32 | Missing V2 in **old published** 0.1.12 | Current main contains V2 core and volume code | Qualify and publish 0.1.13 package, then retire legacy |
| FAT16 | Missing V2 in **old published** 0.1.16 | Current main contains V2 core and volume code | Qualify and publish 0.1.17 package, then retire legacy |
| UFS1 | V2 only | Probe, volume query, read-only file extent map | V2 mutation and recovery; more real-image coverage |
| SFS2 | V2 only | Probe, read-only volume query | V2 file map and transactional defrag |
| PFS3 | V2 only | Probe, read-only volume query | V2 file map and transactional defrag |
| FAT12 | V2 + V1 | Core and volume operations | Retire old V1 after Boot/Defragmenter V2 parity |
| EXT3 | V2 + V1 | Probe, read-only volume query | Journal-safe V2 map/writer/recover |
| EXT2 | V2 only | Core operations, volume query, read-only defrag map | V2 writer/recover with integrity qualification |
| UFS2 | V2 only | Probe, volume query, read-only file extent map | V2 mutation/recovery and power-loss qualification |

The initial audit run [43316587698](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/43316587698)
found **9** Common V2-only providers, **11** providers exporting Common V2
*and* historical V1, and **3** old released packages without Common V2.
This is based on actual released shared objects, not a grep of source tree or a
passing historical V1 integration workflow.

## Release-source corrections in progress

- SWAP 1.0.10 is published and validated; it fixes duplicate hard-coded
  on-disk UUIDs and retains a strictly Common V2 ABI.
- exFAT 0.1.13, FAT16 0.1.17 and FAT32 0.1.13 were committed as new release
  candidates. Their pipelines must check the actual standalone DEB and RUN
  exports; do **not** mark their published ABI corrected until the immutable
  releases and this audit confirm it.
- V2-only is a **public Common ABI requirement**. A filesystem that still has
  a working legacy V1 writer is not automatically safe to delete. Implement
  its true filesystem-owned V2 operations, test on disposable images, verify
  rollback/interrupt/recover, migrate every Boot/Files/Defragmenter client,
  then remove the legacy exports and audit the new built SO.
- Compiling, publishing a `.ko`, or passing a V1 worker's old fixture tests
  does **not** qualify V2 file relocation, power-loss recovery or deployment
  on an untested kernel.

The audit's automatic regression check currently guards the nine already
strict Common V2-only public ABIs. It deliberately lists the other fourteen as
migration work rather than falsifying a green whole-system completion.
