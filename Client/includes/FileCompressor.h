#ifndef __FILECOMPRESSOR_H__
#define __FILECOMPRESSOR_H__

#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <common.h>
#include <FilePacker.h>

#define MAX_COMPRESSOR_SIZE (1024 * 1024 * 32)   // 压缩字典最大尺寸32M
#define COMPRESSOR_FILE_NAME "COMPRESSOR.BACKUP" // 压缩文件名

// 压缩字典树节点
struct CharNode
{
    unsigned int CurrentIndex;                     // 字典项索引
    std::unordered_map<char, CharNode *> Children; //字典项子项
};

// 释放字典
// node：字典树节点
void FreeCharNode(CharNode *node);

class FileCompressor
{
private:
    CharNode Dictionary;         //字典树根节点
    unsigned int Counter;        //字典项计数器
    std::fstream CompressorFile; //压缩文件
    FileRW *PackerDataFile;      //数据文件
    std::string RootDirectory;   //根目录
    enum
    {
        COMPRESS,   //压缩模式
        DECOMPRESS, //解压模式
        ERROR       //错误
    } Status{};     // 记录状态
public:
    // 构造函数
    // rootDirectory：根目录
    // packerDataFile：数据文件
    FileCompressor(std::string rootDirectory, FileRW *packerDataFile);
    // 构造函数
    // rootDirectory：根目录
    FileCompressor(std::string rootDirectory);
    ~FileCompressor();
    // 压缩
    StatusCode Compress();
    // 解压缩
    StatusCode Decompress();
    // 删除压缩文件
    void DeleteFile();
};

#endif