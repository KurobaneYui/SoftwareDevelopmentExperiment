#include <InodeRecorder.h>

InodeRecorderBackup *InodeRecorder::inodeRecorderBackup = new InodeRecorderBackup();
InodeRecorderRestore *InodeRecorder::inodeRecorderRestore = new InodeRecorderRestore();

InodeRecorderBackup::InodeRecorderBackup()
{
    Status = NORMAL;
}

InodeRecorderBackup::~InodeRecorderBackup() = default;

StatusCode InodeRecorderBackup::AddInode(const ino_t inode, const off_t offset)
{
    // 判断状态
    if (Status == ERROR)
        return ERROR_UNKNOW;

    // 判断inode是否存在
    if (inodeDataMap.find(inode) != inodeDataMap.end())
        return ERROR_ALREADY_EXISTS;

    // 增加inode和offset
    inodeDataMap[inode] = offset;
    return NO_ERROR;
}

StatusCode InodeRecorderBackup::GetInodeOffset(const ino_t inode, off_t &offset)
{
    // 判断状态
    if (Status == ERROR)
        return ERROR_UNKNOW;

    // 判断inode是否存在
    if (inodeDataMap.find(inode) == inodeDataMap.end())
        return ERROR_NOT_EXISTS;

    // 获取inode和offset
    offset = inodeDataMap[inode];
    return NO_ERROR;
}

bool InodeRecorderBackup::IsInodeExists(const ino_t inode)
{
    // 判断状态
    if (Status == ERROR)
        return ERROR_UNKNOW;

    // 判断inode是否存在
    if (inodeDataMap.find(inode) == inodeDataMap.end())
        return false;

    return true;
}

InodeRecorderRestore::InodeRecorderRestore()
{
    Status = NORMAL;
}

InodeRecorderRestore::~InodeRecorderRestore() = default;

StatusCode InodeRecorderRestore::AddInode(const ino_t inode, const std::string &path)
{
    // 判断状态
    if (Status == ERROR)
        return ERROR_UNKNOW;

    // 判断inode是否存在
    if (inodePathMap.find(inode) != inodePathMap.end())
        return ERROR_ALREADY_EXISTS;

    // 增加inode和path
    inodePathMap[inode] = path;
    return NO_ERROR;
}

StatusCode InodeRecorderRestore::GetInodePath(const ino_t inode, std::string &path)
{
    // 判断状态
    if (Status == ERROR)
        return ERROR_UNKNOW;

    // 判断inode是否存在
    if (inodePathMap.find(inode) == inodePathMap.end())
        return ERROR_NOT_EXISTS;

    // 获取inode和path
    path = inodePathMap[inode];
    return NO_ERROR;
}

bool InodeRecorderRestore::IsInodeExists(const ino_t inode)
{
    // 判断状态
    if (Status == ERROR)
        return ERROR_UNKNOW;

    // 判断inode是否存在
    if (inodePathMap.find(inode) == inodePathMap.end())
        return false;

    return true;
}