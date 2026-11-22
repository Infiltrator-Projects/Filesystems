#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Audit release payloads from ALL independently owned filesystem repositories.
# No formats are synthesized; released ELF files are the authority.
# No Python, no host device access, no package installation.
set -euo pipefail
: "${GH_TOKEN:?A read-only GH_TOKEN is required}"
auditor="${1:?Pass the compiled Common V2 ABI inspector executable}"
report="${2:-filesystem-common-v2-release-audit.tsv}"
work="$(mktemp -d)"
trap 'rm -rf -- "$work"' EXIT
formats=(
  HFS ZFS XFS NTFS SFS FFS Minix HFSPlus Btrfs Swap APFS OFS exFAT
  EXT4 FAT32 FAT16 UFS1 SFS2 PFS3 FAT12 EXT3 EXT2 UFS2
)
printf 'repository\trelease_tag\tso_name\tstatus\tformat_id\truntime_profiles\tcore_caps\tvolume_caps\tdefrag_caps\tlegacy_v1_exports\n' > "$report"
for fmt in "${formats[@]}"; do
  repo="Infiltrator-Projects/Filesystem-${fmt}"
  dir="$work/$fmt"
  mkdir -p "$dir/assets" "$dir/unpacked"
  tag="$(gh api "repos/$repo/releases/latest" --jq '.tag_name' 2>/dev/null || true)"
  if [[ -z "$tag" ]]; then
    printf '%s\t-\t-\tNO_RELEASE\t-\t-\t-\t-\t-\t-\n' "$repo" >> "$report"
    continue
  fi
  if ! gh release download "$tag" -R "$repo" --pattern '*.deb' --dir "$dir/assets" >/dev/null 2>&1; then
    printf '%s\t%s\t-\tNO_DEB_RELEASE_ASSET\t-\t-\t-\t-\t-\t-\n' "$repo" "$tag" >> "$report"
    continue
  fi
  shopt -s nullglob
  debs=("$dir/assets"/*.deb)
  shopt -u nullglob
  if (( ${#debs[@]} != 1 )); then
    printf '%s\t%s\t-\tAMBIGUOUS_DEB_ASSETS\t-\t-\t-\t-\t-\t-\n' "$repo" "$tag" >> "$report"
    continue
  fi
  if ! dpkg-deb -x "${debs[0]}" "$dir/unpacked"; then
    printf '%s\t%s\t-\tUNPACK_FAILED\t-\t-\t-\t-\t-\t-\n' "$repo" "$tag" >> "$report"
    continue
  fi
  mapfile -d '' sos < <(find "$dir/unpacked" -type f -path '*/Filesystems/*.so' -print0)
  if (( ${#sos[@]} != 1 )); then
    printf '%s\t%s\t-\tEXPECTED_ONE_FILESYSTEM_SO_GOT_%d\t-\t-\t-\t-\t-\t-\n' "$repo" "$tag" "${#sos[@]}" >> "$report"
    continue
  fi
  so="${sos[0]}"
  exports="$(nm -D --defined-only "$so" 2>/dev/null || true)"
  legacy="$(printf '%s\n' "$exports" | grep -Ec 'infiltrator_filesystem_.*_v(1|3|4|5)|infiltrator_filesystem_module_v[135]' || true)"
  # A module with V2 and any other globally visible symbol is not V2-only,
  # regardless of whether that second symbol is a known retired interface.
  unexpected="$(printf '%s\n' "$exports" | awk 'NF >= 3 {print $3}' |
    sed 's/@.*//' | grep -vFx 'infiltrator_filesystem_api_v2' |
    LC_ALL=C sort -u | paste -sd, - || true)"
  if ! grep -qE ' [TW] infiltrator_filesystem_api_v2$' <<< "$exports"; then
    printf '%s\t%s\t%s\tMISSING_COMMON_V2\t-\t-\t-\t-\t-\t%s\n' \
      "$repo" "$tag" "$(basename "$so")" "$legacy" >> "$report"
    continue
  fi
  if [[ -n "$unexpected" ]]; then
    echo "::error title=$fmt extra public filesystem ABI exports::$unexpected" >&2
    printf '%s\t%s\t%s\tUNEXPECTED_PUBLIC_EXPORTS\t-\t-\t-\t-\t-\t%s\n' \
      "$repo" "$tag" "$(basename "$so")" "$legacy" >> "$report"
    continue
  fi
  if details="$("$auditor" "$so" 2>"$dir/inspector-error")"; then
    status=COMMON_V2_VALID
    if (( legacy > 0 )); then status=COMMON_V2_PLUS_LEGACY_V1; fi
    printf '%s\t%s\t%s\t%s\t%s\t%s\n' \
      "$repo" "$tag" "$(basename "$so")" "$status" "$details" "$legacy" >> "$report"
  else
    printf '%s\t%s\t%s\tINVALID_OR_UNLOADABLE_COMMON_V2\t-\t-\t-\t-\t-\t%s\n' \
      "$repo" "$tag" "$(basename "$so")" "$legacy" >> "$report"
    printf '::warning title=%s V2 inspection failed::%s\n' "$repo" "$(head -1 "$dir/inspector-error")"
  fi
done
printf '\nIndependent filesystem release audit (23 exact repositories):\n'
column -s $'\t' -t < "$report" || cat "$report"
# The migration now targets all 23 independent providers, not only the
# nine modules from the initial 13 November audit. A missing package, invalid
# V2 descriptor, retired symbol or public non-V2 entry fails the whole gate.
# SWAP remains intentionally nonbootable, but must still expose V2-only.
if [[ "$(wc -l < "$report")" -ne 24 ]]; then
  echo "::error::Independent filesystem audit did not cover all 23 providers" >&2
  exit 1
fi
regression=0
for clean in "${formats[@]}"; do
  repo="Infiltrator-Projects/Filesystem-$clean"
  status="$(awk -F '\t' -v repo="$repo" '$1 == repo {print $4}' "$report")"
  if [[ "$status" != COMMON_V2_VALID ]]; then
    echo "::error title=$clean Common V2 public ABI rejected::$status" >&2
    regression=1
  fi
done
echo "Strict all-23 Common V2 public ABI gate: any non-V2 module is a failure."
echo "Common V2 export validity is distinct from BOOT/OVMF and full DEFRAG qualification."
exit "$regression"
