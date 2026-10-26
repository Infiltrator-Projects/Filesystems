/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Freestanding FAT16 namespace reader (ABI v2).
 * Consolidation port of Defragmenter's FAT directory catalogue/VFAT decoder:
 * defragger/gui/filesystems/fat/native/fat_directory.c
 * Disk geometry/cluster model follows fat_volume.c.
 *
 * No POSIX, libc, allocator, writes or implicit Linux kernel dependency.
 * Existing v1 format API is retained unchanged.
 */
#include "fat16_module.h"

#define MAX_LFN_SLOTS 20u
#define LFN_UNITS (MAX_LFN_SLOTS * 13u)

typedef struct {
    uint16_t chars[LFN_UNITS];
    uint8_t checksum;
    uint8_t remaining;
    uint8_t total;
    uint8_t valid;
} LfnState;

static uint16_t load16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}
static void reset_lfn(LfnState *s)
{
    uint32_t i;
    s->checksum = 0;
    s->remaining = s->total = s->valid = 0;
    for (i = 0; i < LFN_UNITS; ++i) s->chars[i] = 0xffffu;
}
static uint8_t short_checksum(const uint8_t *dir)
{
    uint8_t sum = 0;
    uint32_t i;
    for (i = 0; i < 11u; ++i)
        sum = (uint8_t)(((sum & 1u) ? 0x80u : 0u) + (sum >> 1) + dir[i]);
    return sum;
}
/* Port of Defragmenter's ordinal/checksum chain, tightened to reject gaps. */
static void take_lfn(LfnState *s, const uint8_t *d)
{
    static const uint8_t offset[13] =
        {1,3,5,7,9,14,16,18,20,22,24,28,30};
    const uint32_t slot = d[0] & 0x1fu;
    uint32_t i;
    if (d[0] & 0x40u) {
        reset_lfn(s);
        if (slot == 0 || slot > MAX_LFN_SLOTS || d[12] != 0 ||
            load16(d + 26) != 0) return;
        s->valid = 1;
        s->total = (uint8_t)slot;
        s->remaining = (uint8_t)slot;
        s->checksum = d[13];
    }
    if (!s->valid || (d[0] & 0x80u) || slot != s->remaining ||
        d[13] != s->checksum || d[12] != 0 || load16(d + 26) != 0) {
        reset_lfn(s);
        return;
    }
    for (i = 0; i < 13u; ++i)
        s->chars[(slot - 1u) * 13u + i] = load16(d + offset[i]);
    --s->remaining;
}
static int emit_utf8(char *dst, uint32_t *pos, uint32_t cp)
{
    if (!cp || cp == '/' || cp == '\\') cp = '_';
    if (cp > 0x10ffffu || (cp >= 0xd800u && cp <= 0xdfffu))
        cp = 0xfffdu;
    if (*pos + 4u >= IFS_FAT16_NAME_BYTES) return -1;
    if (cp < 0x80u) dst[(*pos)++] = (char)cp;
    else if (cp < 0x800u) {
        dst[(*pos)++] = (char)(0xc0u | (cp >> 6));
        dst[(*pos)++] = (char)(0x80u | (cp & 63u));
    } else if (cp < 0x10000u) {
        dst[(*pos)++] = (char)(0xe0u | (cp >> 12));
        dst[(*pos)++] = (char)(0x80u | ((cp >> 6) & 63u));
        dst[(*pos)++] = (char)(0x80u | (cp & 63u));
    } else {
        dst[(*pos)++] = (char)(0xf0u | (cp >> 18));
        dst[(*pos)++] = (char)(0x80u | ((cp >> 12) & 63u));
        dst[(*pos)++] = (char)(0x80u | ((cp >> 6) & 63u));
        dst[(*pos)++] = (char)(0x80u | (cp & 63u));
    }
    dst[*pos] = '\0';
    return 0;
}
static int make_lfn(const LfnState *s, const uint8_t *short_d, char *name)
{
    uint32_t i, pos = 0;
    int terminated = 0;
    if (!s->valid || s->remaining || s->total == 0 ||
        s->checksum != short_checksum(short_d)) return 0;
    name[0] = '\0';
    for (i = 0; i < (uint32_t)s->total * 13u; ++i) {
        uint16_t ch = s->chars[i];
        uint32_t cp = ch;
        if (ch == 0 || ch == 0xffffu) {
            terminated = 1;
            continue;
        }
        if (terminated) return 0; /* non-padding after terminator */
        if (ch >= 0xd800u && ch <= 0xdbffu) {
            uint16_t low;
            if (++i >= (uint32_t)s->total * 13u) return 0;
            low = s->chars[i];
            if (low < 0xdc00u || low > 0xdfffu) return 0;
            cp = 0x10000u + (((uint32_t)ch - 0xd800u) << 10) +
                 ((uint32_t)low - 0xdc00u);
        } else if (ch >= 0xdc00u && ch <= 0xdfffu) return 0;
        if (emit_utf8(name, &pos, cp) != 0) return 0;
    }
    return pos != 0;
}
static int make_short(const uint8_t *d, char *name)
{
    uint32_t end = 8, ext = 3, pos = 0, i;
    while (end && d[end - 1] == ' ') --end;
    while (ext && d[8 + ext - 1] == ' ') --ext;
    if (!end) return -1;
    for (i = 0; i < end; ++i) {
        uint8_t ch = d[i];
        if (ch < 0x20u || ch > 0x7eu) return -1; /* OEM codec pending */
        name[pos++] = (char)ch;
    }
    if (ext) {
        name[pos++] = '.';
        for (i = 0; i < ext; ++i) {
            uint8_t ch = d[8+i];
            if (ch < 0x20u || ch > 0x7eu) return -1;
            name[pos++] = (char)ch;
        }
    }
    name[pos] = '\0';
    return 0;
}
static int bounded_read(const IfsFat16Io *io, const IfsFat16Volume *v,
                        uint64_t offset, uint8_t *buf, uint32_t bytes)
{
    if (!io || !io->read_at || !buf || !v ||
        v->volume_bytes > io->device_bytes ||
        offset > v->volume_bytes || bytes > v->volume_bytes - offset)
        return -1;
    return io->read_at(io->context, offset, buf, bytes) == 0 ? 0 : -1;
}
static int next_cluster(const IfsFat16Io *io, const IfsFat16Volume *v,
                        uint32_t cluster, uint32_t *next)
{
    uint8_t value[2];
    uint64_t offset;
    uint16_t c;
    if (!next || cluster < 2u || cluster > v->cluster_count + 1u) return -1;
    offset = v->fat_offset + (uint64_t)cluster * 2u;
    if (offset + 2u > v->fat_offset +
        (uint64_t)v->sectors_per_fat * v->bytes_per_sector ||
        bounded_read(io, v, offset, value, sizeof value) != 0) return -1;
    c = load16(value);
    if (c >= 0xfff8u) { *next = 0; return 0; }
    if (c < 2u || c >= 0xfff0u || c > v->cluster_count + 1u) return -1;
    *next = c;
    return 0;
}
static int list_directory(const IfsFat16Io *io, const IfsFat16Volume *v,
                          uint32_t directory_cluster, uint32_t nth,
                          IfsFat16Dirent *output)
{
    LfnState lfn;
    uint32_t cluster = directory_cluster, tortoise = cluster, hare = cluster;
    uint32_t listed = 0, traversed = 0, slot = 0;
    const uint32_t per_cluster = v ? v->cluster_bytes / 32u : 0u;
    if (!v || !output || !io || !per_cluster ||
        (v->cluster_bytes % 32u) != 0 ||
        (cluster && (cluster < 2u || cluster > v->cluster_count + 1u)))
        return -1;
    reset_lfn(&lfn);
    for (;;) {
        uint8_t d[32];
        uint64_t offset;
        uint32_t capacity = cluster ? per_cluster : v->root_entries;
        if (slot == capacity) {
            uint32_t a, b;
            if (!cluster) return 1;
            if (++traversed > v->cluster_count ||
                next_cluster(io, v, cluster, &cluster) != 0) return -1;
            if (!cluster) return 1;
            /* Floyd cycle detection for damaged FAT chains. */
            if (next_cluster(io, v, tortoise, &a) != 0 ||
                next_cluster(io, v, hare, &b) != 0) return -1;
            if (b && next_cluster(io, v, b, &b) != 0) return -1;
            tortoise = a;
            hare = b;
            if (hare && hare == tortoise) return -1;
            slot = 0;
        }
        offset = cluster
            ? v->data_offset + (uint64_t)(cluster - 2u) * v->cluster_bytes +
              (uint64_t)slot * 32u
            : v->root_offset + (uint64_t)slot * 32u;
        ++slot;
        if (bounded_read(io, v, offset, d, sizeof d) != 0) return -1;
        if (d[0] == 0) return 1;
        if (d[0] == 0xe5u) { reset_lfn(&lfn); continue; }
        if (d[11] == 0x0fu) { take_lfn(&lfn, d); continue; }
        if (d[0] == '.' || (d[11] & 0x08u)) {
            reset_lfn(&lfn);
            continue;
        }
        if (listed++ == nth) {
            IfsFat16Dirent result = {{0}, {0}, 0, 0, 0};
            if (make_short(d, result.short_name) != 0) return -1;
            if (!make_lfn(&lfn, d, result.name)) {
                uint32_t j = 0;
                while ((result.name[j] = result.short_name[j]) != '\0') ++j;
            }
            result.first_cluster = load16(d + 26);
            result.size_bytes = (uint32_t)load16(d + 28) |
                                ((uint32_t)load16(d + 30) << 16);
            result.attributes = d[11];
            if ((result.attributes & 0x10u) &&
                (result.first_cluster < 2u ||
                 result.first_cluster > v->cluster_count + 1u)) return -1;
            *output = result;
            return 0;
        }
        reset_lfn(&lfn);
    }
}
static int read_entry(const IfsFat16Io *io, const IfsFat16Volume *v,
                      const IfsFat16Dirent *entry, uint64_t offset,
                      void *buf, uint32_t length)
{
    const IfsFat16Module *legacy = infiltrator_filesystem_module_v1();
    IfsFat16File file = {{0}, 0, 0, 0};
    if (!entry || (entry->attributes & 0x10u) || !legacy) return -1;
    file.first_cluster = entry->first_cluster;
    file.size_bytes = entry->size_bytes;
    file.attributes = entry->attributes;
    return legacy->read_file(io, v, &file, offset, buf, length);
}
static const IfsFat16ModuleV2 module_v2 = {
    IFS_FAT16_ABI_V2, sizeof(IfsFat16ModuleV2), "fat16",
    IFS_FAT16_CAP_DIRECTORY | IFS_FAT16_CAP_VFAT_LFN,
    list_directory, read_entry
};
__attribute__((visibility("default")))
const IfsFat16ModuleV2 *infiltrator_filesystem_module_v2(void)
{
    return &module_v2;
}
