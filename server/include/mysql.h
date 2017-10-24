#ifndef __MYSQL_H__
#define __MYSQL_H__
#include "head.h"

void get_rand(char*,int);

int mysql_isuser(char*);

int mysql_insert(char*,char*,char*);

int mysql_select(char*,int,char*);

int mysql_updata(int,char*);

#endif
