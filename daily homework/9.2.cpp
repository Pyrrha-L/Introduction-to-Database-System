/*
 * author: li zitong ID: 2017202121
 * time: 2019.10.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_INPUT_LENGTH 150 // 允许输入的最大字符串长度
#define MAX_DIM 3 // i<=3
#define MAX_C 10 // 条件语句的最大数目 
#define MAX_C_LENGTH 30  // 条件语句的最大长度
#define MAX_TABLE_NAME 15 
#define ATTR_NAME_LENGTH 15
#define MAX_JOIN_NUM 10
#define JOIN_PARA 4
#define fi 50 //树形结构索引的内部结点的平均可放50个元组 
#define NODE 20
#define JOINCACHE 1 // 假设连接操作中，缓冲区只能容纳每个关系的一个块

/* glabal variables */
int relation_num; // 语句中出现的关系数 
int clause_num; // 语句中出现的条件数 
int join_num; // 语句中的连接数
int select_num; // 语句中的选择数 
int nR[MAX_DIM];
int fR[MAX_DIM];
int bR[MAX_DIM];
int MR[MAX_DIM];
int SelectionSwitch[MAX_DIM]; // 确定哪些表格用到了选择 
int JoinPara[MAX_JOIN_NUM][JOIN_PARA];
int attribute_num[MAX_DIM]; // 各个表格的属性数 
int DistR[MAX_DIM][MAX_DIM];
int MinR[MAX_DIM][MAX_DIM];
int MaxR[MAX_DIM][MAX_DIM];
int LowR[MAX_DIM][MAX_DIM][2]; // [i][j][0]: raw number [i][j][1]: for '=' 
int HighR[MAX_DIM][MAX_DIM][2];
int IndexR[MAX_DIM][MAX_DIM]; // 1:primary index; 2:secondary index; 3: no index, table scan; 4: binary search 
int HTR[MAX_DIM][MAX_DIM]; // 各属性的索引高度 
int LBR[MAX_DIM][MAX_DIM];
double SFR[MAX_DIM][MAX_DIM]; // 各属性的选择率 

char line[MAX_INPUT_LENGTH];
char clause[MAX_C][MAX_C_LENGTH];
char attrname[MAX_DIM][MAX_DIM][ATTR_NAME_LENGTH];
char tablename[MAX_DIM][MAX_TABLE_NAME];

// sql原生词 
char * select = "select";
char * from = "from";
char * where = "where";
char * in = "in";
char * _and = "and";
char * left_bracket = "(" ;
char * right_bracket = ")";

//初始化函数 
int Init()
{
	relation_num = 1;
	clause_num = 1;
	join_num = 0;
	
	int i,j;

	for( i=0;i<MAX_DIM;i++ ){
		nR[i] = 0;
		fR[i] = 0;
		bR[i] = 0;
		MR[i] = 0;
		SelectionSwitch[i] = 0;
		attribute_num[i] = 0; 
	}

	for( i=0;i<MAX_DIM;i++ ){
		for( j=0;j<MAX_DIM;j++ ){
			DistR[i][j] = 0;
			IndexR[i][j] = 0;
			HTR[i][j] = 0;
			LBR[i][j] = 0;
			SFR[i][j] = 0.0;
			LowR[i][j][0]=-1;
			LowR[i][j][1]=-1;
			HighR[i][j][0]=-1;
			HighR[i][j][1]=-1;
		}
	}
	
	memset(JoinPara,-1,sizeof(int)*MAX_JOIN_NUM*JOIN_PARA);
	memset(line,'\0',sizeof(char)*MAX_INPUT_LENGTH);
	memset(attrname,'\0',sizeof(char)*MAX_DIM*MAX_DIM*ATTR_NAME_LENGTH);
	return 0;
}

//返回子串的第一个字符所在位置 
int FindStr( char *str, char *substr) 
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

// 根据表格名字返回其编号（编号根据输入sql语句中表格排序确定） 
int FindTableByName( char * name ){
	
	char * table2 = (char *)malloc(sizeof(char)*MAX_TABLE_NAME); 
	strcpy(table2,name);
	
	if( !strcmp(name,"Default") ){
		return 0;
	}
	
	int i;
	for( i=0;i<MAX_DIM;i++ )
	{
		if( !strcmp(tablename[i],name) ){
			return i;
		}
	}
	
	printf("ERROR: Table %s Not Found.\n",table2);
	exit(0);
}

// i 为对应的表格编号，name为欲插入的属性名，将名为name的属性插入相应编号的表格 
int InsertAttrname(int i, char * name)
{	
	int j ;
	for( j=0;j<MAX_DIM;j++ )
	{
		if(strcmp(attrname[i][j],name)==0){ // 如该属性已存在 
			return j;
		}
		if(strlen(attrname[i][j]) == 0){
			attribute_num[i]++;
			strcpy(attrname[i][j],name);
			return j;
		}
	}
	return -1;
}

//把num的值根据相应的flag赋给名字为name的属性的范围值 
int InsertAttrrange(char * name, int num, int flag)
{
	int i,j;
	int flag2=0;
	for( i=0;i<MAX_DIM;i++ ){
		for( j=0;j<MAX_DIM;j++ ){
			if(strcmp(name,attrname[i][j])==0){
				flag2=1;
				break;
			}
		}
		if(flag2){
			break;
		}
	}

	if(flag == 1){
		LowR[i][j][0]=num;
		LowR[i][j][1]=1;
		return 0;
	}
	else if(flag == 2){
		HighR[i][j][0]=num;
		HighR[i][j][1]=1;
		return 0;
	}
	else if(flag == 3){
		LowR[i][j][0]=num;
		LowR[i][j][1]=0;
		return 0;
	}
	else if(flag == 4){
		HighR[i][j][0]=num;
		HighR[i][j][1]=0;
		return 0;
	}
	else if(flag == 5){
		LowR[i][j][0]=num;
		LowR[i][j][1]=1;
		HighR[i][j][0]=num;
		HighR[i][j][1]=1;
		return 0;
	}
	
	return 1;
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

// 计算索引树深度 
int Cal_HT( )
{
	int i,j;
	for( i=0;i<MAX_DIM;i++ ){
		for( j=0;j<MAX_DIM;j++ ){
			if(DistR[i][j]) HTR[i][j]=ceil(log(DistR[i][j])/log(NODE));
		}
	}
	
/*	printf("The HT Matrix is :\n");
	for( i=0;i<MAX_DIM;i++ ){
		for( j=0;j<MAX_DIM;j++ ){
			printf("%d ",HTR[i][j]);
			if( j == MAX_DIM-1 ){
				printf("\n");
			}
		}
	}
	printf("\n");
*/	return 0;
}

// 对连接语句进行拆分, clause为连接语句如“R1.A1=R2.A2” 
int ParseJoin( char * clause )
{
	char * clause2 = (char *)malloc(sizeof(char)*MAX_C_LENGTH);
	strcpy(clause2, clause);
	
	char * attrname1 = (char *)malloc(sizeof(char)*ATTR_NAME_LENGTH);
	char * attrname2 = (char *)malloc(sizeof(char)*ATTR_NAME_LENGTH);
	char * tablename1 = (char *)malloc(sizeof(char)*MAX_TABLE_NAME);
	char * tablename2 = (char *)malloc(sizeof(char)*MAX_TABLE_NAME);
	
	char * cursor;
	char * cursor2;
	
	cursor2 = strstr(clause2,"=");
	cursor = strstr(clause2,".");
	
	//提取左表格名
	strncpy(tablename1,clause2,cursor-clause2);
	tablename1[cursor-clause2]='\0';
	 
	cursor++;
	
	//提取左属性名 
	strncpy(attrname1,cursor,cursor2-cursor);
	attrname1[cursor2-cursor]='\0';
	cursor2++;
	
	//提取右表格名
	cursor = strstr(cursor,".");
	strncpy(tablename2,cursor2,cursor-cursor2);
	tablename2[cursor-cursor2]='\0';
	
	//提取右属性名 
	cursor++;
	strcpy(attrname2,cursor);
	
	int i,m,n;
	
	m = InsertAttrname(FindTableByName(tablename1),attrname1);
	n = InsertAttrname(FindTableByName(tablename2),attrname2);

	JoinPara[join_num][0]=FindTableByName(tablename1);
	JoinPara[join_num][1]=m;
	JoinPara[join_num][2]=FindTableByName(tablename2);
	JoinPara[join_num][3]=n;
	join_num++;
	
	return 0;
}

// 对一般条件语句进行拆分，clause为形如“A1>10”的语句 
int ParseClause( char *clause )
{
	char * clause2 = (char *)malloc(sizeof(char)*MAX_C_LENGTH);
	char * attrname2 = (char *)malloc(sizeof(char)*ATTR_NAME_LENGTH);
	char * tablename = (char *)malloc(sizeof(char)*MAX_TABLE_NAME);
	strcpy(clause2, clause);
	
	char * cursor;
	char * cursor2;
	
	// 提取表格名 
	if((cursor2 = strstr(clause2,"."))){
		strncpy(tablename,clause2,cursor2-clause2);
		tablename[cursor2-clause2] = '\0';
		cursor2++;
	}else{
		tablename = "Default"; // 设置默认表格名 
	}
	
	// 对不同符号进行相似处理 
	if((cursor = strstr(clause2,">="))){
		cursor+=2;
		int tmp = atoi(cursor);
		
		if(cursor2) {
			strncpy(attrname2,cursor2,cursor-cursor2-2);
			attrname2[cursor-cursor2-2]='\0';
		}
		else {
			strncpy(attrname2,clause2,cursor-clause2-2);
			attrname2[cursor-clause2-2]='\0';
		}	
		
		SelectionSwitch[FindTableByName(tablename)]++;
		InsertAttrname(FindTableByName(tablename),attrname2);
		InsertAttrrange(attrname2, tmp, 1);
	}
	else if((cursor = strstr(clause2,"<="))){
		cursor+=2;
		int tmp = atoi(cursor);
		
		if(cursor2) {
			strncpy(attrname2,cursor2,cursor-cursor2-2);
			attrname2[cursor-cursor2-2]='\0';
		}
		else {
			strncpy(attrname2,clause2,cursor-clause2-2);
			attrname2[cursor-clause2-2]='\0';
		}
		
		SelectionSwitch[FindTableByName(tablename)]++;
		InsertAttrname(FindTableByName(tablename),attrname2);
		InsertAttrrange(attrname2, tmp, 2);
	}
	else if((cursor = strstr(clause2,">"))){
		cursor+=1;
		int tmp = atoi(cursor);
		
			if(cursor2) {
				strncpy(attrname2,cursor2,cursor-cursor2-1);
				attrname2[cursor-cursor2-1]='\0';
			}
			else {
				strncpy(attrname2,clause2,cursor-clause2-1);
				attrname2[cursor-clause2-1]='\0';
			}
		
		SelectionSwitch[FindTableByName(tablename)]++;
		InsertAttrname(FindTableByName(tablename),attrname2);
		InsertAttrrange(attrname2, tmp, 3);
	}
	else if((cursor = strstr(clause2,"<"))){
		cursor+=1;
		int tmp = atoi(cursor);
		
		if(cursor2) {
			strncpy(attrname2,cursor2,cursor-cursor2-1);
			attrname2[cursor-cursor2-1]='\0';
		}
		else {
			strncpy(attrname2,clause2,cursor-clause2-1);
			attrname2[cursor-clause2-1]='\0';
		}
		
		SelectionSwitch[FindTableByName(tablename)]++;
		InsertAttrname(FindTableByName(tablename),attrname2);
		InsertAttrrange(attrname2, tmp, 4);
	}
	else if((cursor = strstr(clause2,"="))){
		cursor+=1;
		if( *cursor >='0' && *cursor <='9' ){
			int tmp = atoi(cursor);
			
			if(cursor2) {
				strncpy(attrname2,cursor2,cursor-cursor2-1);
				attrname2[cursor-cursor2-1]='\0';
			}
			else {
				strncpy(attrname2,clause2,cursor-clause2-1);
				attrname2[cursor-clause2-1]='\0';
			}
			
			SelectionSwitch[FindTableByName(tablename)]++;
			InsertAttrname(FindTableByName(tablename),attrname2);
			InsertAttrrange(attrname2, tmp, 5);
		}
		else{
			free(attrname2);
			ParseJoin( clause );
		}
	}
	
	return 0;
}

// 测试用打印输出 
void printtest()
{
	int i,j; 
	
	printf("The LowR Matrix is :\n");
		for( i=0;i<MAX_DIM;i++ ){
			for( j=0;j<MAX_DIM;j++ ){
				printf("%d ",LowR[i][j][0]);
				if( j == MAX_DIM-1 ){
					printf("\n");
				}
			}
		}
	
	printf("The HighR Matrix is :\n");
		for( i=0;i<MAX_DIM;i++ ){
			for( j=0;j<MAX_DIM;j++ ){
				printf("%d ",HighR[i][j][0]);
				if( j == MAX_DIM-1 ){
					printf("\n");
				}
			}
		}
		
	printf("The DistR Matrix is :\n");
		for( i=0;i<MAX_DIM;i++ ){
			for( j=0;j<MAX_DIM;j++ ){
				printf("%d ",DistR[i][j]);
				if( j == MAX_DIM-1 ){
					printf("\n");
				}
			}
		}
	
	printf("The HTR Matrix is :\n");
		for( i=0;i<MAX_DIM;i++ ){
			for( j=0;j<MAX_DIM;j++ ){
				printf("%d ",HTR[i][j]);
				if( j == MAX_DIM-1 ){
					printf("\n");
				}
			}
		}
}

// 计算选择率 
int Cal_SF( )
{	
	double addval=0;
	int i,j; 
	
	for ( i=0;i<MAX_DIM;i++ ){
		for ( j=0;j<MAX_DIM;j++ ){
			if( DistR[i][j] ){
				addval = 1.0/DistR[i][j];
			}
			else{
				continue;
			}
			
			if(LowR[i][j][0]!=-1 ){
				if( HighR[i][j][0]!=-1 ){
					if( LowR[i][j][0]==HighR[i][j][0] ){
						SFR[i][j] = addval;
						continue;
					}
					SFR[i][j] = (HighR[i][j][0]-LowR[i][j][0])*1.0/(MaxR[i][j]-MinR[i][j]);
					if((HighR[i][j][1]+LowR[i][j][1])>0){
						SFR[i][j]+=(HighR[i][j][1]+LowR[i][j][1])*addval;
					}
					continue;
				}
				SFR[i][j] = (MaxR[i][j]-LowR[i][j][0])*1.0/(MaxR[i][j]-MinR[i][j]);
				if(LowR[i][j][1]>0){
					SFR[i][j]+=addval;
				}
				continue;
			}
			else if( HighR[i][j][0]!=-1 ){
				SFR[i][j] = (HighR[i][j][0]-MinR[i][j])*1.0/(MaxR[i][j]-MinR[i][j]);
				if(LowR[i][j][1]>0){
					SFR[i][j]+=addval;
				}
				continue;
			}
		} 
	} 
/*	
	printf("The SF Matrix is :\n");
	for( i=0;i<MAX_DIM;i++ ){
		for( j=0;j<MAX_DIM;j++ ){
			printf("%.5lf",SFR[i][j]);
			if( j == MAX_DIM-1 ){
				printf("\n");
			}
		}
	}
	printf("\n");
*/	return 0;
}

// 计算最底层叶子数 
int Cal_LB( )
{
	int i,j;
	
	for( i=0;i<MAX_DIM;i++ ){
		for( j=0;j<MAX_DIM;j++ ){
			if( DistR[i][j] ){
				// 假设索引节点可存放20个指针对,每个结点至少半满。 
				LBR[i][j] = DistR[i][j] / (NODE/2) ; 
			}
		}
	}
	
/*	printf("The LB Matrix is :\n");
	for( i=0;i<MAX_DIM;i++ ){
		for( j=0;j<MAX_DIM;j++ ){
			printf("%d ",LBR[i][j]);
			if( j == MAX_DIM-1 ){
				printf("\n");
			}
		}
	}
	printf("\n");
*/
	return 0;
}

// 拆分函数，line为输入sql语句 
int Parser( char * line )
{
	char * line1 = (char *)malloc(sizeof(char)*MAX_INPUT_LENGTH);
	strcpy(line1,line);
	
	char * relationall = GetStrBetween(from,where,line1);
	char comma = ',';
	int p = 0;
	
	int i = 0;
	while( relationall[i] ){
		if( relationall[i]==comma ){
			strncpy(tablename[relation_num-1],relationall+p,i-p);
			tablename[relation_num-1][i-p]='\0';
			relation_num++;
			i++;
			p=i;
		}
		i++;
	}
	strcpy(tablename[relation_num-1],relationall+p);
	tablename[relation_num-1][strlen(relationall)-p]='\0';
	
//	for( i=0;i<3;i++ ){
//		printf("570:%s\n",tablename[i]);
//	}
	
	char * linetmp = GetStrBetween(where, _and , line1);
	
	if( linetmp == NULL ){ // where 后无 and 
		linetmp = strstr(line1,where);
		linetmp += 6;
		ParseClause(linetmp);
	}
	else{
		
		ParseClause(linetmp);
		
		char * p = strstr(line1,_and);
		char * q = p;
		
		while( p!= NULL ){
			q = q+4;
			p = strstr(q,_and);
			
			if( p == NULL )//没有and了，说明是最后
			{
				ParseClause(q);
				break;
			 }
			else{
				*(p-1) = '\0';
				ParseClause(q);
			}
			
			q = p;
		}
	}
	
	return relation_num;
}

/* EvaluateSelection函数组，均根据ppt顺序 */ 
int SelectA1(int i, int j)
{
	int ret = bR[i];
	//printf("A1:全表扫描 E=bR=%d\n",bR[i]);
	return ret;
}

int SelectA2(int i, int j)
{
	int ret =  bR[i] + ceil(log(bR[i])/log(2)) + ceil(nR[i]*SFR[i][j]/fR[i]) -1 ;
	//printf("A2:折半扫描 E=bR + log2(bR)+SC(A,r)/fR-1=%d\n",ret);
	return ret;
}

int SelectA4(int i, int j)
{
	int ret = ceil(HTR[i][j]) + ceil(nR[i]*SFR[i][j]/fi);
	//printf("A4:利用主索引等值比较 E=HTi+SC(A,r)/fr=%d\n",ret);
	return ret;
}

int SelectA5(int i, int j)
{
	int ret = ceil(HTR[i][j]) + (nR[i]*SFR[i][j]);
	//printf("A5:利用辅助索引等值比较 E=HTi+SC(A,r)=%d\n",ret);
	return ret;
}

int SelectA6(int i, int j)
{
	int ret = HTR[i][j] + bR[i]*SFR[i][j];
	//printf("A6:利用主索引非等值比较 E=HTi+bR*SFi=%d\n",ret);
	return ret;
}

int SelectA7(int i, int j)
{
	int ret = HTR[i][j] + LBR[i][j]*SFR[i][j]+nR[i]*SFR[i][j];
	//printf("A7:利用辅助索引非等值比较 E=HTi+LBi*SFi+nR*SFi=%d\n",ret);
	return ret;
}

// 返回对应attr[relation][i]最适合的查询方式 
int ClassifySelection( int relation , int i )
{
	int index = IndexR[relation][i];
	int cost;
	
	if( index == 1){
		if(DistR[relation][i] > 0 && LowR[relation][i][0] == HighR[relation][i][0]){
			cost = SelectA4(relation, i);
		}
		else if(DistR[relation][i] > 0 && LowR[relation][i][0] != HighR[relation][i][0]){
			cost = SelectA6(relation, i);
		}
	}
	else if( index == 2){
		if(DistR[relation][i] > 0 && LowR[relation][i][0] == HighR[relation][i][0]){
			cost = SelectA5(relation, i);
		}
		else if(DistR[relation][i] > 0 && LowR[relation][i][0] != HighR[relation][i][0]){
			cost = SelectA7(relation, i);
		}
	}
	else if( index == 3){
		cost = SelectA1(relation, i);
	}
	else if( index == 4){
		cost = SelectA2(relation, i);
	}
	else{
		printf("Index Not Found!\n");
		exit(0);
	}

	return cost;
}

// 计算最小的选择代价 
int EvaSelection( int relation )
{
	int attrnum = attribute_num[relation];
	int retindex[3] = {0,0,0};
	int retcost[3] = {0,0,0} ;
	int l,m,n;
	int cost = 0;
	int bias = 0;
	
	for( l=0;l<attrnum;l++ ){
		if( LowR[relation][l][0]==MinR[relation][l] &&
			HighR[relation][l][0]==MaxR[relation][l])
		{
			bias++;
		}
	}
	attrnum -= bias;
	
	// 根据元组中属性的个数分别计算总开销 
	if( attrnum==1 ){
		for( l=1;l<=4;l++ ){
			IndexR[relation][0] = l;
			if(cost == 0 || cost>ClassifySelection(relation,0) ){
				retcost[0] =  ClassifySelection(relation,0);
				cost = ClassifySelection(relation,0);
				retindex[0] = l;
			}
		}
	}
	else if( attrnum == 2){
		for( l=1;l<=4;l++ ){
			IndexR[relation][0] = l;
			for( m=1;m<=4;m++ ){
				IndexR[relation][1] = m;
				
				if( l==1 && m ==1 ){
					continue;
				}
				
				if(cost == 0 || cost>ClassifySelection(relation,0)+ClassifySelection(relation,1) ){
					cost = ClassifySelection(relation,0)+ClassifySelection(relation,1);
					retcost[0]=ClassifySelection(relation,0);
					retcost[1]=ClassifySelection(relation,1);
					
					retindex[0] = l;
					retindex[1] = m;
				}
				
			}
		}
	}
	else if( attrnum == 3 ){
		for( l=1;l<=4;l++ ){
			IndexR[relation][0] = l;
			for( m=1;m<=4;m++ ){
				IndexR[relation][1] = m;
				
				if( l==1 && m==1 ){
					continue;
				}
				
				for( n=1;n<=4;n++ ){
					IndexR[relation][2] = n;
					if( (l==1&&m==1) || (l==1&&n==1) || (m==1&&n==1) ){
						continue;
					}
					
					if(cost == 0 || cost>ClassifySelection(relation,0)+
						ClassifySelection(relation,1)+
						ClassifySelection(relation,2) ){
						cost = ClassifySelection(relation,0)
						+ClassifySelection(relation,1)+
						+ClassifySelection(relation,2);
						retcost[0] = ClassifySelection(relation,0);
						retcost[1] = ClassifySelection(relation,1);
						retcost[2] = ClassifySelection(relation,2);
						retindex[0] = l;
						retindex[1] = m;
						retindex[2] = n;
					}
				}
			}
		}
	}
	
	l=0;
	printf("\nSelection Choice Result\n");
	while( retindex[l] ){
		printf("Attribute %d: ",l+1);
		switch(retindex[l]){
			case 1: printf("Primary Key. Cost: %d\n",retcost[l]);break;
			case 2: printf("Secondary Key. Cost: %d\n",retcost[l]);break;
			case 3: printf("Table Scan. Cost: %d\n",retcost[l]);break;
			case 4: printf("Binary Search. Cost: %d\n",retcost[l]);break;
			default:printf("No suggestion provided\n");
		}		
		l++;
	}
	printf("Total Cost: %d\n",cost);
	printf("END\n");
	
	return cost; 
}

// 块嵌套循环连接 
int JoinB1( int table1, int attr1, int table2, int attr2 )
{
	int ret;
	int out = ( nR[table1]<nR[table2] )? table1:table2;
	int in = ( nR[table1]<nR[table2] )? table2:table1;
	ret = bR[out]*bR[in]+bR[out];
	
	return ret;
}

// 索引循环嵌套连接，计算以table1中attr1为主索引的代价。 
int JoinB2( int table1, int attr1, int table2, int attr2 ) 
{
	int ret;
	int c = ceil(log(nR[table1])/log(NODE))+1;
	ret =  c*nR[table2] + bR[table2];
	return ret;
}

// 不考虑已经排好序的归并排序连接，其中排序代价采用快排空间复杂度，O(nlog(2,n)) 
int JoinB3( int table1, int attr1, int table2, int attr2 )
{
	int ret;
	ret = (bR[table1]+bR[table2])*2+
		bR[table1]*(2*ceil(log(bR[table1]/3)/log(2))+1) +
		bR[table2]*(2*ceil(log(bR[table2]/3)/log(2))+1);
	//ret = (bR[table1]+bR[table2])*2+
	//	bR[table1]*2*(ceil(log(bR[table1]/3))/log(2)+1)+
	//	bR[table2]*2*(ceil(log(bR[table2]/3))/log(2)+1);
	return ret;
}
 
// 散列连接 
int JoinB4( int table1, int attr1, int table2, int attr2 )
{
	int ret = 3*(bR[table1]+bR[table2]);
	return ret;
}

// 根据传入的四个参数，计算代价最小的连接操作 
int JoinSelection( int table1, int attr1, int table2, int attr2 )
{
	int ret = 1;
	int cost = JoinB1(table1, attr1, table2, attr2 );
	
	if( cost > JoinB2(table1, attr1, table2, attr2 )){
		cost = JoinB2(table1, attr1, table2, attr2 );
		ret = 2;
	}
	
	if( cost > JoinB2(table2, attr2, table1, attr1 )){
		cost = JoinB2(table2, attr2, table1, attr1 );
		ret = 3;
	}
	
	if( cost > JoinB3(table1, attr1, table2, attr2 )){
		cost = JoinB3(table1, attr1, table2, attr2 );
		ret = 4;
	}
	
	if( cost > JoinB4(table1, attr1, table2, attr2 )){
		cost = JoinB4(table1, attr1, table2, attr2 );
		ret = 5;
	}
	
	printf("\nJoin Choice Result for Table %d and Table %d \n",table1,table2);
	switch(ret){
		case 1: printf("块嵌套循环连接，总代价为%d\n",cost);break;
		case 2: printf("索引嵌套循环连接，主键为%s，总代价为%d\n",attrname[table1][attr1],cost);break;
		case 3: printf("索引嵌套循环连接，主键为%s，总代价为%d\n",attrname[table2][attr2],cost);break;
		case 4: printf("排序归并连接，总代价为%d\n",cost);break;
		case 5: printf("散列连接，总代价为%d\n",cost);break;
	}
	printf("END\n");
	
	return ret; 
} 

// 读取输入函数 
int Input()
{
	printf("Please enter the complete SQL query:\n");
	gets(line);
	
	int len = strlen(line);
	
	//除去末尾的空格和分号 
	while( line[len-1]==' ' || line[len-1]==';' ){
		len--;
	}
	line[len]='\0';
	
	if(!strcmp(line,"#")){
		printf("EXIT...\n");
		exit(0);
	}
	
	if(!strstr(line,from) || !strstr(line,where)){
		printf("The input SQL language is not valid.\n");
		exit(0);
	}
	
	Parser(line);
	
	printf("\nRelations and attributes are numbered by the input query order, please enter more infomation.\n\n");
	
	int i = 1;
	int j = 1;
	char * buffer = (char *)malloc(sizeof(char));
	
	while( i <= relation_num){
		
		printf("SET RELATION %d details:\n", i);
		printf("please enter 'n f' in order:\n");
		scanf("%d %d",&nR[i-1],&fR[i-1]);
		bR[i-1] = nR[i-1]/fR[i-1];
		
		j=1;
		
		while( j <= attribute_num[i-1] ){
			printf("SET ATTRIBUTE %d details:\n",j);
			
			printf("please enter Dist of this attribute:\n");
			gets(buffer);
			scanf("%d",&DistR[i-1][j-1]);
			
			printf("please enter Lowest and Highest of this attribute:\n");
			scanf("%d %d",&MinR[i-1][j-1],&MaxR[i-1][j-1]);
			
			if(LowR[i-1][j-1][0]==-1) LowR[i-1][j-1][0] = MinR[i-1][j-1];
			if(LowR[i-1][j-1][0] < MinR[i-1][j-1]){
				printf("The Min Value is too big.\n");
				exit(0);
			}
			
			if(HighR[i-1][j-1][0]==-1) HighR[i-1][j-1][0] = MaxR[i-1][j-1];
			if(HighR[i-1][j-1][0] > MaxR[i-1][j-1]){
				printf("The Max Value is too small.\n");
				exit(0);
			}

			j++;
		}
		i++;
	}
	
	Cal_HT();
	Cal_LB();
	Cal_SF();
	
	//printtest();
	
	return 0;
}

// 程序说明 
void Declaration()
{
	printf("----- VERY SIMPLE OPTIMIZATION STRATEGY CALCULATOR -----\n\n");
	printf("------------ SOME SETTINGS FOR YOU TO KNOW -------------\n");
	printf("1. 树形索引结点中，一个节点可以放20个指针。\n");
	printf("2. 树形索引结点中，一个节点可以放50个不同的数据值。\n");
	printf("3. 连接操作中，缓冲区只能容纳每个关系的一个块。\n");
	printf("4. 允许的最大关系数以及每个关系中的最大属性数为3。\n");
	printf("5. 可以通过键入'#'退出本程序。\n");
	printf("-------------------- BEGIN TO USE ----------------------\n\n");
	return; 
}

int main()
{
	Declaration();
	
	int k = 1;
	
	Init();
	Input();
	
	int i=0;
	
	while(i<MAX_DIM){
		if(SelectionSwitch[i])
			EvaSelection(i);
		i++;
	}
	
	i = 0;
	while(JoinPara[i][0]!=-1){
		JoinSelection(JoinPara[i][0],JoinPara[i][1],JoinPara[i][2],JoinPara[i][3]);
		i++;
	}
	
	return 0;
}
