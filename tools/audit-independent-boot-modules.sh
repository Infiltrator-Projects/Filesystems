#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Read-only BOOT acceptance classification of 23 actual published filesystem
# shared libraries. No source VM, ESP, EFI state or physical disk is touched.
set -euo pipefail
: "${GH_TOKEN:?GitHub release read permission required}"
validator="${1:?Pass unmodified production BOOT validate-module}"
report="${2:-filesystem-boot-validator.tsv}"
test -x "$validator"
temp="$(mktemp -d)"
trap 'rm -rf -- "$temp"' EXIT
formats=(FAT12 FAT16 FAT32 EXT4 OFS EXT2 Minix exFAT FFS NTFS
         APFS Btrfs EXT3 HFSPlus HFS PFS3 SFS SFS2 UFS1 UFS2 XFS ZFS Swap)
printf 'repository\trelease\tmodule\tboot_validator\tv2_exports\tlegacy_exports\thas_embedded_elf\tmodule_sha256\tclassification\n' > "$report"
errors=0
for fmt in "${formats[@]}"; do
    repo="Infiltrator-Projects/Filesystem-$fmt"
    directory="$temp/$fmt"
    mkdir -p "$directory/assets" "$directory/extracted"
    case "$fmt" in
      EXT2|Minix) expectation=release_must_pass ;;
      Swap) expectation=nonbootable_must_fail ;;
      FAT12|FAT16|FAT32|EXT4|OFS) expectation=user_reports_working ;;
      *) expectation=not_yet_qualified ;;
    esac
    tag="$(gh api "repos/$repo/releases/latest" --jq '.tag_name' 2>/dev/null || true)"
    module=- result=NO_RELEASE digest=- v2=0 v1=0 embedded=no
    if [[ -n "$tag" ]]; then
      if gh release download "$tag" -R "$repo" --pattern '*.deb' --dir "$directory/assets" >/dev/null 2>&1; then
        shopt -s nullglob
        files=("$directory/assets"/*.deb)
        shopt -u nullglob
        if (( ${#files[@]} != 1 )); then
          result=AMBIGUOUS_PACKAGE
        elif ! dpkg-deb -x "${files[0]}" "$directory/extracted"; then
          result=BAD_PACKAGE
        else
          mapfile -d '' sos < <(find "$directory/extracted" -type f -path '*/Filesystems/*.so' -print0)
          if (( ${#sos[@]} != 1 )); then
            result=WRONG_MODULE_COUNT
          else
            so="${sos[0]}"
            module="$(basename "$so")"
            digest="$(sha256sum "$so" | awk '{print $1}')"
            exports="$(nm -D --defined-only "$so" 2>/dev/null || true)"
            v2="$(grep -Ec ' [TW] infiltrator_filesystem_api_v2$' <<< "$exports" || true)"
            v1="$(grep -Ec 'infiltrator_filesystem_.*_v1|infiltrator_filesystem_module_v1' <<< "$exports" || true)"
            if readelf -SW "$so" | grep -Fq '.infiltrator_boot'; then embedded=yes; fi
            if "$validator" "$so" >/dev/null 2>&1; then
              result=BOOT_ACCEPTED
            else
              result=BOOT_REJECTED
            fi
          fi
        fi
      else
        result=NO_DEB
      fi
    else
      tag=-
    fi
    printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
      "$repo" "$tag" "$module" "$result" "$v2" "$v1" "$embedded" "$digest" "$expectation" >> "$report"
    if [[ "$expectation" == release_must_pass ]] &&
       [[ "$result" != BOOT_ACCEPTED || "$v2" != 1 || "$v1" != 0 ]]; then
      echo "::error title=$fmt BOOT regression::Published module is $result, Common V2=$v2, legacy=$v1" >&2
      errors=1
    fi
    if [[ "$expectation" == nonbootable_must_fail ]] &&
       [[ "$result" != BOOT_REJECTED || "$v2" != 1 || "$v1" != 0 ]]; then
      echo "::error title=SWAP must never be bootable::Published module is $result" >&2
      errors=1
    fi
done
test "$(wc -l < "$report")" -eq 24 || {
 echo 'Expected exactly 23 independent modules and report header' >&2
 exit 1
}
echo 'Unmodified BOOT validator — actual published binary evidence:'
column -s $'\t' -t < "$report" || cat "$report"
echo 'Validator success is only structural validation, not UEFI file-read proof.'
echo 'Independent per-repository OVMF read/corruption qualification remains required.'
exit "$errors"
