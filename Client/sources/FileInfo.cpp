#include "FileInfo.h"

FileInfo::FileInfo(const std::string relativePath, FilePacker *packer, const std::string &rootDirectory)
{
    Packer = packer;
    RootDirectory = rootDirectory;
    if (RootDirectory.back() != '/')
        RootDirectory += '/';
    UnitInfo.RelativePath = relativePath;
    // 从文件本身获取元信息
    StatusCode state = GetUnitDataForBackup();
    if (state != NO_ERROR)
    {
        Status = ERROR;
        return;
    }
    Status = GOT_INFO;
    return;
}

FileInfo::FileInfo(FilePacker *packer, const std::string &rootDirectory)
{
    Packer = packer;
    RootDirectory = rootDirectory;
    if (RootDirectory.back() != '/')
        RootDirectory += '/';
    // 从备份文件获取元信息
    StatusCode state = GetUnitDataForRestore();
    if (state != NO_ERROR)
    {
        Status = ERROR;
        return;
    }
    Status = GOT_INFO;
    return;
}

FileInfo::~FileInfo() = default;

StatusCode FileInfo::GetUnitDataForBackup()
{
    struct stat tmp_stat{};

    if (Status == ERROR)
        return ERROR_UNKNOW;

    // if file or directory not exists
    if (lstat((RootDirectory + UnitInfo.RelativePath).c_str(), &tmp_stat) == -1)
    {
        Status = ERROR;
        return ERROR_NOT_EXISTS;
    }
    // else, file or directory exists
    else
    {
        // 不同类型文件不同的部分
        switch (tmp_stat.st_mode & S_IFMT)
        {
        case S_IFREG:
            UnitInfo.Type = FILE_TYPE_FILE;
            UnitInfo.Size = tmp_stat.st_size;
            break;
        case S_IFLNK:
            UnitInfo.Type = FILE_TYPE_SYMLINK;
            UnitInfo.Size = tmp_stat.st_size;
            break;
        case S_IFIFO:
            UnitInfo.Type = FILE_TYPE_FIFO;
            UnitInfo.Size = 0;
            break;
        case S_IFDIR:
            UnitInfo.Type = FILE_TYPE_DIRECTORY;
            UnitInfo.Size = 0;
            break;
        default:
            Status = ERROR;
            return ERROR_FILE_TYPE_UNKNOWN;
        }
        // 不同类型文件相同的部分
        UnitInfo.Inode = tmp_stat.st_ino;
        UnitInfo.InodeCount = tmp_stat.st_nlink;
        UnitInfo.Auth = tmp_stat.st_mode & 07777;
        UnitInfo.Atime = tmp_stat.st_atim;
        UnitInfo.UserID = tmp_stat.st_uid;
        UnitInfo.GroupID = tmp_stat.st_gid;
        UnitInfo.Size = tmp_stat.st_size;

        // 将inode信息加入维护，可以针对硬链接减少额外保存数据
        // if inode exists in inodeRecorder, get backupBegin
        if (InodeRecorder::inodeRecorderBackup->IsInodeExists(UnitInfo.Inode))
        {
            InodeRecorder::inodeRecorderBackup->GetInodeOffset(UnitInfo.Inode, UnitInfo.StartOffset);
        }
        // else get the backupBegin and insert into inodeRecorder
        else
        {
            UnitInfo.StartOffset = Packer->DataFile->Length();
            InodeRecorder::inodeRecorderBackup->AddInode(UnitInfo.Inode, UnitInfo.StartOffset);
        }

        // 计算元信息结构体总长
        UnitInfo.TotalLength = FILE_UNIT_INFO_HEAD_SIZE + UnitInfo.RelativePath.size();
        Status = GOT_INFO;
        return NO_ERROR;
    }
}

StatusCode FileInfo::GetUnitDataForRestore()
{
    if (Status == ERROR)
        return ERROR_UNKNOW;

    off_t oneUnitSize{}; // 记录一个元信息结构体的长度
    size_t readLength{}; // 已读长度

    // 获取一个元信息
    // 先获取元信息结构体的长度，再读取其中的内容
    Packer->UnitFile->read((char *)&oneUnitSize, sizeof(off_t));
    if (Packer->UnitFile->gcount() != sizeof(off_t))
    {
        Status = ERROR;
        return ERROR_FILE_CANT_READ;
    }
    Packer->UnitFile->seekg(-sizeof(off_t), SEEK_BASE_CUR);
    char *buffer = new char[oneUnitSize];
    Packer->UnitFile->read(buffer, oneUnitSize);
    if (Packer->UnitFile->gcount() != oneUnitSize)
    {
        Status = ERROR;
        delete[] buffer;
        buffer = nullptr;
        return ERROR_FILE_CANT_READ;
    }
    UnitInfo.TotalLength = ((FileUnitInfo *)buffer)->TotalLength;
    UnitInfo.Inode = ((FileUnitInfo *)buffer)->Inode;
    UnitInfo.InodeCount = ((FileUnitInfo *)buffer)->InodeCount;
    UnitInfo.Type = ((FileUnitInfo *)buffer)->Type;
    UnitInfo.Auth = ((FileUnitInfo *)buffer)->Auth;
    UnitInfo.Atime = ((FileUnitInfo *)buffer)->Atime;
    UnitInfo.UserID = ((FileUnitInfo *)buffer)->UserID;
    UnitInfo.GroupID = ((FileUnitInfo *)buffer)->GroupID;
    UnitInfo.Size = ((FileUnitInfo *)buffer)->Size;
    UnitInfo.StartOffset = ((FileUnitInfo *)buffer)->StartOffset;
    UnitInfo.RelativePath = std::string(buffer + FILE_UNIT_INFO_HEAD_SIZE, oneUnitSize - FILE_UNIT_INFO_HEAD_SIZE);
    delete[] buffer;
    buffer = nullptr;

    // 将inode对应的路径加入维护，用于创建硬链接
    // if inode not exists in inodeRecorder, insert relative path into inodeRecorder
    if (!InodeRecorder::inodeRecorderRestore->IsInodeExists(UnitInfo.Inode))
    {
        InodeRecorder::inodeRecorderRestore->AddInode(UnitInfo.Inode, UnitInfo.RelativePath);
    }

    Status = GOT_INFO;
    return NO_ERROR;
}

StatusCode FileInfo::Verify()
{
    size_t readLength{};                                 // 已读长度
    off_t tmpReadUnitOffset = Packer->UnitFile->tellg(); // 记录UnitFile当前读指针位置，验证完后恢复，做到对UnitFile无副作用

    // 如果是文件夹，从DirectoryUnitOffset开始验证，
    // 因为备份文件夹后还备份了其中的文件，无法直接通过反向偏移找到元信息头
    if (UnitInfo.Type == FILE_TYPE_DIRECTORY)
        Packer->UnitFile->seekg(DirectoryUnitOffset);
    else
        Packer->UnitFile->seekg(-UnitInfo.TotalLength, SEEK_BASE_END);

    // 读取备份的元信息
    char *UnitInfoInBytes = (char *)new char[UnitInfo.TotalLength];
    Packer->UnitFile->read((char *)UnitInfoInBytes, UnitInfo.TotalLength);
    if (Packer->UnitFile->gcount() != UnitInfo.TotalLength)
    {
        delete[] UnitInfoInBytes;
        UnitInfoInBytes = nullptr;
        return ERROR_FILE_CANT_READ;
    }
    FileUnitInfo backupedUnitInfo;
    backupedUnitInfo.TotalLength = ((FileUnitInfo *)UnitInfoInBytes)->TotalLength;
    backupedUnitInfo.Inode = ((FileUnitInfo *)UnitInfoInBytes)->Inode;
    backupedUnitInfo.InodeCount = ((FileUnitInfo *)UnitInfoInBytes)->InodeCount;
    backupedUnitInfo.Type = ((FileUnitInfo *)UnitInfoInBytes)->Type;
    backupedUnitInfo.Auth = ((FileUnitInfo *)UnitInfoInBytes)->Auth;
    backupedUnitInfo.Atime = ((FileUnitInfo *)UnitInfoInBytes)->Atime;
    backupedUnitInfo.UserID = ((FileUnitInfo *)UnitInfoInBytes)->UserID;
    backupedUnitInfo.GroupID = ((FileUnitInfo *)UnitInfoInBytes)->GroupID;
    backupedUnitInfo.Size = ((FileUnitInfo *)UnitInfoInBytes)->Size;
    backupedUnitInfo.StartOffset = ((FileUnitInfo *)UnitInfoInBytes)->StartOffset;
    backupedUnitInfo.RelativePath = std::string(UnitInfoInBytes + FILE_UNIT_INFO_HEAD_SIZE, backupedUnitInfo.TotalLength - FILE_UNIT_INFO_HEAD_SIZE);
    delete[] UnitInfoInBytes;
    UnitInfoInBytes = nullptr;
    // 还原UnitFile的读指针
    Packer->UnitFile->seekg(tmpReadUnitOffset);

    // 验证元信息是否备份正确
    if (UnitInfo.Inode == backupedUnitInfo.Inode &&
        UnitInfo.InodeCount == backupedUnitInfo.InodeCount &&
        UnitInfo.Type == backupedUnitInfo.Type &&
        UnitInfo.Auth == backupedUnitInfo.Auth &&
        UnitInfo.Atime.tv_nsec == backupedUnitInfo.Atime.tv_nsec &&
        UnitInfo.Atime.tv_sec == backupedUnitInfo.Atime.tv_sec &&
        UnitInfo.UserID == backupedUnitInfo.UserID &&
        UnitInfo.GroupID == backupedUnitInfo.GroupID &&
        UnitInfo.Size == backupedUnitInfo.Size &&
        UnitInfo.StartOffset == backupedUnitInfo.StartOffset &&
        UnitInfo.RelativePath == backupedUnitInfo.RelativePath)
    {
        // 验证文件数据是否备份正确
        switch (UnitInfo.Type)
        {
        case FILE_TYPE_FILE:
        {
            char *buffer_ori = new char[BUFF_LENGTH]; // 原始文件数据
            char *buffer_new = new char[BUFF_LENGTH]; // 备份的文件数据
            FileStream.open(RootDirectory + UnitInfo.RelativePath, std::ios::in | std::ios::binary);
            Packer->DataFile->seekg(UnitInfo.StartOffset);
            while (FileStream.peek() != EOF)
            {
                off_t tmpRealReadLength{};
                FileStream.read(buffer_ori, BUFF_LENGTH);
                tmpRealReadLength = FileStream.gcount();
                Packer->DataFile->read(buffer_new, tmpRealReadLength);
                // 如果读取的数据长度不一致，则说明文件备份失败
                if (Packer->DataFile->gcount() != tmpRealReadLength)
                {
                    Status = ERROR;
                    delete[] buffer_ori;
                    buffer_ori = nullptr;
                    delete[] buffer_new;
                    buffer_new = nullptr;
                    FileStream.close();
                    return ERROR_FILE_INFO_NOT_MATCH;
                }
                // 对比两段读取内容是否一致
                if (memcmp(buffer_ori, buffer_new, tmpRealReadLength) != 0)
                {
                    Status = ERROR;
                    delete[] buffer_ori;
                    buffer_ori = nullptr;
                    delete[] buffer_new;
                    buffer_new = nullptr;
                    FileStream.close();
                    return ERROR_FILE_INFO_NOT_MATCH;
                }
            }
            delete[] buffer_ori;
            buffer_ori = nullptr;
            delete[] buffer_new;
            buffer_new = nullptr;
            FileStream.close();
        }
        break;
        case FILE_TYPE_SYMLINK:
        {
            char *link_ori = new char[UnitInfo.Size]; // 原始链接数据
            char *link_new = new char[UnitInfo.Size]; // 备份的链接数据
            Packer->DataFile->seekg(UnitInfo.StartOffset);
            readlink((RootDirectory + UnitInfo.RelativePath).c_str(), link_ori, UnitInfo.Size);
            Packer->DataFile->read(link_new, UnitInfo.Size);
            // 如果读取的数据长度不一致，则说明文件备份失败
            if (Packer->DataFile->gcount() != UnitInfo.Size)
            {
                Status = ERROR;
                delete[] link_ori;
                link_ori = nullptr;
                delete[] link_new;
                link_new = nullptr;
                return ERROR_FILE_INFO_NOT_MATCH;
            }
            // 对比两段读取内容是否一致
            if (memcmp(link_ori, link_new, UnitInfo.Size) != 0)
            {
                Status = ERROR;
                delete[] link_ori;
                link_ori = nullptr;
                delete[] link_new;
                link_new = nullptr;
                return ERROR_FILE_INFO_NOT_MATCH;
            }
            delete[] link_ori;
            link_ori = nullptr;
            delete[] link_new;
            link_new = nullptr;
        }
        break;
        case FILE_TYPE_FIFO:
            // FIFO文件没有文件数据，不需要验证
            break;
        case FILE_TYPE_DIRECTORY:
        {
            DIR *openedDir = opendir((RootDirectory + UnitInfo.RelativePath).c_str()); // 打开一个目录
            dirent *dirEntry{};                                                        // 用于记录一个目录项
            std::priority_queue<ino_t> testQueue{};                                    // 维护目录项的顺序

            // 遍历目录，验证目录下每个目录项是否被记录
            while ((dirEntry = readdir(openedDir)) != nullptr)
            {
                // 对于特殊的.目录和..目录，不验证
                if (std::string(dirEntry->d_name) == "." || std::string(dirEntry->d_name) == "..")
                    continue;
                // 只验证FIFO、文件、链接文件和文件夹，其他的不验证
                switch (dirEntry->d_type)
                {
                case DT_FIFO:
                case DT_LNK:
                case DT_REG:
                case DT_DIR:
                    testQueue.push(dirEntry->d_ino);
                    break;
                default:
                    break;
                }
            }
            // 验证长度，长度不一致说明存在部分目录项缺失
            if (testQueue.size() != DirectoryChildrenInodesQueue.size())
            {
                Status = ERROR;
                return ERROR_FILE_INFO_NOT_MATCH;
            }
            // 按顺序依次弹出目录项，验证inode是否一致，不一致说明部分目录项备份失败
            while (!testQueue.empty())
            {
                if (testQueue.top() != DirectoryChildrenInodesQueue.top())
                {
                    Status = ERROR;
                    return ERROR_FILE_INFO_NOT_MATCH;
                }
                testQueue.pop();
                DirectoryChildrenInodesQueue.pop();
            }
        }
        break;
        default:
            Status = ERROR;
            return ERROR_FILE_INFO_NOT_MATCH;
        }
        Status = VERIFIED;
        return NO_ERROR;
    }
    else
    {
        Status = ERROR;
        return ERROR_FILE_INFO_NOT_MATCH;
    }
}

StatusCode FileInfo::Backup()
{
    if (Status != GOT_INFO)
        return ERROR_UNKNOW;

    // 将UnitFile写指针移动到备份文件的末尾，添加新的元信息数据。
    // 并记录起始偏移到DirectoryUnitOffset中，以便验证时使用
    Packer->UnitFile->seekp(0, SEEK_BASE_END);
    DirectoryUnitOffset = Packer->UnitFile->tellp();
    Packer->UnitFile->write((char *)&UnitInfo, FILE_UNIT_INFO_HEAD_SIZE);
    Packer->UnitFile->write(UnitInfo.RelativePath.c_str(), UnitInfo.RelativePath.size());

    // 获取备份文件长度，用于判断文件数据是否需要备份，（不是第一次出现的同一个inode的硬链接不用备份）
    off_t backupLength = Packer->DataFile->Length();
    switch (UnitInfo.Type)
    {
    case FILE_TYPE_FILE:
        // 如果备份起始偏移是DataFile的长度，说明需要将数据添加到DataFile尾部
        // 如果偏移不是DataFile的长度，说明数据已经在DataFile中，不用再次备份
        if (UnitInfo.StartOffset == backupLength)
        {
            char *buffer = new char[BUFF_LENGTH];
            Packer->DataFile->seekp(0, SEEK_BASE_END);
            FileStream.open((RootDirectory + UnitInfo.RelativePath).c_str(), std::ios::in | std::ios::binary);
            while (FileStream.peek() != EOF)
            {
                FileStream.read(buffer, BUFF_LENGTH);
                off_t tmpReadLength = FileStream.gcount();
                Packer->DataFile->write(buffer, tmpReadLength);
            }
            FileStream.close();
            delete[] buffer;
            buffer = nullptr;
        }
        break;
    case FILE_TYPE_FIFO:
        // FIFO文件没有数据，不需要备份
        break;
    case FILE_TYPE_SYMLINK:
        // 如果备份起始偏移是DataFile的长度，说明需要将数据添加到DataFile尾部
        // 如果偏移不是DataFile的长度，说明数据已经在DataFile中，不用再次备份
        if (UnitInfo.StartOffset == backupLength)
        {
            char *buffer = new char[UnitInfo.Size];
            readlink((RootDirectory + UnitInfo.RelativePath).c_str(), buffer, UnitInfo.Size);
            Packer->DataFile->seekp(0, SEEK_BASE_END);
            Packer->DataFile->write(buffer, UnitInfo.Size);
            delete[] buffer;
            buffer = nullptr;
        }
        break;
    case FILE_TYPE_DIRECTORY:
    {
        // 目录本身没有数据需要备份到DataFile，但需要完成对所有需要备份的目录项的遍历和备份才算备份完成，才可验证本目录的备份
        DIR *openedDir = opendir((RootDirectory + UnitInfo.RelativePath).c_str());
        dirent *dirEntry{};
        while ((dirEntry = readdir(openedDir)) != nullptr)
        {
            // 忽略.和..
            if (std::string(dirEntry->d_name) == "." || std::string(dirEntry->d_name) == "..")
                continue;
            // 只备份FIFO、文件、符号链接和目录
            switch (dirEntry->d_type)
            {
            case DT_FIFO:
            case DT_LNK:
            case DT_REG:
            case DT_DIR:
            {
                // 对每一个需要备份的目录项发起备份
                std::string fileName = UnitInfo.RelativePath + "/" + std::string(dirEntry->d_name);
                FileInfo *fileInfo = new FileInfo(fileName, Packer, RootDirectory);
                if (!fileInfo->good())
                {
                    Status = ERROR;
                    delete fileInfo;
                    fileInfo = nullptr;
                    return ERROR_UNKNOW;
                }
                if (fileInfo->Backup() != NO_ERROR)
                {
                    Status = ERROR;
                    delete fileInfo;
                    fileInfo = nullptr;
                    return ERROR_UNKNOW;
                }
                delete fileInfo;
                fileInfo = nullptr;

                // 将已备份的目录项inode记录，用于本目录的备份验证
                DirectoryChildrenInodesQueue.push(dirEntry->d_ino);
            }
            break;
            default:
                break;
            }
        }
    }
    break;
    default:
        Status = ERROR;
        return ERROR_FILE_TYPE_UNKNOWN;
    }

    // 备份完成要经过验证
    return Verify();
}

StatusCode FileInfo::Restore()
{
    if (Status != GOT_INFO)
        return ERROR_UNKNOW;

    std::string savedPath{}; // 记录已经维护的inode对应的路径
    InodeRecorder::inodeRecorderRestore->GetInodePath(UnitInfo.Inode, savedPath);
    // 如果已经维护过，则建立硬链接即可
    if (savedPath != UnitInfo.RelativePath)
    {
        link((RootDirectory + savedPath).c_str(), (RootDirectory + UnitInfo.RelativePath).c_str());
    }
    // 如果没有维护过，则正常还原文件数据
    else
    {
        switch (UnitInfo.Type)
        {
        case FILE_TYPE_FILE:
        {
            FileStream.open((RootDirectory + UnitInfo.RelativePath).c_str(), std::ios::out | std::ios::binary);
            char *buffer = new char[BUFF_LENGTH];
            size_t readLength{};
            off_t leftBytes = UnitInfo.Size;
            Packer->DataFile->seekg(UnitInfo.StartOffset);
            while (leftBytes > 0)
            {
                if (BUFF_LENGTH < leftBytes)
                    Packer->DataFile->read(buffer, BUFF_LENGTH);
                else
                    Packer->DataFile->read(buffer, leftBytes);
                readLength = Packer->DataFile->gcount();
                FileStream.write(buffer, readLength);
                leftBytes -= readLength;
            }
            delete[] buffer;
            buffer = nullptr;
            FileStream.close();
        }
        break;
        case FILE_TYPE_FIFO:
            mkfifo((RootDirectory + UnitInfo.RelativePath).c_str(), UnitInfo.Auth);
            break;
        case FILE_TYPE_SYMLINK:
        {
            size_t readLength{};
            char *buffer = new char[UnitInfo.Size];
            Packer->DataFile->seekg(UnitInfo.StartOffset);
            Packer->DataFile->read(buffer, UnitInfo.Size);
            std::string destination = std::string(buffer, UnitInfo.Size);
            symlink(destination.c_str(), (RootDirectory + UnitInfo.RelativePath).c_str());
            delete[] buffer;
        }
        break;
        case FILE_TYPE_DIRECTORY:
            mkdir((RootDirectory + UnitInfo.RelativePath).c_str(), UnitInfo.Auth);
            break;
        default:
            Status = ERROR;
            return ERROR_FILE_TYPE_UNKNOWN;
            break;
        }
    }

    // 获取备份的时间戳，并还原
    utimbuf utb{};
    utb.actime = UnitInfo.Atime.tv_sec;
    utb.modtime = UnitInfo.Atime.tv_sec;
    utime((RootDirectory + UnitInfo.RelativePath).c_str(), &utb);

    // 还原备份的权限和属主
    chmod((RootDirectory + UnitInfo.RelativePath).c_str(), UnitInfo.Auth);
    lchown((RootDirectory + UnitInfo.RelativePath).c_str(), UnitInfo.UserID, UnitInfo.GroupID);

    Status = RESTORED;
    return NO_ERROR;
}

bool FileInfo::good()
{
    return Status != ERROR;
}