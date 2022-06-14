#include "Netdisk_client.h"

size_t min(size_t a, size_t b)
{
    return a < b ? a : b;
}

int logIn(int fd, const std::string &username, const std::string &password)
{
    // 信号缓冲区
    char buf[36]{};

    // 发送登录信息，限制用户名和密码长度
    buf[0] = '1';
    memcpy(buf + 1, username.c_str(), min(username.size(), 16));
    memcpy(buf + 18, password.c_str(), min(password.size(), 17));
    int outchars = sizeof(buf);
    write(fd, buf, outchars);

    // 接收返回值
    read(fd, buf, 4);
    if (memcmp(buf, "200", 4))
    {
        printf("user login fail\n");
        return 0;
    }
    else
    {
        printf("user login success\n");
        return 1;
    }
    printf("user add fail\n");
    return 0;
}

int signUp(int fd, const std::string &username, const std::string &password)
{
    // 信号缓冲区
    char buf[36]{};

    // 发送登录信息，限制用户名和密码长度
    buf[0] = '2';
    memcpy(buf + 1, username.c_str(), min(username.size(), 16));
    memcpy(buf + 18, password.c_str(), min(password.size(), 17));
    buf[17] = '\0';
    buf[35] = '\0';
    int outchars = sizeof(buf);
    write(fd, buf, outchars);

    // 接收返回值
    read(fd, buf, 4);
    if (memcmp(buf, "200", 4))
    {
        printf("user add fail\n");
        return 0;
    }
    else
    {
        printf("user add success\n");
        return 1;
    }
    printf("user add fail\n");
    return 0;
}

char *getTime()
{
    time_t rawTime{};
    tm *pstTime{};
    char *buf = new char[20];

    // 获取当前系统时间
    time(&rawTime);
    pstTime = localtime(&rawTime);

    // 转换为字符串
    strftime(buf, 20, "%Y/%m/%d %H:%M:%S", pstTime);

    return buf;
}

int upload(int fd, const std::string &path)
{
    FILE *fp{nullptr};     // 文件指针
    long filesize{0};      // 文件大小
    char filesize_s[17]{}; // 文件大小字符串
    char *time_s{nullptr}; // 时间字符串指针
    char buf[4096]{};      // 发送数据缓冲区
    int inchars{};         // 已读取字符数
    int outchars{};        // 已发送发送字符数
    int needchars{};       // 需要发送或接收的字符数

    // 打开文件
    if ((fp = fopen(path.c_str(), "rb")) == nullptr)
    {
        printf("cannot open this file\n");
        return 0;
    }

    // 读取文件长度，并转换为字符串
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    sprintf(filesize_s, "%ld", filesize);

    // 获取当前时间
    time_s = getTime();

    // 发送上传请求头
    buf[0] = '3';
    memcpy(buf + 1, filesize_s, 17);
    memcpy(buf + 18, path.c_str(), min(129, path.size()));
    memcpy(buf + 147, time_s, 20);
    write(fd, buf, 168);
    delete[] time_s;
    time_s = nullptr;

    // 接收服务器响应
    inchars = 0;
    needchars = 4;
    while (inchars < needchars)
    {
        int n = read(fd, buf + inchars, needchars - inchars);
        if (n < 0)
            errexit("socket read failed: %s\n", strerror(errno));
        inchars += n;
    }
    if (memcmp(buf, "200", 4))
    {
        printf("file upload fail\n");
    }
    else
    {
        // 发送要上传的文件
        fseek(fp, 0, SEEK_SET);
        printf("file could upload\n");
        printf("filesize:%ld\n", filesize);
        int count{0}; // 文件已读取长度
        while (count < filesize)
        {
            if (filesize - count > 4096)
            {
                outchars = 0;
                needchars = 4096;
                bzero(buf, needchars);
                fread(buf, 1, needchars, fp);
                while (outchars < needchars)
                {
                    int n = write(fd, buf + outchars, needchars - outchars);
                    if (n < 0)
                        errexit("socket write failed: %s\n", strerror(errno));
                    outchars += n;
                }
            }
            else
            {
                outchars = 0;
                needchars = filesize - count;
                bzero(buf, needchars);
                fread(buf, 1, needchars, fp);
                printf("filesize:%ld\n", filesize);
                while (outchars < needchars)
                {
                    int n = write(fd, buf + outchars, needchars - outchars);
                    if (n < 0)
                        errexit("socket write failed: %s\n", strerror(errno));
                    outchars += n;
                }
            }
            count += outchars;
            printf("上传进度:(%d/%ld)\r", count, filesize);
            fflush(stdout);
        }
    }
    fclose(fp);
    fp = nullptr;

    // 读取服务器响应
    read(fd, buf, 4);
    if (memcmp(buf, "200", 4))
    {
        printf("\nupload fail\n");
        return 0;
    }
    else
    {
        printf("\nupload success\n");
        return 1;
    }
}

int download(int fd, int history_id, const std::string &package_path)
{
    FILE *fp{nullptr};   // 下载数据保存文件描述符
    char buf[18]{};      // 发送数据缓冲区
    char fileInfo[50]{}; // 文件信息缓冲区
    char *filename{};    // 文件名缓冲区
    int filesize{};      //文件大小
    int inchars{};       //已读字符数
    int outchars{};      //已发送字符数
    int needchars{};     //需要发送或接收的字符数

    // 发送下载请求头
    buf[0] = '4';
    sprintf(buf + 1, "%d", history_id);
    write(fd, buf, 18);

    // 读取文件信息
    needchars = 50;
    inchars = 0;
    while (inchars < needchars)
    {
        int n = read(fd, &fileInfo[inchars], needchars - inchars);
        if (n < 0)
            errexit("socket read failed: %s\n", strerror(errno));
        inchars += n;
    }
    filesize = atoi(fileInfo);
    filename = fileInfo + 17;
    printf("filesize:%d,filename:%s\n", filesize, filename);

    // 打开文件以保存数据
    std::string path = package_path + "/" + filename;
    if ((fp = fopen(path.c_str(), "wb+")) == nullptr)
    {
        printf("cannot open this file\n");
        return 0;
    }

    // 下载并写入数据
    int count{0};         // 文件写入大小
    char filebuf[4096]{}; // 文件缓冲区
    while (count < filesize)
    {
        if (filesize - count > 4096)
        {
            needchars = 4096;
            inchars = 0;
            while (inchars < needchars)
            {
                int n = read(fd, filebuf + inchars, needchars - inchars);
                if (n < 0)
                    errexit("socket read failed: %s\n", strerror(errno));
                inchars += n;
            }
            fwrite(filebuf, 1, needchars, fp);
        }
        else
        {
            inchars = 0;
            needchars = filesize - count;
            while (inchars < needchars)
            {
                int n = read(fd, filebuf + inchars, needchars - inchars);
                if (n < 0)
                    errexit("socket read failed: %s\n", strerror(errno));
                inchars += n;
            }
            fwrite(filebuf, 1, needchars, fp);
        }
        count += needchars;
        printf("下载进度:(%d/%d)\r", count, filesize);
        fflush(stdout);
    }
    printf("\ndownload success!\n");
    fclose(fp);
    return 1;
}

int deleteHistory(int fd, int history_id)
{
    // 请求头缓冲区
    char buf[18]{};

    // 发送删除请求头
    buf[0] = '5';
    sprintf(buf + 1, "%d", history_id);
    write(fd, buf, 18);

    // 读取服务器响应
    read(fd, buf, 4);
    if (memcmp(buf, "200", 4))
    {
        printf("delete fail\n");
        return 0;
    }
    else
    {
        printf("delete success\n");
        return 1;
    }
}

int searchHistory(int fd)
{
    char his_num_s[17]{};       // 历史记录数字符串
    int needchars{}, inchars{}; // 需要读取的字节数，已读取的字节数

    // 发送请求头
    write(fd, "6", 1);
    // 读取历史记录数
    int n = read(fd, his_num_s, 17);
    int his_num = atoi(his_num_s);
    if (n = 17)
    {
        // 确认记录数接收正常
        write(fd, "200", 4);

        // 开辟历史记录数据缓冲区
        historysend histories[his_num];
        // 接收历史记录
        needchars = sizeof(historysend) * his_num;
        while (inchars < needchars)
        {
            n = read(fd, ((char *)histories + inchars), needchars - inchars);
            if (n < 0)
                errexit("socket read failed: %s\n", strerror(errno));
            inchars += n;
        }

        // 打印历史记录
        printf("hitory_num:%d\n", his_num);
        for (int i = 0; i < his_num; i++)
        {
            printf("id:%s,time:%s,path:%s,size:%s\n", histories[i].id, histories[i].time, histories[i].path, histories[i].size);
        }

        return 1;
    }
    return 0;
}
