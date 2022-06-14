#include"sqlcontrol.h"
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<string.h>
#include<malloc.h>
using namespace std;

#define MAXBUF 300


static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

sqlcontrol::sqlcontrol(char *path){
    int rc;
    rc = sqlite3_open(path, &this->db);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }
}

void sqlcontrol::close(){
    sqlite3_close(this->db);
}

int sqlcontrol::excuteSql(char *sql){
    int rc;
    char *errmsg=0;
    rc = sqlite3_exec(this->db,sql,callback,0,&errmsg);
    if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", errmsg);
      return -1;
   }else{
      fprintf(stdout, "Operation done successfully\n");
   }
   return 1;
}

int sqlcontrol::createTable(){
   excuteSql("create table user(ID INTEGER PRIMARY KEY AUTOINCREMENT,USERNAME TEXT NOT NULL UNIQUE,PASSWORD CHAR(50))");
   excuteSql("create table history(ID INTEGER PRIMARY KEY AUTOINCREMENT,TIME TEXT NOT NULL,USER INT,PATH TEXT NOT NULL, OLDPATH TEXT NOT NULL, SIZE INT NOT NULL, CONSTRAINT fk_user FOREIGN KEY (USER) REFERENCES user(ID))");
   return 1;
}

int sqlcontrol::addUser(char *name, char *password){
   int rc;
   char *errmsg=0;
   char sql[MAXBUF];

   sprintf(sql, "insert into user(USERNAME,PASSWORD) values('%s','%s');", name, password);

   rc = sqlite3_exec(db, sql, callback, 0, &errmsg);
   
   if(SQLITE_OK != rc)
   {
      fprintf(stderr, "SQL error: %s\n", errmsg);
      return -1;
   }
   else
   {
      fprintf(stdout, "insert table successfully\n");
   }
   return 1;
}

int sqlcontrol::checkPassword(char* name,char* password){
   //返回-1为查询失败，0为找不到结果，1为密码错误，2为密码正确
   int rc;
   char *errmsg=0;
   char sql[MAXBUF];
   char **result;
   int row,column;

   sprintf(sql, "select password from user where username='%s';",name);

   rc=sqlite3_get_table(this->db,sql,&result,&row,&column,&errmsg);
   if(SQLITE_OK != rc)
   {
      fprintf(stderr, "SQL error: %s\n", errmsg);
      return -1;
   }
   else
   {
      fprintf(stdout, "select successfully\n");
   }
   
   if(row==0) return 0;
   if(!strcmp(result[column],password)){
      return 2;
   }

   sqlite3_free_table(result);
   return 1;
}

int sqlcontrol::getUserId(char *name){
   //返回0为查询失败，正确应返回用户ID
   int rc;
   char *errmsg=0;
   char sql[MAXBUF];
   char **result;
   int row,column;
   int id;

   sprintf(sql, "select id from user where username='%s';",name);

   rc=sqlite3_get_table(this->db,sql,&result,&row,&column,&errmsg);
   if(SQLITE_OK != rc)
   {
      fprintf(stderr, "SQL error: %s\n", errmsg);
      return -1;
   }
   else
   {
      fprintf(stdout, "select successfully\n");
   }
   
   if(row==0) return 0;

   id=atoi(result[column]);

   sqlite3_free_table(result);
   return id;
}

int sqlcontrol::addHistory(char *time,int user_id,char *path,char *oldpath,int size){
   int rc;
   char *errmsg=0;
   char sql[MAXBUF];

   sprintf(sql, "insert into history(TIME,USER,PATH,OLDPATH,SIZE) values('%s',%d,'%s','%s',%d);", time,user_id,path,oldpath,size);

   rc = sqlite3_exec(db, sql, callback, 0, &errmsg);
   
   if(SQLITE_OK != rc)
   {
      fprintf(stderr, "SQL error: %s\n", errmsg);
      return -1;
   }
   else
   {
      fprintf(stdout, "insert table successfully\n");
   }
   return 1;
}

int sqlcontrol::getHistoryByID(int history_id,history &his){
   //返回0为失败，正确应返回1
   int rc;
   char *errmsg=0;
   char sql[MAXBUF];
   char **result;
   int row,column;

   sprintf(sql, "select * from history where id=%d;",history_id);
   rc=sqlite3_get_table(this->db,sql,&result,&row,&column,&errmsg);
   if(SQLITE_OK != rc)
   {
      fprintf(stderr, "SQL error: %s\n", errmsg);
      return 0;
   }
   else
   {
      fprintf(stdout, "select successfully\n");
   }
   if(row==0) return 0;
   
   his.id = atoi(result[column++]);
   his.time = result[column++];
   his.user_id = atoi(result[column++]);
   his.path = result[column++];
   his.oldpath = result[column++];
   his.size = atoi(result[column++]);

   return 1;
}

int sqlcontrol::getAllHistoryByUserId(int user_id,historyList &list){
   //返回0为查询失败，正确应返回用户ID
   int rc;
   char *errmsg=0;
   char sql[MAXBUF];
   char **result;
   int row,column;

   sprintf(sql, "select * from history where user=%d order by time desc;",user_id);

   rc=sqlite3_get_table(this->db,sql,&result,&row,&column,&errmsg);
   if(SQLITE_OK != rc)
   {
      fprintf(stderr, "SQL error: %s\n", errmsg);
      return -1;
   }
   else
   {
      fprintf(stdout, "select successfully\n");
   }
   
   if(row==0){
      list.length = row;
      return 0;
   } 

   list.length = row;
   list.list = (history*)malloc(sizeof(history)*row);

   int begin = column;
   for(int i=0; i<row; i++){
      list.list[i].id = atoi(result[begin++]);
      list.list[i].time = result[begin++];
      list.list[i].user_id = atoi(result[begin++]);
      list.list[i].path = result[begin++];
      list.list[i].oldpath = result[begin++];
      list.list[i].size = atoi(result[begin++]);
      printf("id:%d,user_id:%d,time:%s,path:%s,old_path:%s,size:%d\n",list.list[i].id,list.list[i].user_id,list.list[i].time,list.list[i].path,list.list[i].oldpath,list.list[i].size);
   }

   return 1;
}

int sqlcontrol::deleteHistory(int id){
   int rc;
   char sql[MAXBUF];
   sprintf(sql,"delete from history where id=%d;",id);
   rc=excuteSql(sql);
   
   return rc;
}
