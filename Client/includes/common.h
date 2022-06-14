#ifndef __COMMON_H__
#define __COMMON_H__

// 函数返回状态码
enum StatusCode : char
{
    NO_ERROR,                  // 没有错误
    END_OF_FILE,               // 文件结束
    ERROR_FILE_CANT_OPEN,      // 文件打开失败
    ERROR_FILE_CANT_READ,      // 文件读取失败
    ERROR_FILE_CANT_WRITE,     // 文件写入失败
    ERROR_NOT_EXISTS,          // 文件不存在
    ERROR_ALREADY_EXISTS,      // 文件已存在
    ERROR_FILE_INFO_NOT_MATCH, // 文件信息不匹配
    ERROR_FILE_NOT_DIRECTORY,  // 文件不是目录
    ERROR_FILE_NOT_REGFILE,    // 文件不是普通文件
    ERROR_FILE_NOT_SYMLINK,    // 文件不是符号链接
    ERROR_FILE_NOT_FIFO,       // 文件不是FIFO
    ERROR_FILE_TYPE_UNKNOWN,   // 文件类型未知
    ERROR_UNKNOW               // 未知错误
};

// 文件类型码
enum FileType : char
{
    FILE_TYPE_FILE,      // 普通文件
    FILE_TYPE_DIRECTORY, // 目录
    FILE_TYPE_SYMLINK,   // 符号链接
    FILE_TYPE_FIFO,      // FIFO
    FILE_TYPE_UNKNOWN    // 未知类型
};

#endif // __COMMON_H__