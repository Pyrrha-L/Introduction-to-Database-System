/*
 * author:lizitong ID:2017202121
 * time:2019.10.
 * 
 */
 
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

#define MAX_LENGTH 200 //���������ַ�������
#define MAX_TABLE_AMOUNT 10 // �������ı���� 
#define MAX_TABLE_NAME_LENGTH 30 // ������Ƶ���󳤶�
#define MAX_ATTRIBUTE_AMOUNT 50 // ��������������
#define MAX_ATTRIBUTE_AMOUNT_PER_TABLE 5 // ÿ���������5������ 
#define MAX_ATTRIBUTE_NAME_LENGTH 30 // �������Ƶ���󳤶�
#define MAX_CLAUSE_AMOUNT 20 // �������Ĺ�ϵ���ʽ�� 
#define MAX_CLAUSES_LENGTH 40 // ����ϵ���ʽ���� 

typedef struct
{
	char * attrname;
	int tableno;
	int attrno;
	int outputflag = 0;
}Attrs;  // ���Խṹ�� 

typedef struct
{
	char * tablename;
	int tableno;
}Tables;  // ���ṹ�� 

typedef struct
{
	char * content;
	int tableno;
	int clauseno;
}Clauses;  // �������ṹ�� 

//������ 
int table_counter = 0;
int attribute_counter = 0;
int clause_counter = 0;

//�̶����� 
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

int PreTreatment( char * str ) //Ԥ������Сдת�����������Ŀ��� 
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

// �����Բ��������б� 
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

// �������б� 
void InsertTables( char * table )
{
//	table_counter ++ Ӧ�ŵ���parser���У���Ϊһ��parser����һ����� 
	tablelist[table_counter].tablename = table;
	tablelist[table_counter].tableno = table_counter;
}

//�����ϵ���ʽ�б� 
void InsertClauses( char * linecut )
{
	clauselist[clause_counter].clauseno = clause_counter;
	clauselist[clause_counter].content = linecut;
	clauselist[clause_counter].tableno = table_counter;
	clause_counter ++;
}

int FindStr( char *str, char *substr) //�����Ӵ��ĵ�һ���ַ�����λ�� 
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

void Parser( char * line, int flag ) 
//�˴������line��ʽӦΪ��select A from B where C in��
//��line����ȡ�����������������������䣬�����й��б� 
{	
	char * line2 = (char * )malloc(sizeof(char)*MAX_LENGTH);
	strcpy(line2,line);
	
	char * tablename1 = GetStrBetween(from, where, line2);
	InsertTables(tablename1);

	table_counter++;

	char * attrname1 =  GetStrBetween(select, from, line2);
	InsertAttrs(attrname1, flag);
	
	
	char * linetmp = GetStrBetween(where, _and , line2);
	
	if( linetmp == NULL ){ // where ���� and 
		linetmp = GetStrBetween(where, in , line2); 
		if( linetmp == NULL ){ // where ���� in��˵���������һ�������� 
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
			
			if( p == NULL )//û��and�ˣ�˵�������
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
// ��ѭ������ʽ�ָ�����
{
	//�������� 
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
			line2 = tmpline; // �޸��ַ��� 
		}
		else{ //�Ѿ�û���������� 
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

// ��ɷ�Ƕ�׻���䲢��� 
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
	
	// ������������� 
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

//����ȫ�ֱ��� 
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

/*  #1 error: stray '\241' in program : �������зǷ������ģ��ַ��� 
	#2 ѭ���У�ʹ����һ������������ȫ�ֱ����� 
	#3 ���ַ������д���ʱ�������ԭ���ַ�������֮�������´�����
	���������ⲻ��Ҫ���ַ���ȡ���� 
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

 
