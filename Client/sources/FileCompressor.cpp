#include <FileCompressor.h>
#include <iostream>

void FreeCharNode(CharNode *node)
{
    // 递归清理节点
    for (auto it = node->Children.begin(); it != node->Children.end(); ++it)
    {
        FreeCharNode(it->second);
        delete it->second;
    }
    node->Children.clear();
}

FileCompressor::FileCompressor(std::string rootDirectory, FileRW *packerDataFile)
{
    if (packerDataFile == nullptr)
    {
        Status = ERROR;
        return;
    }

    PackerDataFile = packerDataFile;
    RootDirectory = rootDirectory;
    Counter = 0;
    Dictionary.CurrentIndex = 0;
    Status = COMPRESS;
    return;
}

FileCompressor::FileCompressor(std::string rootDirectory)
{
    RootDirectory = rootDirectory;
    Counter = 0;
    Dictionary.CurrentIndex = 0;
    Status = DECOMPRESS;
    return;
}

FileCompressor::~FileCompressor()
{
    CompressorFile.close();
    FreeCharNode(&Dictionary);
}

StatusCode FileCompressor::Compress()
{
    if (Status != COMPRESS)
        return ERROR_UNKNOW;

    // 打开保存压缩后文件的文件
    CompressorFile.open(RootDirectory + COMPRESSOR_FILE_NAME, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!CompressorFile.is_open())
    {
        Status = ERROR;
        return ERROR_FILE_CANT_OPEN;
    }

    // 配置进度条
    off_t ProgressBarTotal{};    // 进图条总长度
    off_t ProgressBarCurrent{0}; // 进图条当前长度
    ProgressBarTotal = PackerDataFile->Length();

    CharNode *pNode = &Dictionary; // 字典树当前节点
    char tmp{};                    // 当前读取字符
    // 开始按照LZ78算法压缩
    PackerDataFile->seekg(0);
    while (PackerDataFile->peek() != EOF)
    {
        // 读取字符并在当前节点下继续查找字符
        // 从根节点到当前节点的路径是目前检索的字符串
        PackerDataFile->read((char *)&tmp, 1);
        std::unordered_map<char, CharNode *>::iterator it = pNode->Children.find(tmp);
        // 如果当前节点没有找到该字符，则添加该字符到当前节点（新增字典项），并重新回到头节点
        if (it == pNode->Children.end())
        {
            CharNode *tmpNode = new CharNode;
            tmpNode->CurrentIndex = ++Counter;
            if (Counter == 0)
            {
                std::cerr << "Out of Counter Range" << std::endl;
            }
            pNode->Children.insert(std::unordered_map<char, CharNode *>::value_type(tmp, tmpNode));

            char *buff = new char[sizeof(unsigned int) + 1];
            *(unsigned int *)buff = pNode->CurrentIndex;
            buff[sizeof(unsigned int)] = tmp;
            CompressorFile.write(buff, sizeof(unsigned int) + 1);
            delete[] buff;

            pNode = &Dictionary;
        }
        // 如果找到该字符串，则继续下一个字符的检索
        else
        {
            pNode = it->second;
        }

        // 更新进度条（每0x000FFFFF字节更新一次）
        char adjust = (ProgressBarCurrent >> 20) & 0x1;
        ProgressBarCurrent += 1;
        if ((ProgressBarCurrent >> 20) & 0x1 == adjust + 1)
        {
            std::cout << "\rCompressing: (" << ProgressBarCurrent << " / " << ProgressBarTotal << ")" << std::flush;
        }
    }

    // 当最后一个字符读取结束后，如果当前节点不是根节点
    // 则表明文件结尾最后一段在字典中，直接将编码加入压缩后文件
    bool endWithNull{}; // 指示压缩文件最后一个编码，是否有后继字符
    if (pNode != &Dictionary)
    {
        char *buff = new char[sizeof(unsigned int) + 1];
        *(unsigned int *)buff = pNode->CurrentIndex;
        buff[sizeof(unsigned int)] = '\0';
        CompressorFile.write(buff, sizeof(unsigned int) + 1);
        delete[] buff;
        endWithNull = true;
    }
    // 否则，表明文件结尾没有多余内容
    else
    {
        endWithNull = false;
    }
    // 写入字典总节点数和后继字符标识
    CompressorFile.write((char *)&Counter, sizeof(unsigned int));
    CompressorFile.write((char *)&endWithNull, sizeof(bool));

    // 更新进度条
    std::cout << "\rCompressing: (" << ProgressBarTotal << " / " << ProgressBarTotal << ")" << std::endl;
    std::cout << "Dictionary Index num: " << Counter << std::endl;
    std::cout << "Compress File Size: " << CompressorFile.tellp() << std::endl;

    FreeCharNode(&Dictionary);
    CompressorFile.close();

    return NO_ERROR;
}

StatusCode FileCompressor::Decompress()
{
    if (Status != DECOMPRESS)
        return ERROR_UNKNOW;

    // 打开压缩文件
    CompressorFile.open(RootDirectory + COMPRESSOR_FILE_NAME, std::ios::in | std::ios::binary);
    // 打开解压结果保存的文件
    std::fstream BackupFile = std::fstream(RootDirectory + PACKED_FILE_NAME, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!CompressorFile.is_open() || !BackupFile.is_open())
    {
        CompressorFile.close();
        BackupFile.close();
        return ERROR_FILE_CANT_OPEN;
    }

    off_t TotalLength{}; // 压缩文件长度（文件末尾的标志信息不计入）
    off_t readLength{0}; // 已读取长度
    bool endWithNull{};  // 后继字符标识

    // 读取压缩文件末尾的标志信息
    CompressorFile.seekg(0, SEEK_BASE_END);
    std::cout << "Compress File Size: " << CompressorFile.tellg() << std::endl;
    CompressorFile.seekg(-sizeof(bool) - sizeof(unsigned int), SEEK_BASE_END);
    CompressorFile.read((char *)&Counter, sizeof(unsigned int));
    CompressorFile.read((char *)&endWithNull, sizeof(bool));
    CompressorFile.seekg(-sizeof(bool) - sizeof(unsigned int), SEEK_BASE_END);
    std::cout << "Index Length: " << Counter << std::endl;
    // 获取压缩文件长度
    TotalLength = CompressorFile.tellg();

    // 如果压缩文件长度不是编码序列的整数倍则表明压缩文件损坏
    if (TotalLength % (sizeof(unsigned int) + 1) != 0)
        return ERROR_FILE_INFO_NOT_MATCH;

    // 截断压缩文件，不保留末尾标志信息
    CompressorFile.close();
#if __cplusplus >= 201703L
    std::filesystem::resize_file(RootDirectory + COMPRESSOR_FILE_NAME, TotalLength);
#else
    truncate((RootDirectory + COMPRESSOR_FILE_NAME).c_str(), TotalLength);
#endif
    CompressorFile.open(RootDirectory + COMPRESSOR_FILE_NAME, std::ios::in | std::ios::binary);

    char *buff = new char[sizeof(unsigned int) + 1];                                        // 一个压缩编码的缓冲区
    std::map<unsigned int, std::string> Index2Node;                                         // 初始化字典
    Index2Node.insert(std::map<unsigned int, std::string>::value_type(0, std::string(""))); // 初始化字典
    Counter = 0;                                                                            //索引计数归零

    // 开始解压
    CompressorFile.seekg(0);
    while (CompressorFile.peek() != EOF)
    {
        char adjust = (readLength >> 20) & 0x1;
        // 读取一个压缩编码
        CompressorFile.read(buff, sizeof(unsigned int) + 1);
        readLength += CompressorFile.gcount();
        unsigned int index = *(unsigned int *)buff;
        char tmp = buff[sizeof(unsigned int)];
        // 如果已经到达压缩文件末尾，则根据后继字符标识判断是否需要添加后继字符
        if (readLength == TotalLength)
        {
            unsigned int index = *(unsigned int *)buff;
            char tmp = buff[sizeof(unsigned int)];
            std::string &CurrentString = Index2Node.find(index)->second;
            if (!endWithNull)
                CurrentString += tmp;
            BackupFile.write(CurrentString.c_str(), CurrentString.size());
        }
        // 如果没有到达压缩文件末尾
        else
        {
            unsigned int index = *(unsigned int *)buff;
            char tmp = buff[sizeof(unsigned int)];
            std::string CurrentString = Index2Node.find(index)->second;
            CurrentString += tmp;
            Index2Node.insert(std::map<unsigned int, std::string>::value_type(++Counter, CurrentString));
            BackupFile.write(CurrentString.c_str(), CurrentString.size());
        }

        // 更新进度条（每0x000FFFFF个字符更新一次）
        if ((readLength >> 20) & 0x1 == adjust + 1)
        {
            std::cout << "\rDecompressing: (" << readLength << " / " << TotalLength << ")" << std::flush;
        }
    }

    // 更新进度条
    std::cout << "\rDecompressing: (" << TotalLength << " / " << TotalLength << ")" << std::endl;
    std::cout << "Dictionary Index num: " << Counter << std::endl;

    delete[] buff;
    Index2Node.clear();
    BackupFile.close();
    CompressorFile.close();

    return NO_ERROR;
}

void FileCompressor::DeleteFile()
{
    remove((RootDirectory + COMPRESSOR_FILE_NAME).c_str());
    return;
}