#include <FilePacker.h>
#include <iostream>
#include <cstring>

int main(void)
{
    FileRW testFileRW("./testFileRWTruncate", true);
    if (!testFileRW.good())
    {
        std::cerr << "testFileRW open failed" << std::endl;
        return 0;
    }
    testFileRW.seekp(0, SEEK_BASE_END);
    testFileRW.write("testFileRW", 10);
    off_t length = testFileRW.Length();
    if (length != 10)
    {
        std::cerr << "testFileRW length error" << std::endl;
        return 0;
    }
    char *buffer = new char[length];
    testFileRW.seekg(0);
    testFileRW.read(buffer, length);
    if (testFileRW.gcount() != length)
    {
        std::cerr << "testFileRW read error" << std::endl;
        delete[] buffer;
        return 0;
    }
    if (memcmp(buffer, "testFileRW", length) != 0)
    {
        std::cerr << "testFileRW read or write error" << std::endl;
        delete[] buffer;
        return 0;
    }
    delete[] buffer;
    if (testFileRW.Path() != std::string("./testFileRWTruncate"))
    {
        std::cerr << "testFileRW path error" << std::endl;
        return 0;
    }
    testFileRW.close();
    // #########################################################################
    FileRW testFileRW2("./testFileRWTruncate", true);
    if (!testFileRW2.good())
    {
        std::cerr << "testFileRW2 open failed" << std::endl;
        return 0;
    }
    testFileRW2.seekp(0, SEEK_BASE_END);
    testFileRW2.write("testFileRW2", 11);
    length = testFileRW2.Length();
    if (length != 11)
    {
        std::cerr << "testFileRW2 length error" << std::endl;
        return 0;
    }
    buffer = new char[length];
    testFileRW2.seekg(0);
    testFileRW2.read(buffer, length);
    if (testFileRW2.gcount() != length)
    {
        std::cerr << "testFileRW2 read error" << std::endl;
        delete[] buffer;
        return 0;
    }
    if (memcmp(buffer, "testFileRW2", length) != 0)
    {
        std::cerr << "testFileRW2 read or write error" << std::endl;
        delete[] buffer;
        return 0;
    }
    delete[] buffer;
    if (testFileRW2.Path() != std::string("./testFileRWTruncate"))
    {
        std::cerr << "testFileRW2 path error" << std::endl;
        return 0;
    }
    testFileRW2.close();
    // #########################################################################
    FileRW testFileRW3("./testFileRWTruncate", false);
    if (!testFileRW3.good())
    {
        std::cerr << "testFileRW3 open failed" << std::endl;
        return 0;
    }
    testFileRW3.seekp(0, SEEK_BASE_END);
    testFileRW3.write("testFileRW3", 11);
    length = testFileRW3.Length();
    if (length != 22)
    {
        std::cerr << "testFileRW3 length error" << std::endl;
        return 0;
    }
    buffer = new char[length];
    testFileRW3.seekg(0);
    testFileRW3.read(buffer, length);
    if (testFileRW3.gcount() != length)
    {
        std::cerr << "testFileRW3 read error" << std::endl;
        delete[] buffer;
        return 0;
    }
    if (memcmp(buffer, "testFileRW2testFileRW3", length) != 0)
    {
        std::cerr << "testFileRW3 read or write error" << std::endl;
        delete[] buffer;
        return 0;
    }
    delete[] buffer;
    if (testFileRW3.Path() != std::string("./testFileRWTruncate"))
    {
        std::cerr << "testFileRW3 path error" << std::endl;
        return 0;
    }
    if (testFileRW3.peek() != EOF)
    {
        std::cerr << "testFileRW3 peek error" << std::endl;
        return 0;
    }
    testFileRW3.close();
    // #########################################################################
    std::cout << "All tests finished. If there is no error information printed out, it means all tests passed." << std::endl;
    return 0;
}