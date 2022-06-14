#include "DisplayUI.h"
#include "Netdisk_client.h"
#include "FileInfo.h"
#include "FileCompressor.h"

#define PACKED_FILE_NAME "BACKUP.BACKUP"

using namespace std;

int fd{};
bool authLog{};

int DisplayMainUI()
{
    int choose{};
    for (int i = 0; i < 7; i++)
        cout << endl;
    // system("clear;");
    cout << "Welcome to use Backup Tools!" << endl
         << endl
         << "Choose the function you want to use:" << endl;
    cout << "\t1. Sign up" << endl;
    cout << "\t2. Log in" << endl;
    cout << "\t3. Backup" << endl;
    cout << "\t4. Restore" << endl;
    cout << "\t5. Maintenance backup" << endl;
    cout << "\t6. Exit" << endl;

    do
    {
        cout << "Please choose:" << endl;
        cin >> choose;
        switch (choose)
        {
        case 1:
            SignUpUI();
            return 1;
        case 2:
            LogInUI();
            return 1;
        case 3:
            BackupUI();
            return 1;
        case 4:
            RestoreUI();
            return 1;
        case 5:
            MaintenanceBackupUI();
            return 1;
        case 6:
            return 0;
        default:
            cerr << "Wrong input!" << endl;
            break;
        }
    } while (true);
}

void SignUpUI()
{
    string username{};
    string password{};
    for (int i = 0; i < 7; i++)
        cout << endl;
    // system("clear;");
    cout << "Sign up:" << endl;
    cout << "Please enter your username:" << endl;
    cin >> username;
    cout << endl
         << "Please enter your password:" << endl;
    cin >> password;

    cout << "username:" << username << endl;
    cout << "username length:" << username.length() + 1 << endl;
    cout << "password:" << password << endl;
    cout << "password length:" << password.length() + 1 << endl;
    if (signUp(fd, username, password) == 1)
    {
        cout << "Signup successfully!" << endl;
        cout << "Will go back to main menu in 3 seconds." << endl;
        system("sleep 3");
    }
    else
    {
        cerr << "Signup failed!" << endl;
        cerr << "Will go back to main menu in 3 seconds." << endl;
        system("sleep 3");
    }
}

void LogInUI()
{
    string username{};
    string password{};
    for (int i = 0; i < 7; i++)
        cout << endl;
    // system("clear;");
    cout << "Log in:" << endl;
    cout << "Please enter your username:" << endl;
    cin >> username;
    cout << endl
         << "Please enter your password:" << endl;
    cin >> password;

    if (logIn(fd, username, password) == 1)
    {
        authLog = true;
        cout << "Login successfully!" << endl;
        cout << "Will go back to main menu in 3 seconds." << endl;
        system("sleep 3");
    }
    else
    {
        cerr << "Login failed!" << endl;
        cerr << "Will go back to main menu in 3 seconds." << endl;
        system("sleep 3");
    }
}

void BackupUI()
{
    if (!authLog)
    {
        cerr << "Please log in first!" << endl;
        cerr << "Will go back to main menu in 3 seconds." << endl;
        system("sleep 3");
        return;
    }
    string rootDirectory{};
    for (int i = 0; i < 7; i++)
        cout << endl;
    // system("clear;");
    cout << "Backup:" << endl;
    cout << "Please enter the path of the directory you want to backup:" << endl;
    cin >> rootDirectory;
    if (rootDirectory.back() == '/')
        rootDirectory.pop_back();
    // find the second last '/'
    int pos = rootDirectory.rfind('/', rootDirectory.length() - 2);
    string relativePath = rootDirectory.substr(pos + 1);
    rootDirectory = rootDirectory.substr(0, pos + 1);

    char t[] = "/tmp/BackupToolTmpFile.XXXXXX";
    string tempDirectory = mkdtemp(t);
    tempDirectory += "/";
    cout << "Step 1 of 3: Backuping..." << endl;
    FilePacker Packer = FilePacker(tempDirectory, true);
    delete InodeRecorder::inodeRecorderBackup;
    InodeRecorder::inodeRecorderBackup = new InodeRecorderBackup();
    FileInfo *fileInfo = new FileInfo(relativePath, &Packer, rootDirectory);
    if (fileInfo->Backup() != NO_ERROR)
    {
        delete fileInfo;
        fileInfo = nullptr;
        cerr << "Backup failed!" << endl;
        cerr << "Will go back to main menu in 10 seconds." << endl;
        system("sleep 10");
        return;
    }
    delete fileInfo;
    fileInfo = nullptr;
    Packer.Compact();
    cout << "Step 2 of 3: Compressing..." << endl;
    FileCompressor *fileCompressor = new FileCompressor(tempDirectory, Packer.BackupFile);
    if (fileCompressor->Compress() != NO_ERROR)
    {
        delete fileCompressor;
        cerr << "Compress failed!" << endl;
        cerr << "Will go back to main menu in 10 seconds." << endl;
        system("sleep 10");
        return;
    }
    cout << "Step 3 of 3: Uploading files..." << endl;
    if (upload(fd, tempDirectory + COMPRESSOR_FILE_NAME) == 1)
    {
        fileCompressor->DeleteFile();
        delete fileCompressor;
        Packer.DeleteFile();
        cout << "All backup process finished successfully!" << endl;
        system((string("rm -R ") + tempDirectory).c_str());
        cout << "Will go back to main menu in 3 seconds." << endl;
        system("sleep 3");
    }
    else
    {
        delete fileCompressor;
        cerr << "Backup failed! Because unknown error occurred on uploading." << endl;
        cerr << "Backup File store in " << tempDirectory << endl;
        cerr << "You can copy backup file to another position to manually backup." << endl;
        cerr << "Will go back to main menu in 10 seconds." << endl;
        system("sleep 10");
    }
}

void RestoreUI()
{
    if (!authLog)
    {
        cerr << "Please log in first!" << endl;
        cerr << "Will go back to main menu in 3 seconds." << endl;
        system("sleep 3");
        return;
    }
    string rootDirectory{};
    int backupID{};
    for (int i = 0; i < 7; i++)
        cout << endl;
    // system("clear;");
    cout << "Restore:" << endl;
    cout << "Please enter the path of the directory you want to restore to:" << endl;
    cin >> rootDirectory;
    if (rootDirectory.back() != '/')
        rootDirectory += '/';
    cout << endl
         << "Please enter the backup ID:" << endl
         << "\t(If don't know, please input '0' and use 'Maintenance backup' function to find backupID)" << endl;
    cin >> backupID;
    if (backupID == 0)
        return;

    char t[] = "/tmp/BackupToolTmpFile.XXXXXX";
    string tempDirectory = mkdtemp(t);
    tempDirectory += "/";
    cout << "Step 1 of 3: Downloading files..." << endl;
    if (download(fd, backupID, tempDirectory) == 1)
    {

        cout << "Step 2 of 3: Decompressing..." << endl;
        FileCompressor *fileCompressor = new FileCompressor(tempDirectory);
        if (fileCompressor->Decompress() != NO_ERROR)
        {
            delete fileCompressor;
            cerr << "Decompress failed!" << endl;
            cerr << "Will go back to main menu in 10 seconds." << endl;
            system("sleep 10");
            return;
        }
        fileCompressor->DeleteFile();
        delete fileCompressor;
        cout << "Step 3 of 3: Restoring" << endl;
        FilePacker Packer = FilePacker(tempDirectory, false);
        Packer.Disassemble();
        delete InodeRecorder::inodeRecorderRestore;
        InodeRecorder::inodeRecorderRestore = new InodeRecorderRestore();
        off_t ProcessBarTotal = Packer.UnitFile->Length();
        off_t ProcessBarCurrent{0};
        while (Packer.UnitFile->peek() != EOF)
        {
            FileInfo *fileInfo = new FileInfo(&Packer, rootDirectory);
            if (fileInfo->Restore() != NO_ERROR)
            {
                delete fileInfo;
                fileInfo = nullptr;
                cerr << "Restore failed!" << endl;
                cerr << "Will go back to main menu in 10 seconds." << endl;
                system("sleep 10");
                return;
            }

            char adjust = (ProcessBarCurrent >> 20) & 0x1;
            ProcessBarCurrent = Packer.UnitFile->tellg();
            if ((ProcessBarCurrent >> 20) & 0x1 == adjust + 1)
                cout << "\rRestoring: (" << ProcessBarCurrent << " / " << ProcessBarTotal << ")" << flush;
            delete fileInfo;
            fileInfo = nullptr;
        }
        Packer.DeleteFile();
        cout << "\rRestoring: (" << ProcessBarTotal << " / " << ProcessBarTotal << ")" << endl;
        cout << "All restore process finished successfully!" << endl;
        system((string("rm -R ") + tempDirectory).c_str());
        system("sleep 3");
    }
    else
    {
        cerr << "Restore failed! Because unknown error occurred on downloading." << endl;
        cerr << "Please try again." << endl;
        cerr << "Will go back to main menu in 10 seconds." << endl;
        system((string("rm -R ") + tempDirectory).c_str());
        system("sleep 10");
    }
}

void MaintenanceBackupUI()
{
    if (!authLog)
    {
        cerr << "Please log in first!" << endl;
        cerr << "Will go back to main menu in 3 seconds." << endl;
        system("sleep 3");
        return;
    }
    int backupID{};
    do
    {
        for (int i = 0; i < 7; i++)
            cout << endl;
        // system("clear;");
        cout << "Maintenance backup:" << endl;
        cout << "Following is all backup file stored on server." << endl;
        cout << "Input 0 to return to main menu. Input id shown below to delete remote backup file." << endl;
        if (searchHistory(fd) == 1)
        {
            cin >> backupID;
            if (backupID == 0)
                return;
            deleteHistory(fd, backupID);
        }
        else
        {
            cerr << "Search failed!" << endl;
            cerr << "Will go back to main menu in 3 seconds." << endl;
            system("sleep 3");
            return;
        }
    } while (true);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        delete InodeRecorder::inodeRecorderBackup;
        delete InodeRecorder::inodeRecorderRestore;
        return -1;
    }
    string ip = argv[1];
    string port = argv[2];
    fd = connectTCP(ip, port);
    authLog = false;

    while (DisplayMainUI())
        ;

    close(fd);
    delete InodeRecorder::inodeRecorderBackup;
    delete InodeRecorder::inodeRecorderRestore;
    return 0;
}