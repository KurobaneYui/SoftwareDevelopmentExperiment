#include <FileCompressor.h>
#include <iostream>

using namespace std;

int main(void)
{
    std::string RootDirectory = "./CompressorTestFolder";
    if (RootDirectory.back() != '/')
        RootDirectory += '/';

    FileRW *fr = new FileRW(RootDirectory + PACKED_FILE_NAME, false);
    if (!fr->is_open())
    {
        cerr << "FileRW cannot open, please check file path." << endl;
        delete fr;
        return 0;
    }
    FileCompressor *fc = new FileCompressor(RootDirectory, fr);
    if (fc->Compress() != NO_ERROR)
    {
        cerr << "Compress Error" << endl;
        delete fr;
        delete fc;
        return 0;
    }
    delete fr;
    delete fc;
    cout << "Compress Success" << endl;

    // ################################################################
    fc = new FileCompressor(RootDirectory);
    if (fc->Decompress() != NO_ERROR)
    {
        cerr << "Decompress Error" << endl;
        delete fc;
        return 0;
    }

    cout << "Decompress Success" << endl;
    fc->DeleteFile();
    delete fc;

    // ################################################################
    std::cout << "All tests finished. If there is no error information printed out, it means all tests passed." << std::endl;
    return 0;
}