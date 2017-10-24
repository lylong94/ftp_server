#include"mysql.h"
//编译加-lmysqlclient
#define mysql_host "localhost"
#define mysql_user "root"
#define mysql_pswd "123"
#define mysql_database "ftp"

//插入用户,成功0,失败-1
int mysql_insert(char *name,char *salt,char *passwd)
{
	MYSQL *conn;
	conn=mysql_init(NULL);
	int ret;
	char query[512];
	bzero(query,sizeof(query));
	sprintf(query,"insert into user(name,salt,password) value('%s','%s','%s')",name,salt,passwd);
	if(!mysql_real_connect(conn,mysql_host,mysql_user,mysql_pswd,mysql_database,0,NULL,0))
	{
		printf("mysql_real_connect:%s\n",mysql_error(conn));
		return -1;
	}
	ret=mysql_query(conn,query);
	if(ret)
	{
		printf("mysql_query:%s\n",mysql_error(conn));
		return -1;
	}else{
		printf("insert sucess\n");
	}
	mysql_close(conn);
	return 0;
}

//查询失败返回-1,查询成功返回1,空查询返回0,如果存在buf存放结果
int mysql_select(char *what,int id,char *buf)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	conn=mysql_init(NULL);
	int ret;
	char query[512];
	bzero(query,sizeof(query));
	sprintf(query,"select %s from user where id=%d",what,id);
	if(!mysql_real_connect(conn,mysql_host,mysql_user,mysql_pswd,mysql_database,0,NULL,0))
	{
		printf("mysql_real_connect:%s\n",mysql_error(conn));
		return -1;
	}
	ret=mysql_query(conn,query);//查询
	if(ret)
	{
		printf("mysql_query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
	res=mysql_use_result(conn);
	if(res)
	{
		row=mysql_fetch_row(res);
		if(NULL!=row)
		{
			strcpy(buf,row[0]);
			mysql_free_result(res);
			mysql_close(conn);
			return 1;
		}else{
			mysql_free_result(res);
			mysql_close(conn);
			return 0;
		}
	}else{
		printf("mysql_use_result:%s",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
}

//查询用户名是否存在,错误返回-1,存在返回id,不存在返回0
int mysql_isuser(char *name)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	conn=mysql_init(NULL);
	int ret;
	int id;
	char query[512];
	bzero(query,sizeof(query));
	sprintf(query,"select id from user where name='%s'",name);
	if(!mysql_real_connect(conn,mysql_host,mysql_user,mysql_pswd,mysql_database,0,NULL,0))
	{
		printf("mysql_real_connect:%s\n",mysql_error(conn));
		return -1;
	}
	ret=mysql_query(conn,query);
	if(ret)
	{
		printf("mysql_query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
	res=mysql_use_result(conn);
	if(res)
	{
		row=mysql_fetch_row(res);
		if(NULL!=row)
		{
			id=atoi(row[0]);
			mysql_free_result(res);
			mysql_close(conn);
			return id;
		}else{
			mysql_free_result(res);
			mysql_close(conn);
			return 0;
		}
	}else{
		printf("mysql_use_result:%s",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}
}

//修改密码,成功0,失败-1
int mysql_update(int id,char *passwd)
{
	MYSQL *conn;
	conn=mysql_init(NULL);
	int ret;
	char query[512];
	bzero(query,sizeof(query));
	sprintf(query,"update user set password='%s' where id=%d",passwd,id);
	if(!mysql_real_connect(conn,mysql_host,mysql_user,mysql_pswd,mysql_database,0,NULL,0))
	{
		printf("mysql_real_connect:%s\n",mysql_error(conn));
		return -1;
	}
	ret=mysql_query(conn,query);
	if(ret)
	{
		printf("mysql_query:%s\n",mysql_error(conn));
		mysql_close(conn);
		return -1;
	}else{
		return 0;
	}
}

//int main()
//{
//	int ret;
//	char buf[128]={0};
//	//ret=mysql_insert("name","salt","password");
//	//ret=mysql_isuser("name");//查询用户名是否被占用
//	
//	//id由mysql_isuser()得到，在登录阶段得到
//	
//	//ret=mysql_select("salt",2,buf);//123用户不存在返回0,改为id
//	//ret=mysql_select("password",1,buf);//返回1,buf为密文
//	ret=mysql_update(1,"123");//修改密码
//	if(-1==ret)
//	{
//		printf("fail\n");
//		return -1;
//	}else{
//		printf("sucess\n");
//	}
//	return 0;
//}
