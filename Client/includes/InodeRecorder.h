#ifndef __INODE_RECORDER__
#define __INODE_RECORDER__

#include <sys/stat.h>
#include <map>
#include <string>
#include <common.h>

class InodeRecorderBackup
{
private:
    std::map<ino_t, off_t> inodeDataMap{}; // 记录文件的inode和offset
    enum
    {
        NORMAL,
        ERROR
    } Status{}; // 记录状态

public:
    InodeRecorderBackup();
    ~InodeRecorderBackup();
    StatusCode AddInode(const ino_t inode, const off_t offset);  // 增加inode和offset
    StatusCode GetInodeOffset(const ino_t inode, off_t &offset); // 获取inode和offset
    bool IsInodeExists(const ino_t inode);                       // 判断inode是否存在
};

class InodeRecorderRestore
{
private:
    std::map<ino_t, std::string> inodePathMap{}; // 记录文件的inode和path
    enum
    {
        NORMAL,
        ERROR
    } Status{}; // 记录状态

public:
    InodeRecorderRestore();
    ~InodeRecorderRestore();
    StatusCode AddInode(const ino_t inode, const std::string &path); // 增加inode和path
    StatusCode GetInodePath(const ino_t inode, std::string &path);   // 获取inode和path
    bool IsInodeExists(const ino_t inode);                           // 判断inode是否存在
};

class InodeRecorder
{
public:
    static InodeRecorderBackup *inodeRecorderBackup;
    static InodeRecorderRestore *inodeRecorderRestore;
};

#endif