#include <InodeRecorder.h>
#include <iostream>

int main(void)
{
    // test InodeRecorderBackup class
    InodeRecorderBackup backup_recorder;
    if (backup_recorder.IsInodeExists(1) == true)
    {
        std::cerr << "InodeRecorderBackup::IsInodeExits test failed" << std::endl
                  << std::endl;
    }
    backup_recorder.AddInode(1, 1);
    if (backup_recorder.IsInodeExists(1) == false)
    {
        std::cerr << "InodeRecorderBackup::AddInode test failed" << std::endl
                  << "or" << std::endl
                  << "InodeRecorderBackup::IsInodeExists testFailed" << std::endl
                  << std::endl;
    }
    backup_recorder.AddInode(2, 2);
    if (backup_recorder.IsInodeExists(2) == false)
    {
        std::cerr << "InodeRecorderBackup::AddInode test failed" << std::endl
                  << "or" << std::endl
                  << "InodeRecorderBackup::IsInodeExists testFailed" << std::endl
                  << std::endl;
    }
    if (backup_recorder.IsInodeExists(3) == true)
    {
        std::cerr << "InodeRecorderBackup::IsInodeExits test failed" << std::endl
                  << std::endl;
    }
    off_t offset;
    backup_recorder.GetInodeOffset(1, offset);
    if (offset != 1)
    {
        std::cerr << "InodeRecorderBackup::GetInodeOffset test failed" << std::endl
                  << std::endl;
    }
    backup_recorder.GetInodeOffset(2, offset);
    if (offset != 2)
    {
        std::cerr << "InodeRecorderBackup::GetInodeOffset test failed" << std::endl
                  << std::endl;
    }

    // testInodeRecorderRestore class
    InodeRecorderRestore restore_recorder;
    if (restore_recorder.IsInodeExists(1) == true)
    {
        std::cerr << "InodeRecorderRestore::IsInodeExits test failed" << std::endl
                  << std::endl;
    }
    restore_recorder.AddInode(1, "1");
    if (restore_recorder.IsInodeExists(1) == false)
    {
        std::cerr << "InodeRecorderRestore::AddInode test failed" << std::endl
                  << "or" << std::endl
                  << "InodeRecorderRestore::IsInodeExists testFailed" << std::endl
                  << std::endl;
    }
    restore_recorder.AddInode(2, "2");
    if (restore_recorder.IsInodeExists(2) == false)
    {
        std::cerr << "InodeRecorderRestore::AddInode test failed" << std::endl
                  << "or" << std::endl
                  << "InodeRecorderRestore::IsInodeExists testFailed" << std::endl
                  << std::endl;
    }
    if (restore_recorder.IsInodeExists(3) == true)
    {
        std::cerr << "InodeRecorderRestore::IsInodeExits test failed" << std::endl
                  << std::endl;
    }
    std::string path;
    restore_recorder.GetInodePath(1, path);
    if (path != "1")
    {
        std::cerr << "InodeRecorderRestore::GetInodePath test failed" << std::endl
                  << std::endl;
    }
    restore_recorder.GetInodePath(2, path);
    if (path != "2")
    {
        std::cerr << "InodeRecorderRestore::GetInodePath test failed" << std::endl
                  << std::endl;
    }

    delete InodeRecorder::inodeRecorderBackup;
    delete InodeRecorder::inodeRecorderRestore;
    std::cout << "All tests finished. If there is no error information printed out, it means all tests passed." << std::endl;
    return 0;
}