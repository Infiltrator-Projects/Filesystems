#!/usr/bin/env python3
"""Source contract for the EFI adapter's read-failure classification.

Build/OVMF validation is separate; static assertions guard against silently
equating disk transport errors with corrupted AmigaDOS metadata.
"""
from pathlib import Path

source = Path("native/filesystems/ofs/efi/uefi_driver.c").read_text()

read_sector = source[source.index("static int read_sector("):
                     source.index("/* Probe at most the boot block")]
assert "result = ctx->disk->ReadDisk(" in read_sector
assert "if (EFI_ERROR(result))" in read_sector
assert "ctx->last_read_error = result == EFI_VOLUME_CORRUPTED" in read_sector
assert "? EFI_DEVICE_ERROR : result;" in read_sector
assert "ctx->last_read_error = EFI_NO_MEDIA;" in read_sector
assert "ctx->last_read_error = EFI_VOLUME_CORRUPTED;" in read_sector
assert "return EFI_ERROR(result) ? -1 : 0;" not in read_sector

start = source.index("static EFI_STATUS reader_error(")
end = source.index("static int read_sector(", start)
error_mapping = source[start:end]
assert "EFI_ERROR(ctx->last_read_error)" in error_mapping
assert "? ctx->last_read_error : EFI_VOLUME_CORRUPTED" in error_mapping

opens = source[source.index("static EFI_STATUS EFIAPI ofs_open(",
                           source.index("/* Operations below expose")):
               source.index("static EFI_STATUS EFIAPI ofs_close(",
                            source.index("/* Operations below expose"))]
# Skip forward declarations to the actual implementation.
opens = source[source.index("static EFI_STATUS EFIAPI ofs_open(EFI_FILE_PROTOCOL *file,", start):]
assert "parent->owner->last_read_error = EFI_SUCCESS;" in opens
assert "reader_error(parent->owner)" in opens

reads = source[source.index("static EFI_STATUS EFIAPI ofs_read(EFI_FILE_PROTOCOL *file, UINTN *length, VOID *buffer)\n{"):]
reads = reads[:reads.index("static EFI_STATUS EFIAPI ofs_write(")]
assert reads.count("node->owner->last_read_error = EFI_SUCCESS;") == 2
assert reads.count("return reader_error(node->owner);") == 2
assert "return EFI_VOLUME_CORRUPTED;" not in reads
print("OFS EFI I/O-status attribution contract passed")
