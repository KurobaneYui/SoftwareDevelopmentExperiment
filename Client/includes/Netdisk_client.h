#ifndef __NETDISK_CLIENT_H__
#define __NETDISK_CLIENT_H__

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <string>
#include <cmath>

#include "netTools.h"

size_t min(size_t a, size_t b);

// 备份历史信息结构体
struct historysend
{
    char id[17];    // 备份记录id
    char time[20];  // 日期时间字符串
    char path[129]; // 备份路径
    char size[17];  // 备份文件大小
};

// 用户注册
// fd：socket描述符
// username：用户名字符串
// password：密码字符串
int signUp(int fd, const std::string &username, const std::string &password);

// 用户登录
// fd：socket描述符
// username：用户名字符串
// password：密码字符串
int logIn(int fd, const std::string &username, const std::string &password);

// 获取当前系统时间
char *getTime();

// 上传文件
// fd：socket描述符
// path：文件路径
int upload(int fd, const std::string &path);

// 下载文件
// fd：socket描述符
// history_id：备份记录id
// package_path：保存到指定路径
int download(int fd, int history_id, const std::string &package_path);

// 删除备份记录
// fd：socket描述符
// history_id：备份记录id
int deleteHistory(int fd, int history_id);

// 查看备份记录
// fd：socket描述符
int searchHistory(int fd);

#endif