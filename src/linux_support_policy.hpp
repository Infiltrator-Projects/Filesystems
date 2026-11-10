// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "catalog.hpp"

#include <string_view>
#include <vector>

namespace filesystems {

struct LinuxSupportPolicy {
    std::string_view filesystem_id;
    std::vector<std::string_view> modules;
    std::vector<std::string_view> packages;
    AccessMode access;
    SupportProvider provider;
};

inline const std::vector<LinuxSupportPolicy>& linux_support_policies()
{
    static const std::vector<LinuxSupportPolicy> policies = {
        {"infiltratorfs", {"infiltratorfs"}, {"infiltratorfs"}, AccessMode::ReadWrite, SupportProvider::Dkms},
        {"ext2", {}, {}, AccessMode::ReadWrite, SupportProvider::Independent},
        {"ext3", {}, {}, AccessMode::ReadWrite, SupportProvider::Independent},
        {"ext4", {}, {}, AccessMode::ReadWrite, SupportProvider::Independent},
        {"xfs", {}, {}, AccessMode::ReadWrite, SupportProvider::Independent},
        {"btrfs", {}, {}, AccessMode::ReadWrite, SupportProvider::Independent},
        {"f2fs", {"f2fs"}, {"f2fs-tools"}, AccessMode::ReadWrite, SupportProvider::KernelWithUserspace},
        {"jfs", {"jfs"}, {"jfsutils"}, AccessMode::ReadWrite, SupportProvider::KernelWithUserspace},
        {"nilfs2", {"nilfs2"}, {"nilfs-tools"}, AccessMode::ReadWrite, SupportProvider::KernelWithUserspace},
        {"reiserfs", {"reiserfs"}, {"reiserfsprogs"}, AccessMode::Mixed, SupportProvider::KernelWithUserspace},
        {"minix", {}, {}, AccessMode::ReadWrite, SupportProvider::Independent},
        {"bcachefs", {"bcachefs"}, {}, AccessMode::Mixed, SupportProvider::Kernel},
        {"gfs2", {"gfs2"}, {"gfs2-utils"}, AccessMode::ReadWrite, SupportProvider::KernelWithUserspace},
        {"ocfs2", {"ocfs2"}, {"ocfs2-tools"}, AccessMode::ReadWrite, SupportProvider::KernelWithUserspace},
        {"zfs", {}, {}, AccessMode::ReadWrite, SupportProvider::Independent},
        {"adfs", {"adfs"}, {}, AccessMode::Mixed, SupportProvider::Kernel},
        {"ofs", {}, {}, AccessMode::Mixed, SupportProvider::Independent},
        {"ffs", {}, {}, AccessMode::Mixed, SupportProvider::Independent},
        {"sfs", {}, {}, AccessMode::ReadWrite, SupportProvider::Independent},
        {"sfs2", {}, {}, AccessMode::ReadWrite, SupportProvider::Independent},
        {"pfs3", {}, {}, AccessMode::ReadOnly, SupportProvider::Independent},
        {"befs", {"befs"}, {}, AccessMode::ReadOnly, SupportProvider::Kernel},
        {"bfs", {"bfs"}, {}, AccessMode::Mixed, SupportProvider::Kernel},
        {"efs", {"efs"}, {}, AccessMode::ReadOnly, SupportProvider::Kernel},
        {"hpfs", {"hpfs"}, {}, AccessMode::Mixed, SupportProvider::Kernel},
        {"qnx4", {"qnx4"}, {}, AccessMode::ReadOnly, SupportProvider::Kernel},
        {"qnx6", {"qnx6"}, {}, AccessMode::ReadOnly, SupportProvider::Kernel},
        {"sysv", {"sysv"}, {}, AccessMode::Mixed, SupportProvider::Kernel},
        {"ufs", {}, {}, AccessMode::Mixed, SupportProvider::Independent},
        {"omfs", {"omfs"}, {}, AccessMode::Mixed, SupportProvider::Kernel},
        {"fat16", {}, {}, AccessMode::Experimental, SupportProvider::Independent},
        {"exfat", {}, {}, AccessMode::ReadWrite, SupportProvider::Independent},
        {"ntfs", {}, {}, AccessMode::ReadWrite, SupportProvider::Independent},
        {"hfs", {}, {}, AccessMode::Mixed, SupportProvider::Independent},
        {"hfsplus", {}, {}, AccessMode::Mixed, SupportProvider::Independent},
        {"apfs", {}, {}, AccessMode::Experimental, SupportProvider::Independent},
        {"udf", {}, {}, AccessMode::ReadWrite, SupportProvider::Kernel},
        {"vmfs", {}, {"vmfs-tools"}, AccessMode::ReadOnly, SupportProvider::Userspace},
        {"vmfs6", {}, {"vmfs6-tools"}, AccessMode::ReadOnly, SupportProvider::Userspace},
        {"cpm", {}, {"cpmtools"}, AccessMode::ToolsOnly, SupportProvider::ToolsOnly},
        {"fosfat", {}, {"fosfat"}, AccessMode::ReadOnly, SupportProvider::Userspace}
    };
    return policies;
}

} // namespace filesystems
