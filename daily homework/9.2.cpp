/*
 * author: li zitong ID: 2017202121
 * time: 2019.10.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_INPUT_LENGTH 150 // �������������ַ�������
#define MAX_DIM 3 // i<=3
#define MAX_C 10 // �������������Ŀ 
#define MAX_C_LENGTH 30  // ����������󳤶�
#define MAX_TABLE_NAME 15 
#define ATTR_NAME_LENGTH 15
#define MAX_JOIN_NUM 10
#define JOIN_PARA 4
#define fi 50 //���νṹ�������ڲ�����ƽ���ɷ�50��Ԫ�� 
#define NODE 20
#define JOINCACHE 1 // �������Ӳ����У�������ֻ������ÿ����ϵ��һ����

/* glabal variables */
int relation_num; // ����г��ֵĹ�ϵ�� 
int clause_num; // ����г��ֵ������� 
int join_num; // ����е�������
int select_num; // ����е�ѡ���� 
int nR[MAX_DIM];
int fR[MAX_DIM];
int bR[MAX_DIM];
int MR[MAX_DIM];
int SelectionSwitch[MAX_DIM]; // ȷ����Щ����õ���ѡ�� 
int JoinPara[MAX_JOIN_NUM][JOIN_PARA];
int attribute_num[MAX_DIM]; // �������������� 
int DistR[MAX_DIM][MAX_DIM];
int MinR[MAX_DIM][MAX_DIM];
int MaxR[MAX_DIM][MAX_DIM];
int LowR[MAX_DIM][MAX_DIM][2]; // [i][j][0]: raw number [i][j][1]: for '=' 
int HighR[MAX_DIM][MAX_DIM][2];
int IndexR[MAX_DIM][MAX_DIM]; // 1:primary index; 2:secondary index; 3: no index, table scan; 4: binary search 
int HTR[MAX_DIM][MAX_DIM]; // �����Ե������߶� 
int LBR[MAX_DIM][MAX_DIM];
double SFR[MAX_DIM][MAX_DIM]; // �����Ե�ѡ���� 

char line[MAX_INPUT_LENGTH];
char clause[MAX_C][MAX_C_LENGTH];
char attrname[MAX_DIM][MAX_DIM][ATTR_NAME_LENGTH];
char tablename[MAX_DIM][MAX_TABLE_NAME];

// sqlԭ���� 
char * select = "select";
char * from = "from";
char * where = "where";
char * in = "in";
char * _and = "and";
char * left_bracket = "(" ;
char * right_bracket = ")";

//��ʼ������ 
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

//�����Ӵ��ĵ�һ���ַ�����λ�� 
int FindStr( char *str, char *substr) 
{
	int mainstrlen = strlen(str);  // ���ַ������� 
	int substrlen = strlen(substr);  // ���ַ������� 
	int i=0;
	
	char * cursor = str;
	char * tmp = (char *)malloc(sizeof(char)*(substrlen+2));
	
	while(cursor!=NULL && i != mainstrlen ){
		if(*cursor == substr[0]){
			strncpy(tmp,cursor,substrlen);
			tmp[substrlen] = '\0';  // �ض��ַ��� 
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

// ���ݱ�����ַ������ţ���Ÿ�������sql����б������ȷ���� 
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

// i Ϊ��Ӧ�ı���ţ�nameΪ�������������������Ϊname�����Բ�����Ӧ��ŵı�� 
int InsertAttrname(int i, char * name)
{	
	int j ;
	for( j=0;j<MAX_DIM;j++ )
	{
		if(strcmp(attrname[i][j],name)==0){ // ��������Ѵ��� 
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

//��num��ֵ������Ӧ��flag��������Ϊname�����Եķ�Χֵ 
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

// ��ȡline�ַ����У�head�ַ�����tail�ַ���֮����ַ��� 
char * GetStrBetween( char * head, char * tail, char * line)
{
	int headlen = strlen(head);
	int taillen = strlen(tail);
	
	int headpos = FindStr(line,head);
	int tailpos = FindStr(line,tail);
	
	// ���Ҳ�������ַ���λ�ã��򷵻�-1 
	if( headpos == -1 ){
		return NULL;
	}
	else if( tailpos == -1){
		return NULL ;
	}
	
	int strbetlen = tailpos - headpos - headlen - 2; //Ҫ��ȡ���Ӵ����� 
	
	char * strbet = (char *)malloc(sizeof(char)*(strbetlen+1));
	*(strbet+strbetlen) = '\0';
	
	strncpy(strbet, line+headpos+headlen+1, strbetlen);
	return strbet; 
}

// ������������� 
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

// �����������в��, clauseΪ��������硰R1.A1=R2.A2�� 
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
	
	//��ȡ������
	strncpy(tablename1,clause2,cursor-clause2);
	tablename1[cursor-clause2]='\0';
	 
	cursor++;
	
	//��ȡ�������� 
	strncpy(attrname1,cursor,cursor2-cursor);
	attrname1[cursor2-cursor]='\0';
	cursor2++;
	
	//��ȡ�ұ����
	cursor = strstr(cursor,".");
	strncpy(tablename2,cursor2,cursor-cursor2);
	tablename2[cursor-cursor2]='\0';
	
	//��ȡ�������� 
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

// ��һ�����������в�֣�clauseΪ���硰A1>10������� 
int ParseClause( char *clause )
{
	char * clause2 = (char *)malloc(sizeof(char)*MAX_C_LENGTH);
	char * attrname2 = (char *)malloc(sizeof(char)*ATTR_NAME_LENGTH);
	char * tablename = (char *)malloc(sizeof(char)*MAX_TABLE_NAME);
	strcpy(clause2, clause);
	
	char * cursor;
	char * cursor2;
	
	// ��ȡ����� 
	if((cursor2 = strstr(clause2,"."))){
		strncpy(tablename,clause2,cursor2-clause2);
		tablename[cursor2-clause2] = '\0';
		cursor2++;
	}else{
		tablename = "Default"; // ����Ĭ�ϱ���� 
	}
	
	// �Բ�ͬ���Ž������ƴ��� 
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

// �����ô�ӡ��� 
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

// ����ѡ���� 
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

// ������ײ�Ҷ���� 
int Cal_LB( )
{
	int i,j;
	
	for( i=0;i<MAX_DIM;i++ ){
		for( j=0;j<MAX_DIM;j++ ){
			if( DistR[i][j] ){
				// ���������ڵ�ɴ��20��ָ���,ÿ��������ٰ����� 
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

// ��ֺ�����lineΪ����sql��� 
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
	
	if( linetmp == NULL ){ // where ���� and 
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
			
			if( p == NULL )//û��and�ˣ�˵�������
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

/* EvaluateSelection�����飬������ppt˳�� */ 
int SelectA1(int i, int j)
{
	int ret = bR[i];
	//printf("A1:ȫ��ɨ�� E=bR=%d\n",bR[i]);
	return ret;
}

int SelectA2(int i, int j)
{
	int ret =  bR[i] + ceil(log(bR[i])/log(2)) + ceil(nR[i]*SFR[i][j]/fR[i]) -1 ;
	//printf("A2:�۰�ɨ�� E=bR + log2(bR)+SC(A,r)/fR-1=%d\n",ret);
	return ret;
}

int SelectA4(int i, int j)
{
	int ret = ceil(HTR[i][j]) + ceil(nR[i]*SFR[i][j]/fi);
	//printf("A4:������������ֵ�Ƚ� E=HTi+SC(A,r)/fr=%d\n",ret);
	return ret;
}

int SelectA5(int i, int j)
{
	int ret = ceil(HTR[i][j]) + (nR[i]*SFR[i][j]);
	//printf("A5:���ø���������ֵ�Ƚ� E=HTi+SC(A,r)=%d\n",ret);
	return ret;
}

int SelectA6(int i, int j)
{
	int ret = HTR[i][j] + bR[i]*SFR[i][j];
	//printf("A6:�����������ǵ�ֵ�Ƚ� E=HTi+bR*SFi=%d\n",ret);
	return ret;
}

int SelectA7(int i, int j)
{
	int ret = HTR[i][j] + LBR[i][j]*SFR[i][j]+nR[i]*SFR[i][j];
	//printf("A7:���ø��������ǵ�ֵ�Ƚ� E=HTi+LBi*SFi+nR*SFi=%d\n",ret);
	return ret;
}

// ���ض�Ӧattr[relation][i]���ʺϵĲ�ѯ��ʽ 
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

// ������С��ѡ����� 
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
	
	// ����Ԫ�������Եĸ����ֱ�����ܿ��� 
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

// ��Ƕ��ѭ������ 
int JoinB1( int table1, int attr1, int table2, int attr2 )
{
	int ret;
	int out = ( nR[table1]<nR[table2] )? table1:table2;
	int in = ( nR[table1]<nR[table2] )? table2:table1;
	ret = bR[out]*bR[in]+bR[out];
	
	return ret;
}

// ����ѭ��Ƕ�����ӣ�������table1��attr1Ϊ�������Ĵ��ۡ� 
int JoinB2( int table1, int attr1, int table2, int attr2 ) 
{
	int ret;
	int c = ceil(log(nR[table1])/log(NODE))+1;
	ret =  c*nR[table2] + bR[table2];
	return ret;
}

// �������Ѿ��ź���Ĺ鲢�������ӣ�����������۲��ÿ��ſռ临�Ӷȣ�O(nlog(2,n)) 
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
 
// ɢ������ 
int JoinB4( int table1, int attr1, int table2, int attr2 )
{
	int ret = 3*(bR[table1]+bR[table2]);
	return ret;
}

// ���ݴ�����ĸ����������������С�����Ӳ��� 
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
		case 1: printf("��Ƕ��ѭ�����ӣ��ܴ���Ϊ%d\n",cost);break;
		case 2: printf("����Ƕ��ѭ�����ӣ�����Ϊ%s���ܴ���Ϊ%d\n",attrname[table1][attr1],cost);break;
		case 3: printf("����Ƕ��ѭ�����ӣ�����Ϊ%s���ܴ���Ϊ%d\n",attrname[table2][attr2],cost);break;
		case 4: printf("����鲢���ӣ��ܴ���Ϊ%d\n",cost);break;
		case 5: printf("ɢ�����ӣ��ܴ���Ϊ%d\n",cost);break;
	}
	printf("END\n");
	
	return ret; 
} 

// ��ȡ���뺯�� 
int Input()
{
	printf("Please enter the complete SQL query:\n");
	gets(line);
	
	int len = strlen(line);
	
	//��ȥĩβ�Ŀո�ͷֺ� 
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

// ����˵�� 
void Declaration()
{
	printf("----- VERY SIMPLE OPTIMIZATION STRATEGY CALCULATOR -----\n\n");
	printf("------------ SOME SETTINGS FOR YOU TO KNOW -------------\n");
	printf("1. ������������У�һ���ڵ���Է�20��ָ�롣\n");
	printf("2. ������������У�һ���ڵ���Է�50����ͬ������ֵ��\n");
	printf("3. ���Ӳ����У�������ֻ������ÿ����ϵ��һ���顣\n");
	printf("4. ���������ϵ���Լ�ÿ����ϵ�е����������Ϊ3��\n");
	printf("5. ����ͨ������'#'�˳�������\n");
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
