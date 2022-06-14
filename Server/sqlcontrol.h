#include<sqlite3.h>
#include<stdlib.h>

typedef struct history
{
    int id;
    char *time;
    int user_id;
    char *path;
    char *oldpath;
    int size;
};

typedef struct historyList
{
    history* list;
    int length;
};

class sqlcontrol{
    private:
        sqlite3 *db;//数据库
    public:
        sqlcontrol(char *path);//类构造函数
        void close();//关闭数据库连接

        int excuteSql(char *sql);//执行具体命令

        int createTable();//创建所有表

        int addUser(char *name,char *password);//添加用户

        int checkPassword(char *name,char *password);//检查密码是否正确

        int getUserId(char *name);//根据用户名获取用户ID

        int addHistory(char *time,int user_id,char *path,char *oldpath,int size);//添加用户备份历史

        int getHistoryByID(int history_id,history &history);//通过历史id获取备份文件路径

        int getAllHistoryByUserId(int user_id, historyList &list);//通过用户ID获取其所有备份历史

        int deleteHistory(int id);//按历史ID删除
};

static int callback(void *data, int argc, char **argv, char **azColName);