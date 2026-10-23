#!/usr/bin/env python3
"""Source contract for the EFI adapter's read-failure classification.

Build/OVMF validation is separate; static assertions guard against silently
equating disk transport errors with corrupted AmigaDOS metadata.
"""
from pathlib import Path

source = Path("native/filesystems/ofs/efi/uefi_driver.c").read_text()
link = Path("native/filesystems/ofs/efi/rtsx525a_link.c").read_text()
build_script = Path("scripts/build-ofs-efi.sh").read_text()
assert "ifs_rtsx525a_disable_aspm();" in source
assert "rtsx525a_link" in build_script
assert "DELL_RTS525A_PCI_ID    0x525a10ecU" in link
assert "DELL_RTS525A_SUBSYS_ID 0x0cb91028U" in link
assert "DELL_INTEL_ROOT_ID     0x7e398086U" in link
assert "endpoint_count != 1U" in link
assert "bridge_count != 1U" in link
assert "seg != endpoint_segment" in link
assert "(UINTN)secondary != endpoint_bus" in link
assert "PCIE_EXP_ASPM" not in link
assert "PCI_EXP_ASPM_MASK      0x0003U" in link
assert "disable_aspm(endpoint, endpoint_offset, endpoint_ctl)" in link
assert "disable_aspm(bridge, bridge_offset, bridge_ctl)" in link
assert "(void)pci_put16(endpoint, endpoint_offset, endpoint_ctl);" in link
assert "BS->FreePool(handles);" in link

read_one = source[source.index("static EFI_STATUS read_one_block("):
                  source.index("static int read_sector(")]
read_sector = source[source.index("static int read_sector("):
                     source.index("/* Probe at most the boot block")]
assert "ctx->block->ReadBlocks(" in read_one
assert "ctx->block->Media->MediaId" in read_one
assert "RT->GetTime(&before, NULL)" in read_one
assert "RT->GetTime(&after, NULL)" in read_one
assert "OFS_SLOW_READ_REPORT_LIMIT 12U" in source
assert "ctx->slow_read_reports < OFS_SLOW_READ_REPORT_LIMIT" in read_one
assert "elapsed >= 2U && elapsed <= 3600U" in read_one
assert "OFS EFI: sector %u, attempt %u: %u sec, %r" in read_one
assert "for (attempt = 0U; attempt < 2U; ++attempt)" in read_sector
assert "read_one_block(ctx, number, attempt)" in read_sector
assert "result != EFI_DEVICE_ERROR" in read_sector
assert "!ctx->block->Media->MediaPresent || attempt != 0U" in read_sector
assert "BS->Stall(1000U);" in read_sector
assert "ctx->sector_buffer" in source
assert "OFS_MAX_IO_ALIGNMENT" in source
assert "OFS_DMA_PAGE_ALIGNMENT 4096U" in source
assert "#define OFS_DMA_MAX_ADDRESS 0xffffffffULL" in source
assert "EFI_PHYSICAL_ADDRESS address = OFS_DMA_MAX_ADDRESS;" in source
assert "BS->AllocatePages(AllocateMaxAddress, EfiBootServicesData" in source
assert "BS->AllocatePages(AllocateAnyPages, EfiBootServicesData" not in source
assert "(UINT64)aligned + 511ULL > OFS_DMA_MAX_ADDRESS" in source
assert "BS->FreePages(address, pages);" in source
assert "BS->FreePages(ctx->sector_allocation, ctx->sector_allocation_pages)" in source
assert "ctx->sector_allocation_pages = pages;" in source
assert "if (alignment < OFS_DMA_PAGE_ALIGNMENT)" in source
assert "alignment & (alignment - 1U)" in source
assert "ctx->sector_buffer = (VOID *)" in source
assert "release_sector_buffer(&probe);" in source
assert source.count("release_sector_buffer(ctx);") >= 4
assert "EFI_OPEN_PROTOCOL_BY_DRIVER" in source
assert "ctx->last_failed_sector = number;" in read_sector
assert "ctx->last_block_status = result;" in read_sector
assert "ctx->last_read_error = result == EFI_VOLUME_CORRUPTED" in read_sector
assert "ctx->last_read_error = EFI_NO_MEDIA;" in read_sector
assert "ctx->last_read_error = EFI_VOLUME_CORRUPTED;" in read_sector
assert "prefetch" not in source
assert "cached_sector" not in source
assert "ReadDisk(" not in read_sector
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
assert "return EFI_VOLUME_CORRUPTED;" not in reads
print("OFS EFI I/O-status attribution contract passed")
