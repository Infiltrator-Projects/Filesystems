#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Qualify the actual released Debian carriers of the 23 independent filesystem
# modules against the canonical Common V2 ABI. Never inspect VM/ESP/firmware.
set -euo pipefail

: "${GH_TOKEN:?Read-only GitHub release access is required}"
auditor="${1:?Provide the built Common V2 ABI inspector}"
report="${2:-filesystem-common-v2-release-audit.tsv}"
test -x "$auditor"
work="$(mktemp -d)"
trap 'rm -rf -- "$work"' EXIT

formats=(
  APFS Btrfs EXT2 EXT3 EXT4 FAT12 FAT16 FAT32 FFS HFS HFSPlus
  Minix NTFS OFS PFS3 SFS SFS2 Swap UFS1 UFS2 XFS ZFS exFAT
)
printf 'repository\trelease_tag\tso_name\tstatus\tformat_id\truntime_profiles\tcore_caps\tvolume_caps\tdefrag_caps\tlegacy_v1_exports\n' > "$report"

for fmt in "${formats[@]}"; do
    repo="Infiltrator-Projects/Filesystem-$fmt"
    dir="$work/$fmt"
    mkdir -p "$dir/assets" "$dir/unpacked"
    tag="$(gh api "repos/$repo/releases/latest" --jq '.tag_name' 2>/dev/null || true)"
    if [[ -z "$tag" ]]; then
        printf '%s\t-\t-\tNO_RELEASE\t-\t-\t-\t-\t-\t-\n' "$repo" >> "$report"
        continue
    fi

    # Do not confuse a previously green tag with the current module source.
    main_version="$(gh api "repos/$repo/contents/VERSION?ref=main" \
        --jq '.content' 2>/dev/null | base64 -d 2>/dev/null | tr -d '\r\n' || true)"
    if [[ -z "$main_version" || "$tag" != "v$main_version" ]]; then
        printf '%s\t%s\t-\tRELEASE_BEHIND_MAIN_VERSION\t-\t-\t-\t-\t-\t-\n' \
            "$repo" "$tag" >> "$report"
        continue
    fi

    # Every independent module has precisely one generic .deb and one
    # CPU-native source .run release artifact. Reject missing/ambiguous files.
    asset_names="$(gh api "repos/$repo/releases/tags/$tag" \
        --jq '.assets[].name' 2>/dev/null || true)"
    run_count="$(printf '%s\n' "$asset_names" | grep -Ec '\.run$' || true)"
    deb_count="$(printf '%s\n' "$asset_names" | grep -Ec '\.deb$' || true)"
    if [[ "$run_count" != 1 || "$deb_count" != 1 ]]; then
        printf '%s\t%s\t-\tMISSING_OR_DUPLICATE_RUN_DEB\t-\t-\t-\t-\t-\t-\n' \
            "$repo" "$tag" >> "$report"
        continue
    fi

    if ! gh release download "$tag" -R "$repo" \
        --pattern '*.deb' --dir "$dir/assets" >/dev/null 2>&1; then
        printf '%s\t%s\t-\tNO_DEB_RELEASE_ASSET\t-\t-\t-\t-\t-\t-\n' \
            "$repo" "$tag" >> "$report"
        continue
    fi
    shopt -s nullglob
    debs=("$dir/assets"/*.deb)
    shopt -u nullglob
    if (( ${#debs[@]} != 1 )); then
        printf '%s\t%s\t-\tAMBIGUOUS_DEB_ASSETS\t-\t-\t-\t-\t-\t-\n' \
            "$repo" "$tag" >> "$report"
        continue
    fi
    if ! dpkg-deb -x "${debs[0]}" "$dir/unpacked"; then
        printf '%s\t%s\t-\tUNPACK_FAILED\t-\t-\t-\t-\t-\t-\n' \
            "$repo" "$tag" >> "$report"
        continue
    fi

    mapfile -d '' sos < <(find "$dir/unpacked" -type f \
        -path '*/Filesystems/*.so' -print0)
    mapfile -d '' pdfs < <(find "$dir/unpacked" -type f \
        -iname '*.pdf' -print0)
    mapfile -d '' kos < <(find "$dir/unpacked" -type f \
        -name '*.ko' -print0)
    if (( ${#sos[@]} != 1 || ${#pdfs[@]} != 1 || ${#kos[@]} != 0 )); then
        printf '%s\t%s\t-\tINVALID_SO_PDF_OR_KERNEL_COUNT\t-\t-\t-\t-\t-\t-\n' \
            "$repo" "$tag" >> "$report"
        continue
    fi
    so="${sos[0]}"
    if [[ "$(basename "$so")" != "$fmt.so" ]]; then
        # Installed module spelling follows the canonical project identifier.
        # The SFS0 and HFSPLUS aliases are handled by the file's own name.
        case "$fmt:$(basename "$so")" in
            SFS:SFS.so|HFSPlus:HFSPLUS.so|Btrfs:BTRFS.so|Minix:MINIX.so|exFAT:EXFAT.so) ;;
            *) printf '%s\t%s\t%s\tWRONG_SO_FILENAME\t-\t-\t-\t-\t-\t-\n' \
                    "$repo" "$tag" "$(basename "$so")" >> "$report"; continue ;;
        esac
    fi

    exports="$(nm -D --defined-only "$so" 2>/dev/null || true)"
    legacy="$(printf '%s\n' "$exports" |
        grep -Ec 'infiltrator_filesystem_.*_v(1|3|4|5)|infiltrator_filesystem_module_v[135]' || true)"
    unexpected="$(printf '%s\n' "$exports" | awk 'NF >= 3 {print $3}' |
        sed 's/@.*//' | grep -vFx 'infiltrator_filesystem_api_v2' |
        LC_ALL=C sort -u | paste -sd, - || true)"
    if ! grep -qE ' [TW] infiltrator_filesystem_api_v2$' <<< "$exports"; then
        printf '%s\t%s\t%s\tMISSING_COMMON_V2_EXPORT\t-\t-\t-\t-\t-\t%s\n' \
            "$repo" "$tag" "$(basename "$so")" "$legacy" >> "$report"
        continue
    fi
    if [[ -n "$unexpected" || "$legacy" != 0 ]]; then
        printf '%s\t%s\t%s\tNON_V2_PUBLIC_EXPORTS\t-\t-\t-\t-\t-\t%s\n' \
            "$repo" "$tag" "$(basename "$so")" "$legacy" >> "$report"
        continue
    fi

    if details="$("$auditor" "$so" 2>"$dir/auditor-error")"; then
        printf '%s\t%s\t%s\tCOMMON_V2_VALID\t%s\t%s\n' \
            "$repo" "$tag" "$(basename "$so")" "$details" "$legacy" >> "$report"
    else
        printf '%s\t%s\t%s\tINVALID_OR_UNLOADABLE_COMMON_V2\t-\t-\t-\t-\t-\t%s\n' \
            "$repo" "$tag" "$(basename "$so")" "$legacy" >> "$report"
        printf '::warning title=%s V2 auditor refused package::%s\n' \
            "$repo" "$(head -1 "$dir/auditor-error")"
    fi
done

if [[ "$(wc -l < "$report")" != 24 ]]; then
    echo "::error::The audit did not classify all 23 providers" >&2
    exit 1
fi
printf '\nCommon V2 release audit — actual 23 Debian carriers:\n'
column -s $'\t' -t < "$report" || cat "$report"
regression=0
for fmt in "${formats[@]}"; do
    repo="Infiltrator-Projects/Filesystem-$fmt"
    status="$(awk -F '\t' -v repo="$repo" '$1==repo {print $4}' "$report")"
    if [[ "$status" != COMMON_V2_VALID ]]; then
        printf '::error title=%s not V2-qualified::%s\n' "$fmt" "$status" >&2
        regression=1
    fi
done
echo 'A validated Common V2 descriptor is NOT BOOT V2/OVMF handoff proof.'
exit "$regression"
