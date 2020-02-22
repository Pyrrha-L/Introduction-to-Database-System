#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LENGTH 100
#define MAX_NAME_LEN 40
#define MAX_CLAUSE_LEN 40
#define VIEW_ATTR_NUM 5

typedef struct
{
	char *viewname;
	char *tablename;
	char *attrname[4];
	char *clausedefault;
 }View;

typedef struct
{
	char * attrname;
	int attrno;
}Attrs;  // 属性结构体

typedef struct
{
	char * content;
	int clauseno;
}Clauses;  // 条件语句结构体 

//固定短语 
char * select = "select";
char * from = "from";
char * where = "where";
char * in = "in";
char * _and = "and";
char * left_bracket = "(" ;
char * right_bracket = ")";

View SC_view;
char nameout[MAX_NAME_LEN] = {'\0'};
char clauseout[MAX_CLAUSE_LEN] = {'\0'};
char allout[MAX_LENGTH] = {'\0'};

//对内置视图进行初始化 
void InitView()
{
	SC_view.viewname = "Student_Course";
	SC_view.tablename = "Students s, SC e, Courses c";
	SC_view.clausedefault = "s.Sno = e.Sno and e.Course_no = c.Course_no";
	SC_view.attrname[0] = "s.Sno";
	SC_view.attrname[1] = "s.Name";
	SC_view.attrname[2] = "c.Course_no";
	SC_view.attrname[3] = "Title";
	
	memset(nameout,0,MAX_NAME_LEN);
	memset(clauseout,0,MAX_CLAUSE_LEN);
	memset(allout,0,MAX_LENGTH);
	
	/*
	int i = 0;
	while(i<4){
		printf("62:%s ",SC_view.attrname[i]);
		i++;
	}
	*/
	
	return;
}

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
		printf( "brackets match error.\n\n" );
		return 1;
	}
	
	return 0;
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

// 检查输入的属性是否在内置视图中 
int Checkattr( char * name ){
	char * name2 = (char *)malloc(sizeof(char)*MAX_NAME_LEN);
	strcpy(name2,name);
	int i = 0;
	int flag = 0;
	
	if(!strcmp(name2,"*")){
		strcpy(nameout,"s.Sno, s.Name, c.Course_no, Title ");
		return flag;
	}
	
	char * fullname = (char *)malloc(sizeof(char)*MAX_NAME_LEN);
	char * rawname = (char *)malloc(sizeof(char)*MAX_NAME_LEN);
	char * p = strchr(name2,',');
	char * q = name2;
	
	if( p==NULL ){
		while( SC_view.attrname[i] ){
			rawname = strchr(SC_view.attrname[i],'.')+1;
			strcpy(fullname, SC_view.attrname[i]);
			if( !strcmp(fullname,name2) || 
			!(strcmp(rawname,name2))){
				strcpy(nameout,SC_view.attrname[i]);
				strcat(nameout," ");
				return flag;
			}
			i++;
		}
		
		if( i >=4 ){
			flag = 1;
		}
	}
	else{
		// 更新截断指针 
		if(p) {
			*p = '\0';
			p+=2; // 各个属性名之间应有一空格连接 
		}
		while( q ){
			
			i=0;
			while( i<4 ){
				if(strchr(SC_view.attrname[i],'.')){
					rawname = strchr(SC_view.attrname[i],'.')+1;
				}
				else{
					rawname = SC_view.attrname[i];
				}
				strcpy(fullname, SC_view.attrname[i]);
				
				if( !strcmp(fullname,q) || 
				!(strcmp(rawname,q))){
					if( !nameout ){
						strcpy(nameout,SC_view.attrname[i]);
					}else{
						strcat(nameout,SC_view.attrname[i]);
					}
					//strcat(nameout," ");
					break;
				}
				i++;
			}
			
			if( i >= 4 ){
				flag = 1;
				break;
			}
			q = p;
			if(q){
				strcat(nameout,",");
				p = strchr(q, ',');
			} 
		}
		
	}
	
	strcat(nameout," ");
	
	if(flag){
		printf("Illegal attributes input.\n\n");
	}

	return flag;
}

// 检查输入的表格名称是否是内置视图名称 
int Checktable( char * name ){
	if( strcmp(name, SC_view.viewname) ){
		return 1;
	}
	return 0;
}

int Parser(char * line)
{
	
	char * line2 = (char * )malloc(sizeof(char)*MAX_LENGTH);
	strcpy(line2,line);
	
	char * tablename1 = GetStrBetween(from, where, line2); 
	if( Checktable( tablename1 )){
		printf(" The View Is Not Created.\n\n");
		return 1;
	}

	char * attrname1 =  GetStrBetween(select, from, line2);
	if( Checkattr( attrname1 )){
		return 1;
	}
	
	char * clause1 = strstr(line2,"where ");
	strcpy(clauseout,"s.Sno = e.Sno and e.Course_no = c.Course_no ");
	
	if( clause1 ){
		clause1 += 6;
		
		strcat( clauseout, "and " );
		strcat( clauseout, clause1 );
	}
	
	return 0;
}

void ComposeStr()
{
	strcat(allout,"selest ");
	strcat(allout,nameout);
	strcat(allout,"from ");
	strcat(allout,SC_view.tablename);
	strcat(allout," where ");
	strcat(allout,clauseout);
	strcat(allout," ;");
	
	printf("TRANSFORMED: %s\n\n",allout);
	return;
}

int main()
{
	/* ----- TEST SAMPLE ----- */
	/* #1 select * from Student_Course where Sno = X
	 * #2 SELECT s.Sno, s.Name Student_Course where Sno = X
	 * #3 select s.Sno from Student_Course where Sno = X
	 * #4 select s.Sno, s.Name from Stu_Cou where Sno = X
	 * #5 select Sno, Name from Student_Course where Sno = X
	 * #6 select S.Serror, S.Name from Student_Course where Sno = X
	 */
	 
	printf(" SQL TRANSFORMATION BY 2017202121 \n");
	printf(" enter '#' to exit \n\n");
	
	int k = 1;
	char * line = (char *)malloc(sizeof(char)*MAX_LENGTH);
	
	while(k){
		InitView();
		printf("Please enter SQL request:\n");
		gets(line);
		
		if( strcmp(line,"#") ){
			PreTreatment(line);
			if(Parser(line)){
				continue;
			}
			ComposeStr();
		}
		else{
			k = 0;
		}
	}
	return 0;
}
