#ifndef __FILEINFO__
#define __FILEINFO__

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <ctime>
#include <utime.h>
#include <dirent.h>
#include <queue>
#include <string>
#include <cstring>
#include <InodeRecorder.h>
#include <common.h>
#include <FilePacker.h>

#define FILE_UNIT_INFO_HEAD_SIZE 529 // 文件元信息头部大小（不包含std::string FileUnitInfo::RelativePath的空间）

struct FileUnitInfo
{
    off_t TotalLength;        // 元信息总长（记录在备份文件里的总长）
    ino_t Inode;              // 文件inode
    nlink_t InodeCount;       // 文件nlink计数
    FileType Type;            //文件类型
    short int Auth;           //文件权限信息
    timespec Atime;           //文件last access time信息
    uid_t UserID;             //文件uid
    gid_t GroupID;            //文件gid
    off_t Size;               //文件备份大小（记录在备份文件里的大小）
    off_t StartOffset;        //文件偏移（记录在备份文件里的偏移）
    std::string RelativePath; //文件相对路径
};

class FileInfo
{
private:
    FileUnitInfo UnitInfo;                                   // 文件元信息
    std::fstream FileStream;                                 // 文件流
    std::string RootDirectory;                               // 根目录
    FilePacker *Packer;                                      // 文件打包器
    std::priority_queue<ino_t> DirectoryChildrenInodesQueue; // 目录子文件的inode优先队列
    off_t DirectoryUnitOffset;                               // 目录元信息在备份文件中的偏移量
    enum : char
    {
        GOT_INFO, // 已获取信息
        BACKUPED, // 已备份
        VERIFIED, // 已验证
        RESTORED, // 已恢复
        ERROR     // 错误
    } Status;     // 文件状态

    // 在备份任务时，获取文件的元信息
    StatusCode GetUnitDataForBackup();
    // 在还原任务时，获取文件的元信息
    StatusCode GetUnitDataForRestore();
    // 验证备份文件完整性
    StatusCode Verify();

public:
    // 打开一个文件用于备份
    // relativePath: 文件相对路径
    // packer: 文件打包器
    // rootDirectory: 根目录
    FileInfo(const std::string relativePath, FilePacker *packer, const std::string &rootDirectory);
    // 打开一个文件用于还原
    // packer: 文件打包器
    // rootDirectory: 根目录
    FileInfo(FilePacker *packer, const std::string &rootDirectory);
    ~FileInfo();
    // 备份文件
    StatusCode Backup();
    // 还原文件
    StatusCode Restore();
    // 验证状态
    bool good();
};

#endif