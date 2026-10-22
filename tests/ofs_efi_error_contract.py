#!/usr/bin/env python3
"""Source contract for the EFI adapter's read-failure classification.

Build/OVMF validation is separate; static assertions guard against silently
equating disk transport errors with corrupted AmigaDOS metadata.
"""
from pathlib import Path

source = Path("native/filesystems/ofs/efi/uefi_driver.c").read_text()

read_sector = source[source.index("static int read_sector("):
                     source.index("/* Probe at most the boot block")]
assert "result = ctx->block->ReadBlocks(" in read_sector
assert "ctx->block->Media->MediaId" in read_sector
assert "for (attempt = 0U; attempt < 2U; ++attempt)" in read_sector
assert "result != EFI_DEVICE_ERROR" in read_sector
assert "!ctx->block->Media->MediaPresent || attempt != 0U" in read_sector
assert "BS->Stall(1000U);" in read_sector
assert read_sector.count("ctx->block->ReadBlocks(") == 2
assert "#define OFS_READAHEAD_SECTORS 8U" in source
assert "ctx->prefetch_in_file_read && !ctx->prefetch_rejected" in read_sector
assert "ctx->cached_media_id == ctx->block->Media->MediaId" in read_sector
assert "ctx->cached_sector_count = 0U;" in read_sector
assert "sectors * 512U, ctx->sector_buffer" in read_sector
assert "ctx->prefetch_rejected = TRUE;" in read_sector
assert "result != EFI_BAD_BUFFER_SIZE" in read_sector
assert "result != EFI_INVALID_PARAMETER" in read_sector
assert "result != EFI_UNSUPPORTED" in read_sector
assert "ctx->sector_buffer" in read_sector
assert "result = ctx->disk->ReadDisk(" not in read_sector
assert "ctx->last_failed_sector = number;" in read_sector
assert "ctx->last_block_status = result;" in read_sector
assert "OFS_MAX_IO_ALIGNMENT" in source
assert "alignment & (alignment - 1U)" in source
assert "ctx->sector_buffer = (VOID *)" in source
assert "release_sector_buffer(&probe);" in source
assert source.count("release_sector_buffer(ctx);") >= 4
assert "EFI_OPEN_PROTOCOL_BY_DRIVER" in source
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
assert reads.count("return reader_error(node->owner);") == 1
assert "status = reader_error(node->owner);" in reads
assert "return status;" in reads
assert "EFI_ERROR(node->owner->last_block_status)" in reads
assert "node->owner->last_failed_sector" in reads
assert "node->owner->last_block_status = EFI_SUCCESS;" in reads
assert "node->owner->prefetch_in_file_read = TRUE;" in reads
assert "node->owner->prefetch_in_file_read = FALSE;" in reads
assert "return EFI_VOLUME_CORRUPTED;" not in reads
print("OFS EFI I/O-status attribution contract passed")
