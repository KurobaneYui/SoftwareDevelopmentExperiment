#include <FileInfo.h>
#include <iostream>

int main(void)
{
    std::string rootDirectory = "../test_directory";
    if (rootDirectory.back() == '/')
        rootDirectory.pop_back();
    int pos = rootDirectory.rfind('/', rootDirectory.length() - 2);
    std::string relativePath = rootDirectory.substr(pos + 1);
    rootDirectory = rootDirectory.substr(0, pos + 1);
    std::string tempDirectory = "./BackupFileTemporyDirectory";
    tempDirectory += "/";
    FilePacker *Packer = new FilePacker(tempDirectory, true);
    delete InodeRecorder::inodeRecorderBackup;
    InodeRecorder::inodeRecorderBackup = new InodeRecorderBackup();
    FileInfo *fileInfo = new FileInfo(relativePath, Packer, rootDirectory);
    if (fileInfo->Backup() != NO_ERROR)
    {
        delete InodeRecorder::inodeRecorderBackup;
        delete InodeRecorder::inodeRecorderRestore;
        delete fileInfo;
        fileInfo = nullptr;
        std::cerr << "Backup failed!" << std::endl;
        return 0;
    }
    delete fileInfo;
    fileInfo = nullptr;

    std::cout << "BackupFileTest backup test Success!" << std::endl;

    if (Packer->Compact() != NO_ERROR)
    {
        delete InodeRecorder::inodeRecorderBackup;
        delete InodeRecorder::inodeRecorderRestore;
        delete fileInfo;
        fileInfo = nullptr;
        std::cerr << "Compact failed!" << std::endl;
        return 0;
    }
    delete fileInfo;
    delete Packer;

    std::cout << "BackupFileTest compact test Success!" << std::endl;

    // #############################################################################

    rootDirectory = "./BackupFileRestoreFolder/";
    if (rootDirectory.back() != '/')
        rootDirectory += '/';
    Packer = new FilePacker(tempDirectory, false);
    if (Packer->Disassemble() != NO_ERROR)
    {
        delete InodeRecorder::inodeRecorderBackup;
        delete InodeRecorder::inodeRecorderRestore;
        std::cerr << "Disassemble failed!" << std::endl;
        return 0;
    }

    std::cout << "BackupFileTest disassemble test Success!" << std::endl;

    delete InodeRecorder::inodeRecorderRestore;
    InodeRecorder::inodeRecorderRestore = new InodeRecorderRestore();
    while (Packer->UnitFile->peek() != EOF)
    {
        fileInfo = new FileInfo(Packer, rootDirectory);
        if (fileInfo->Restore() != NO_ERROR)
        {
            delete InodeRecorder::inodeRecorderBackup;
            delete InodeRecorder::inodeRecorderRestore;
            delete fileInfo;
            fileInfo = nullptr;
            std::cerr << "Restore failed!" << std::endl;
            return 0;
        }
        delete fileInfo;
        fileInfo = nullptr;
    }
    delete Packer;

    std::cout << "BackupFileTest restore test Success!" << std::endl;

    delete InodeRecorder::inodeRecorderBackup;
    delete InodeRecorder::inodeRecorderRestore;

    // #########################################################################
    std::cout << std::endl
              << "All tests finished. If there is no error information printed out, it means all tests passed." << std::endl;
    return 0;
}