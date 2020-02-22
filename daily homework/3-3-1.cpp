/*
 * author:lizitong ID:2017202121
 * time:2019.10.
 * 
 */
 
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

#define MAX_LENGTH 200 //允许的最大字符串长度
#define MAX_TABLE_AMOUNT 10 // 最多允许的表格数 
#define MAX_TABLE_NAME_LENGTH 30 // 表格名称的最大长度
#define MAX_ATTRIBUTE_AMOUNT 50 // 最多允许的属性数
#define MAX_ATTRIBUTE_AMOUNT_PER_TABLE 5 // 每个表最多有5个属性 
#define MAX_ATTRIBUTE_NAME_LENGTH 30 // 属性名称的最大长度
#define MAX_CLAUSE_AMOUNT 20 // 最多允许的关系表达式数 
#define MAX_CLAUSES_LENGTH 40 // 最大关系表达式长度 

typedef struct
{
	char * attrname;
	int tableno;
	int attrno;
	int outputflag = 0;
}Attrs;  // 属性结构体 

typedef struct
{
	char * tablename;
	int tableno;
}Tables;  // 表格结构体 

typedef struct
{
	char * content;
	int tableno;
	int clauseno;
}Clauses;  // 条件语句结构体 

//计数器 
int table_counter = 0;
int attribute_counter = 0;
int clause_counter = 0;

//固定短语 
char * select = "select";
char * from = "from";
char * where = "where";
char * in = "in";
char * _and = "and";
char * left_bracket = "(" ;
char * right_bracket = ")";

Attrs attrlist[MAX_ATTRIBUTE_AMOUNT];
Tables tablelist[MAX_TABLE_AMOUNT];
Clauses clauselist[MAX_CLAUSE_AMOUNT];

int PreTreatment( char * str ) //预处理：大小写转换与简单括号数目检查 
{
	char * cursor = str;
	while( cursor = strstr(str,"SELECT")){
		cursor[0]='s';
		cursor[1]='e';
		cursor[2]='l';
		cursor[3]='e';
		cursor[4]='c';
		cursor[5]='t';
	}
	
	while( cursor = strstr(str,"FROM")){
		cursor[0]='f';
		cursor[1]='r';
		cursor[2]='o';
		cursor[3]='m';
	}
	
	while( cursor = strstr(str,"WHERE")){
		cursor[0]='w';
		cursor[1]='h';
		cursor[2]='e';
		cursor[3]='r';
		cursor[4]='e';
	}
	
	while( cursor = strstr(str,"AND")){
		cursor[0]='a';
		cursor[1]='n';
		cursor[2]='d';
	}
	
	while( cursor = strstr(str,"IN")){
		cursor[0]='i';
		cursor[1]='n';
	}
	
	cursor = str;
	int left=0;
	int right=0;
	
	while( *cursor ){
		if( * cursor == '(') left++;
		if( * cursor == ')') right++;
		cursor++;
	}
	
	if( left!=right ){
		printf( "brackets match error.\n" );
		return 1;
	}
	
	return 0;
}

void printattr()
{
	int i = 0;
	while( i < attribute_counter ){
		printf("Attrsname:%s atteno:%d tableno:%d outputflag:%d \n",
		attrlist[i].attrname,
		attrlist[i].attrno,
		attrlist[i].tableno,
		attrlist[i].outputflag);
		i++;
		
	} 
} 

void printtable()
{
	int i = 0;
	while( i < table_counter ){
		printf("Tables:%s tableno:%d \n",
		tablelist[i].tablename,
		tablelist[i].tableno);
		i++;
	} 
} 

void printclause()
{
	int i = 0;
	while( i < clause_counter ){
		printf("clausename:%s tableno:%d\n",
		clauselist[i].content,
		clauselist[i].tableno);
		i++;
	} 
} 

// 将属性插入属性列表 
void InsertAttrs( char * attr, int flag )
{
	attrlist[attribute_counter].attrname = attr;
	attrlist[attribute_counter].attrno = attribute_counter;
	attrlist[attribute_counter].tableno = table_counter-1;
	attrlist[attribute_counter].outputflag = flag;
	
	/*printf("Attrsname:%s atteno:%d tableno:%d outputflag:%d \n",
		attrlist[attribute_counter].attrname,
		attrlist[attribute_counter].attrno,
		attrlist[attribute_counter].tableno,
		attrlist[attribute_counter].outputflag);*/
	attribute_counter ++;

}

// 插入表格列表 
void InsertTables( char * table )
{
//	table_counter ++ 应放到主parser体中，因为一次parser处理一次语句 
	tablelist[table_counter].tablename = table;
	tablelist[table_counter].tableno = table_counter;
}

//插入关系表达式列表 
void InsertClauses( char * linecut )
{
	clauselist[clause_counter].clauseno = clause_counter;
	clauselist[clause_counter].content = linecut;
	clauselist[clause_counter].tableno = table_counter;
	clause_counter ++;
}

int FindStr( char *str, char *substr) //返回子串的第一个字符所在位置 
{
	int mainstrlen = strlen(str);  // 主字符串长度 
	int substrlen = strlen(substr);  // 子字符串长度 
	int i=0;
	
	char * cursor = str;
	char * tmp = (char *)malloc(sizeof(char)*(substrlen+2));
	
	while(cursor!=NULL && i != mainstrlen ){
		if(*cursor == substr[0]){
			strncpy(tmp,cursor,substrlen);
			tmp[substrlen] = '\0';  // 截断字符串 
			if(strcmp(tmp, substr)==0){
				return i;
			}
			memset(tmp,'\0',sizeof(char)*(substrlen+2));
		}
		cursor ++;
		i ++;
	}
	
	if(i){
		return -1;
	}
 } 

// 获取line字符串中，head字符串与tail字符串之间的字符串 
char * GetStrBetween( char * head, char * tail, char * line)
{
	int headlen = strlen(head);
	int taillen = strlen(tail);
	
	int headpos = FindStr(line,head);
	int tailpos = FindStr(line,tail);
	
	// 若找不到相关字符串位置，则返回-1 
	if( headpos == -1 ){
		return NULL;
	}
	else if( tailpos == -1){
		return NULL ;
	}
	
	int strbetlen = tailpos - headpos - headlen - 2; //要提取的子串长度 
	
	char * strbet = (char *)malloc(sizeof(char)*(strbetlen+1));
	*(strbet+strbetlen) = '\0';
	
	strncpy(strbet, line+headpos+headlen+1, strbetlen);
	return strbet; 
}

void Parser( char * line, int flag ) 
//此处输入的line形式应为“select A from B where C in”
//从line中提取出属性名、表格名、条件语句，插入有关列表 
{	
	char * line2 = (char * )malloc(sizeof(char)*MAX_LENGTH);
	strcpy(line2,line);
	
	char * tablename1 = GetStrBetween(from, where, line2);
	InsertTables(tablename1);

	table_counter++;

	char * attrname1 =  GetStrBetween(select, from, line2);
	InsertAttrs(attrname1, flag);
	
	
	char * linetmp = GetStrBetween(where, _and , line2);
	
	if( linetmp == NULL ){ // where 后无 and 
		linetmp = GetStrBetween(where, in , line2); 
		if( linetmp == NULL ){ // where 后无 in，说明是在最后一个括号内 
			linetmp = strstr(line2,where);
			linetmp += 6;
			InsertClauses(linetmp);
		}
	}
	else{
		
		InsertClauses(linetmp);
		
		char * p = strstr(line2,_and);
		char * q = p;
		
		while( p!= NULL ){
			q = q+4;
			p = strstr(q,_and);
			
			if( p == NULL )//没有and了，说明是最后
			{
				p = strstr(q,in);
				*(p-1) = '\0';
				InsertAttrs(q,0);
				break;
			 }
			else{
				*(p-1) = '\0';
				InsertClauses(q);
			}
			
			q = p;
		}
	}
	
	return;
}

int DivideLine( char * line )
// 用循环的形式分割括号
{
	//拷贝操作 
	char * line2 = (char *)malloc(sizeof(char)*MAX_LENGTH);
	strcpy(line2,line);
	
	int p = FindStr(line,left_bracket);
	int q = p;
	
	line2[q-1] = '\0';
	Parser(line2,1);
	
	while( p != -1 )
	{
		q += 2;
		p = FindStr(line2+q,left_bracket);
		
		if( p != -1) {
			char * tmpline = (char *)malloc(sizeof(char)*MAX_LENGTH);
			strcpy(tmpline,line2+q);
			tmpline[p-1] = '\0';
			free(line2);
			line2 = tmpline; // 修改字符串 
		}
		else{ //已经没有左括号了 
			p = FindStr(line2+q,right_bracket);
			
			char * tmpline = (char *)malloc(sizeof(char)*MAX_LENGTH);
			strcpy(tmpline,line2+q);
			tmpline[p-1] = '\0';
			free(line2);
			line2 = tmpline;
			
			Parser(line2,0);
			break;
		}
		Parser(line2,0); 
		q = p;
	}
	
	return 0;
}

// 组成非嵌套化语句并输出 
void ComposeStr()
{
	/*
	printattr();
	printf("table\n");
	printtable();
	printclause();
	*/
	
	printf("select ");
	int i = 0;
	int j = 0;
	
	while( attrlist[i].outputflag == 1 ){
		printf("%s",attrlist[i].attrname);
		if( attrlist[i+1].outputflag == 0 ){
			printf(" "); 
		}
		else{
			printf(",");
		}
		i++;
		j++;
	}
	
	printf("from ");
	i = 0;
	while( i<table_counter ){
		printf("%s ",tablelist[i].tablename);
		i++;
		if( i == table_counter){
			break;
		} 
		else{
			printf(",");
		}
	}
	
	printf("where ");
	
	i=j;
	
	// 输出表格连接语句 
	while( i<attribute_counter ){
		printf("%s.%s = %s.%s ",
		tablelist[attrlist[i].tableno].tablename,attrlist[i].attrname,
		tablelist[attrlist[i+1].tableno].tablename,attrlist[i+1].attrname);
		i+=2;
		if( i > attribute_counter){
			break;
		} 
		else{
			printf("and ");
		}
	}
	
	if(clause_counter>0){
		//printf("and ");
		i = 0;
		while( i < clause_counter ){
			printf("%s ",clauselist[i].content);
			i++;
			
			if( i+1 > clause_counter){
				printf(";\n");
				break;
			}
			else{
				printf("and ");
			}
		}
	}
	
	return;
}

//重置全局变量 
void ClearList()
{
	table_counter = 0;
	attribute_counter = 0;
	clause_counter = 0;
	
	memset(attrlist,'\0',sizeof(Attrs)*MAX_ATTRIBUTE_AMOUNT);
	memset(tablelist,'\0',sizeof(Tables)*MAX_TABLE_AMOUNT);
	memset(clauselist,'\0',sizeof(Clauses)*MAX_CLAUSE_AMOUNT);
}

int main()
{
/*  main */
	
	printf("SQL LANGUAGE MODIFICATION (ID:2017202121) \n");
	printf("If you want to exit, please enter '#' \n");
	
	int i = 1;
	
	while(i){
		printf("please input SQL language: \n");
		char * line1 = (char *)malloc(sizeof(char)*MAX_LENGTH);
		gets(line1);
		
		if( !strcmp(line1,"#") ){
			printf("EXIT\n");
			return 0;
		}
		
		if(PreTreatment(line1)){
			continue;
		}
		else{
			DivideLine(line1);
			ComposeStr();
		}
		ClearList();
		printf("\n");
	}
	
	//test 1: select A1 from R1 where A1 > 10 and B1 in ( select B2 from R2 where A2 < 100 and B2 in ( select B3 from R3 where A3 % 9 <> 7 ))
	//test 2: SELECT A1 FROM R1 where A1 > 10 and B1 in ( select B2 FROM R2 where A2 < 100 ); 
	//test 3: SELECT A1 FROM R1 where A1 > 10 and B1 in ( select B2 FROM R2 where A2 < 100 )); 		
	//test 4: SELECT A1,A4 FROM R1 where A1 > 10 and B1 in ( select B2 FROM R2 where A2 < 100 );  
 } 

/* ----- BUG RECORD ---- */ 

/*  #1 error: stray '\241' in program : 程序中有非法（中文）字符。 
	#2 循环中，使用完一次需重置所有全局变量。 
	#3 对字符串进行处理时，建议把原来字符串拷贝之后再在新串进行
	操作，避免不必要的字符读取错误。 
*/

/* ----- FOR TEST ------ */
 
 /*  Function:FindStr Test 
	char * test1 = "select sname from student where sno in ";
	char * test2 = "wheer";
	int i = FindStr( test1, test2);
	
	printf("FindStr : %d\n",i);
*/

/*  Function:GetStrBetween( char * head, char * tail, char * line) Test 
	char * line1 = "( select sno from sc where in ) ";
	char * head1 = "select";
	char * tail1 = "in";
	char * ans1 = GetStrBetween( select, in, line1);
	
	printf("GetStrBetween:%s\n",ans1);
*/

/*  Function:Parser( char * line ) Test 
	char * line1 = "select sname from student where sno in ";
	Parser(line1);
	
	printf("Tables:%s\n Attrs:%s\n tableno:%d\n atteno:%d\n",
		tablelist[table_counter].tablename,
		attrlist[attribute_counter].attrname,
		table_counter,
		attribute_counter);
*/

 
