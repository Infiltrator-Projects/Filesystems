#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# QEMU initramfs only; /dev/vda and /dev/vdb are disposable image files.
set -eu
fail() { echo "FAT-KO-RUNTIME-FAIL: $*" > /dev/ttyS0; poweroff -f; exit 1; }
mount -t proc proc /proc || fail proc
mount -t sysfs sysfs /sys || fail sysfs
mount -t devtmpfs devtmpfs /dev || fail devtmpfs
insmod /fat12.ko || fail 'insmod fat12.ko'
insmod /fat16.ko || fail 'insmod fat16.ko'
grep -q '[[:space:]]fat12$' /proc/filesystems || fail 'fat12 registration'
grep -q '[[:space:]]fat16$' /proc/filesystems || fail 'fat16 registration'
# Refusing the other format must leave every byte unchanged.
for item in 'fat12 /dev/vdb' 'fat16 /dev/vda' 'fat12 /dev/vdc' 'fat16 /dev/vdc'; do
    set -- $item
    before=$(sha256sum "$2")
    if mount -t "$1" "$2" /mnt/test; then
        umount /mnt/test || :
        fail "$1 accepted wrong variant"
    fi
    after=$(sha256sum "$2")
    test "$before" = "$after" || fail "$1 changed rejected medium"
done
for item in 'fat12 /dev/vda' 'fat16 /dev/vdb'; do
    set -- $item
    kind=$1 device=$2
    mount -t "$kind" "$device" /mnt/test || fail "$kind mount"
    grep -q " /mnt/test $kind " /proc/mounts || fail "$kind mount identity"
    mkdir /mnt/test/tree || fail "$kind mkdir"
    echo "$kind kernel driver survives remount" > '/mnt/test/tree/Original Long Name.txt' || fail "$kind write"
    dd if=/dev/zero of=/mnt/test/tree/blocks.bin bs=4096 count=32 || fail "$kind multicluster write"
    mv '/mnt/test/tree/Original Long Name.txt' '/mnt/test/tree/Renamed Long Name.txt' || fail "$kind rename"
    mkdir /mnt/test/remove-me || fail "$kind mkdir remove"
    echo delete > /mnt/test/remove-me/temp.txt || fail "$kind create remove"
    rm /mnt/test/remove-me/temp.txt || fail "$kind delete"
    rmdir /mnt/test/remove-me || fail "$kind rmdir"
    sync
    umount /mnt/test || fail "$kind unmount"
    mount -t "$kind" "$device" /mnt/test || fail "$kind remount"
    test "$(cat '/mnt/test/tree/Renamed Long Name.txt')" = "$kind kernel driver survives remount" || fail "$kind persistent contents"
    test "$(stat -c %s /mnt/test/tree/blocks.bin)" = 131072 || fail "$kind persistent size"
    test ! -e /mnt/test/remove-me || fail "$kind persistent deletion"
    umount /mnt/test || fail "$kind final unmount"
    echo "FAT-KO-RUNTIME-PASS: $kind" > /dev/ttyS0
done
poweroff -f
