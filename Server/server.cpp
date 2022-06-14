#include "netTools.hpp"
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "sqlcontrol.h"

#define _USE_BSD
#define QLEN 32
#define BUFSIZE 4096
#define database "netdisk.db"
#define LOGIN '1'
#define SIGNUP '2'
#define UPLOAD '3'
#define DOWNLOAD '4'
#define DELETE '5'
#define SEARCH '6'

void reaper(int sig);
int servProcess(int fd);
int logIn_s(char *buf, int &user_id);
int signUp_s(char *buf);
int upload_s(int fd, char *buf, int user_id);
int download_s(int fd, char *buf);
int delete_s(int fd, char *buf);
int search_s(int fd, int user_id);
char *getTime();

int main(int argc, char *argv[])
{
    char *service = "netdisk";
    struct sockaddr_in fsin;
    unsigned int alen;
    int msock;
    int ssock;
    sqlcontrol sql(database);
    sql.createTable();
    sql.close();

    switch (argc)
    {
    case 2:
        service = argv[1];
        break;
    default:
        errexit("usage: NetDIsk [port]\n");
        break;
    }

    msock = listenTCP(service, QLEN);

    (void)signal(SIGCHLD, reaper);

    while (1)
    {
        alen = sizeof(fsin);
        ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
        if (ssock < 0)
        {
            if (errno == EINTR)
                continue;
            errexit("accpet: %s\n", strerror(errno));
        }
        switch (fork())
        {
        case 0:
            (void)close(msock);
            exit(servProcess(ssock));
        default:
            (void)close(ssock);
            break;
        case -1:
            errexit("fork: %s\n", strerror(errno));
        }
    }
}

int servProcess(int fd)
{
    sqlcontrol sql(database);
    char buf[BUFSIZE];
    char order;
    int cc;
    bool isLogin = false;
    int user_id = 1;

    while (cc = read(fd, buf, sizeof(buf)))
    {
        if (cc < 0)
        {
            errexit("order:%s\n", strerror(errno));
        }
        switch (buf[0])
        {
        case LOGIN:
            printf("收到登录命令\n");
            switch (logIn_s(buf + 1, user_id))
            {
            case 0:
                printf("用户不存在\n");
                write(fd, "300", 4);
                break;
            case 1:
                printf("密码错误\n");
                write(fd, "300", 4);
                break;
            case 2:
                printf("登录成功\n");
                isLogin = true;
                write(fd, "200", 4);
                printf("user_id:%d\n", user_id);
                break;
            default:
                printf("登录出错");
                write(fd, "300", 4);
                break;
            }
            break;
        case SIGNUP:
            printf("收到注册命令\n");
            if (signUp_s(buf + 1) == 1)
            {
                printf("注册成功\n");
                write(fd, "200", 4);
            }
            else
            {
                printf("注册失败\n");
                write(fd, "300", 4);
            }
            break;
        case UPLOAD:
            printf("收到上传命令\n");
            if (upload_s(fd, buf + 1, user_id) == 1)
            {
                printf("上传成功\n");
                write(fd, "200", 4);
            }
            else
            {
                printf("上传失败\n");
                write(fd, "300", 4);
            }
            break;
        case DOWNLOAD:
            printf("收到下载命令\n");
            download_s(fd, buf + 1);
            printf("用户下载成功\n");
            break;
        case DELETE:
            printf("收到删除命令\n");
            if (delete_s(fd, buf + 1) == 1)
            {
                printf("删除成功\n");
                write(fd, "200", 4);
            }
            else
            {
                printf("删除失败\n");
                write(fd, "300", 4);
            }
            break;
        case SEARCH:
            printf("收到搜索命令\n");
            search_s(fd, user_id);
            printf("搜索成功\n");
            break;
        default:
            break;
        }
    }

    return 0;
}

void reaper(int sig)
{
    int status;

    while (wait3(&status, WNOHANG, (struct rusage *)0) >= 0)
        ;
}

char *getTime()
{
    int year, month, day, hour, minute, second;
    time_t rawTime;
    struct tm *pstTime;
    char *buf = (char *)malloc(sizeof(char) * 20);
    time(&rawTime);
    pstTime = localtime(&rawTime);
    year = pstTime->tm_year + 1900;
    month = pstTime->tm_mon + 1;
    day = pstTime->tm_mday;
    hour = pstTime->tm_hour;
    minute = pstTime->tm_min;
    second = pstTime->tm_sec;
    char year_s[5], month_s[3], day_s[3], hour_s[3], minute_s[3], second_s[3];
    sprintf(year_s, "%d", year);
    if (month < 10)
    {
        sprintf(month_s, "0%d", month);
    }
    else
    {
        sprintf(month_s, "%d", month);
    }
    if (day < 10)
    {
        sprintf(day_s, "0%d", day);
    }
    else
    {
        sprintf(day_s, "%d", day);
    }
    if (hour < 10)
    {
        sprintf(hour_s, "0%d", hour);
    }
    else
    {
        sprintf(hour_s, "%d", hour);
    }
    if (minute < 10)
    {
        sprintf(minute_s, "0%d", minute);
    }
    else
    {
        sprintf(minute_s, "%d", minute);
    }
    if (second < 10)
    {
        sprintf(second_s, "0%d", second);
    }
    else
    {
        sprintf(second_s, "%d", second);
    }
    snprintf(buf, 20, "%s+%s+%s+%s+%s+%s", year_s, month_s, day_s, hour_s, minute_s, second_s);
    printf("%s\n", buf);
    return buf;
}

int logIn_s(char *buf, int &user_id)
{
    char *username = buf;
    char *password = buf + 17;
    sqlcontrol sql(database);
    int rc = sql.checkPassword(username, password);
    printf("username:%s try to log in.\n", username);
    if (rc == 2)
    {
        user_id = sql.getUserId(username);
    }
    sql.close();
    return rc;
}

int signUp_s(char *buf)
{
    char *username = buf;
    char *password = buf + 17;
    sqlcontrol sql(database);
    printf("username:%s\n", username);
    printf("password:%s\n", password);
    int rc = sql.addUser(username, password);
    sql.close();
    return rc;
}

int upload_s(int fd, char *buf, int user_id)
{
    char *filesize_s = buf;
    long filesize = atoi(filesize_s);
    char *filepath_old = buf + 17;
    char filepath[100];
    char filepackage[100];
    char *time_s = buf + 146;
    int inchars, outchars, n, count;
    FILE *fp = NULL;
    sqlcontrol sql(database);

    printf("filesize:%s\n", filesize_s);
    printf("file old path:%s\n", filepath_old);
    printf("time:%s\n", time_s);
    write(fd, "200", 4);
    printf("file begin upload!\n");

    char *currentTime = getTime();
    sprintf(filepackage, "backup_file/%d", user_id);
    sprintf(filepath, "backup_file/%d/%s", user_id, currentTime);
    printf("file saved at:%s\n", filepath);
    free(currentTime);

    if (access(filepackage, NULL) != 0)
    {
        if (mkdir(filepackage, 0755) == -1)
        {
            printf("mkdir   error\n");
            return -1;
        }
    }
    fp = fopen(filepath, "wb+");

    if (fp == NULL)
    {
        printf("The file was not created! \n");
    }

    count = 0;
    char filebuf[4096];
    while (count < filesize)
    {
        if (filesize - count > 4096)
        {
            outchars = 4096;
            for (inchars = 0; inchars < outchars; inchars += n)
            {
                n = read(fd, &filebuf[inchars], outchars - inchars);
                if (n < 0)
                    errexit("socket read failed: %s\n", strerror(errno));
            }
            fwrite(&filebuf, 1, outchars, fp);
        }
        else
        {
            outchars = filesize - count;
            for (inchars = 0; inchars < outchars; inchars += n)
            {
                n = read(fd, &filebuf[inchars], outchars - inchars);
                if (n < 0)
                    errexit("socket read failed: %s\n", strerror(errno));
            }
            fwrite(&filebuf, 1, outchars, fp);
        }
        count = count + outchars;
    }

    printf("file upload success!\n");
    fclose(fp);
    sql.addHistory(time_s, user_id, filepath, filepath_old, filesize);
    sql.close();
    return 1;
}

char *getFileName(char *path)
{
    char *name;
    char temp = '0';
    int i = 0, j = 0;

    name = (char *)malloc(sizeof(char) * 33);
    while (temp != '\0')
    {
        temp = path[i++];
        if (temp == '/')
        {
            j = 0;
        }
        else
        {
            name[j++] = temp;
        }
    }
    name[j] = '\0';
    return name;
}

int download_s(int fd, char *buf)
{
    long history_id;
    char *path;
    FILE *fp = NULL;
    sqlcontrol sql(database);
    long filesize;
    history his;
    char send_buf[50];
    char *filename;
    int inchars, outchars, n, count;

    history_id = atoi(buf);
    printf("download history_id:%d\n", history_id);
    sql.getHistoryByID(history_id, his);
    sql.close();

    filename = getFileName(his.oldpath);
    printf("filename:%s\n", filename);
    sprintf(send_buf, "%d", his.size);
    sprintf(send_buf + 17, "%s", filename);

    write(fd, send_buf, 50);

    path = his.path;
    fp = fopen(path, "rb+");

    if (fp == NULL)
    {
        printf("The file can't open! \n");
    }
    filesize = his.size;
    count = 0;
    char filebuf[4096];
    while (count < filesize)
    {
        if (filesize - count > 4096)
        {
            outchars = 4096;
            fread(&filebuf, 1, outchars, fp);
            for (inchars = 0; inchars < outchars; inchars += n)
            {
                n = write(fd, &filebuf[inchars], outchars - inchars);
                if (n < 0)
                {
                    free(filename);
                    errexit("socket read failed: %s\n", strerror(errno));
                }
            }
        }
        else
        {
            outchars = filesize - count;
            fread(&filebuf, 1, outchars, fp);
            for (inchars = 0; inchars < outchars; inchars += n)
            {
                n = write(fd, &filebuf[inchars], outchars - inchars);
                if (n < 0)
                {
                    free(filename);
                    errexit("socket read failed: %s\n", strerror(errno));
                }
            }
        }
        count = count + outchars;
    }
    fclose(fp);
    free(filename);

    return 1;
}

int delete_s(int fd, char *buf)
{
    sqlcontrol sql(database);
    int id = atoi(buf);
    int rc;
    rc = sql.deleteHistory(id);
    sql.close();
    if (rc == 1)
    {
        return 1;
    }
    return 0;
}

struct historysend
{
    char id[17];
    char time[20];
    char path[129];
    char size[17];
};

int search_s(int fd, int user_id)
{
    sqlcontrol sql(database);
    historyList list;
    int num, i;
    char his_num_s[17];
    char respond[4];
    historysend *histories;
    int outchars, inchars, n;

    list.list = nullptr;
    sql.getAllHistoryByUserId(user_id, list);
    num = list.length;
    printf("history num:%d\n", num);
    sprintf(his_num_s, "%d", num);
    write(fd, his_num_s, 17);
    read(fd, respond, 4);

    if (strcmp(respond, "200"))
    {
        printf("don't recive respond\n");
	if (list.list!=nullptr) {free(list.list); list.list=nullptr;}
        return 0;
    }
    else
    {
        printf("recived respond\n");
        histories = (historysend *)malloc(sizeof(historysend) * num);
        for (i = 0; i < num; i++)
        {
            sprintf(histories[i].id, "%d", list.list[i].id);
            sprintf(histories[i].time, "%s", list.list[i].time);
            sprintf(histories[i].path, "%s", list.list[i].oldpath);
            sprintf(histories[i].size, "%d", list.list[i].size);
        }
        char buf[sizeof(historysend) * num];
        memcpy(buf, histories, sizeof(historysend) * num);

        outchars = sizeof(historysend) * num;
        for (inchars = 0; inchars < outchars; inchars += n)
        {
            n = write(fd, &buf[inchars], outchars - inchars);
            if (n < 0)
            {
                free(histories);
                errexit("socket read failed: %s\n", strerror(errno));
            }
        }
        free(histories);
	if (list.list!=nullptr) {free(list.list); list.list=nullptr;}
        return 1;
    }

    return 0;
}
