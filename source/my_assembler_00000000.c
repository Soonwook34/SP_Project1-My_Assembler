/*
 * 화일명 : my_assembler_00000000.c 
 * 설  명 : 이 프로그램은 SIC/XE 머신을 위한 간단한 Assembler 프로그램의 메인루틴으로,
 * 입력된 파일의 코드 중, 명령어에 해당하는 OPCODE를 찾아 출력한다.
 */

/*
 * 프로그램의 헤더
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>            //bool변수를 사용하기 위해 추가

#include "my_assembler_00000000.h"

#define MAX_LINE_LENGTH 1000    //한 줄의 최대 길이
#define MAX_TOKEN_LENGTH 100    //한 토큰의 최대 길이

/* ----------------------------------------------------------------------------------
 * 설명 : 사용자로 부터 어셈블리 파일을 받아서 명령어의 OPCODE를 찾아 출력한다.
 * 매계 : 실행 파일, 어셈블리 파일 
 * 반환 : 성공 = 0, 실패 = < 0 
 * 주의 : 현재 어셈블리 프로그램의 리스트 파일을 생성하는 루틴은 만들지 않았다. 
 *		   또한 중간파일을 생성하지 않는다. 
 * ----------------------------------------------------------------------------------
 */
int main(int args, char *arg[])
{
	if (init_my_assembler() < 0)
	{
		printf("init_my_assembler: 프로그램 초기화에 실패 했습니다.\n");
		return -1;
	}

	if (assem_pass1() < 0)
	{
		printf("assem_pass1: 패스1 과정에서 실패하였습니다.  \n");
		return -1;
	}
	//make_opcode_output("output_00000000.txt");
    //make_opcode_output(NULL);

	make_symtab_output("symtab_00000000.txt");
    //make_symtab_output(NULL);
    
	make_literaltab_output("literaltab_00000000.txt");
    //make_literaltab_output(NULL);

	if (assem_pass2() < 0)
	{
		printf(" assem_pass2: 패스2 과정에서 실패하였습니다. \n");
		return -1;
	}

	make_objectcode_output("output_00000000.txt");
    //make_objectcode_output(NULL);

	return 0;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 프로그램 초기화를 위한 자료구조 생성 및 파일을 읽는 함수이다. 
 * 매계 : 없음
 * 반환 : 정상종료 = 0 , 에러 발생 = -1
 * 주의 : 각각의 명령어 테이블을 내부에 선언하지 않고 관리를 용이하게 하기 
 *		   위해서 파일 단위로 관리하여 프로그램 초기화를 통해 정보를 읽어 올 수 있도록
 *		   구현하였다. 
 * ----------------------------------------------------------------------------------
 */
int init_my_assembler(void)
{
	int result;

	if ((result = init_inst_file("inst.data")) < 0)
		return -1;
	if ((result = init_input_file("input.txt")) < 0)
		return -1;
	return result;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 머신을 위한 기계 코드목록 파일을 읽어 기계어 목록 테이블(inst_table)을 
 *        생성하는 함수이다. 
 * 매계 : 기계어 목록 파일
 * 반환 : 정상종료 = 0 , 에러 < 0 
 * 주의 : 기계어 목록파일 형식은 자유롭게 구현한다. 예시는 다음과 같다.
 * 기계어 목록파일 형식
 *	===============================================================================
 *		             | 이름 | 형식 | 기계어 코드 | 오퍼랜드의 갯수 |
 *	===============================================================================	   
 * ----------------------------------------------------------------------------------
 */
int init_inst_file(char *inst_file)
{
	FILE *file;
	int errno;

    //기계 명령어 파일 열기
    if ((file = fopen(inst_file, "r")) == NULL)
        errno = -1;
    else {
        inst_index = 0;
        //임시로 정보를 받을 변수
        char name[MAX_LINE_LENGTH] = "";
        int format = 0;
        int opcode = 0;
        int operandCnt = 0;
        //파일을 한 라인씩 읽으며 inst_table에 정보 저장
        while (fscanf(file, "%s\t%d\t%X\t%d\n", name, &format, &opcode, &operandCnt) != EOF) {
            inst_table[inst_index] = (inst*)malloc(sizeof(inst));

            inst_table[inst_index]->name = (char*)malloc(strlen(name) * sizeof(char) + 1);
            strcpy(inst_table[inst_index]->name, name);
            inst_table[inst_index]->format = format;
            inst_table[inst_index]->opcode = opcode;
            inst_table[inst_index]->operandCnt = operandCnt;
            inst_table[0]->format = 1;

            inst_index++;
        }
        fclose(file);
        errno = 0;
    }
    return errno;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 어셈블리 할 소스코드를 읽어 소스코드 테이블(input_data)를 생성하는 함수이다. 
 * 매계 : 어셈블리할 소스파일명
 * 반환 : 정상종료 = 0 , 에러 < 0  
 * 주의 : 라인단위로 저장한다.
 * ----------------------------------------------------------------------------------
 */
int init_input_file(char *input_file)
{
	FILE *file;
	int errno;

    //소스 코드 파일 열기
    if ((file = fopen(input_file, "r")) == NULL)
        errno = -1;
    else {
        line_num = 0;
        char tempCode[MAX_LINE_LENGTH] = "";    //임시로 소스코드를 받을 변수
        //파일을 한 라인씩 읽으며 input_data에 소스 코드 저장
        while (fscanf(file, "%[^\n]", tempCode) != EOF) {
            input_data[line_num] = (char*)malloc(strlen(tempCode) * sizeof(char) + 1);
            strcpy(input_data[line_num], tempCode);

            getc(file);
            line_num++;
        }
        fclose(file);
        errno = 0;
    }

    return errno;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 소스 코드를 읽어와 토큰단위로 분석하고 토큰 테이블을 작성하는 함수이다. 
 *        패스 1로 부터 호출된다. 
 * 매계 : 파싱을 원하는 문자열  
 * 반환 : 정상종료 = 0 , 에러 < 0 
 * 주의 : my_assembler 프로그램에서는 라인단위로 토큰 및 오브젝트 관리를 하고 있다. 
 * ----------------------------------------------------------------------------------
 */
int token_parsing(char *str)
{
    token_line = line_num;
    token_table[token_line] = (token*)malloc(sizeof(token));

    char tokenList[4][MAX_TOKEN_LENGTH] = { 0, };   //임시로 label, operator, operand, comment를 저장할 변수
    bool isLabelExist = true;                       //라벨 위치에 토큰이 있으면 true, 없으면 false

    //라벨 위치에 토큰이 있는지 확인
    if (str[0] == '\t')
        isLabelExist = false;
    char* restLine;
    char* tempToken = strtok_s(str, "\t", &restLine);
    int tokenCnt = 0;
    //tokenList에 정보 저장
    while (tempToken != NULL) {
        if (isLabelExist) {
            strcpy(tokenList[tokenCnt], tempToken);
            tempToken = strtok_s(NULL, "\t", &restLine);
        }
        else {
            tokenList[0][0] = '\0';
            isLabelExist = true;
        }
        tokenCnt++;
    }
    ///////////////tokenList를 바탕으로 token_table의 각각 해당하는 토큰에 정보 저장///////////////
    //label
    token_table[token_line]->label = (char*)malloc(strlen(tokenList[0]) * sizeof(char) + 1);
    strcpy(token_table[token_line]->label, tokenList[0]);

    //operator
    token_table[token_line]->operator = (char*)malloc(strlen(tokenList[1]) * sizeof(char) + 1);
    strcpy(token_table[token_line]->operator, tokenList[1]);

    //operand
        //피연산자는 ','로 한번 더 분리
    int opcode = search_opcode(tokenList[1]);
        //연산자가 RSUB과 같이 피연산자의 개수가 무조건 0개인 경우
        //tokenList[2]에 들어가 있는 주석을 tokenList[3]에 옮기고 피연산자 자리에 아무것도 없게 초기화
    if (opcode != -1 && inst_table[opcode]->operandCnt == 0) {
        strcpy(tokenList[3], tokenList[2]);
        memset(tokenList[2], 0, MAX_TOKEN_LENGTH);
    }
    char operandList[MAX_OPERAND][MAX_TOKEN_LENGTH] = { 0, };
    char* tempVar = strtok(tokenList[2], ",");
    int operandCnt = 0;
        //','로 피연산자를 구분하여 operandList에 저장
    while (tempVar != NULL) {
        if (operandCnt > 3)
            return -1;
        strcpy(operandList[operandCnt], tempVar);

        tempVar = strtok(NULL, ",");
        operandCnt++;
    }
        //구분된 피연산자를 token_table에 저장
    for (int i = 0; i < MAX_OPERAND; i++) {
        token_table[token_line]->operand[i] = (char*)malloc(strlen(operandList[i]) * sizeof(char) + 1);
        strcpy(token_table[token_line]->operand[i], operandList[i]);
    }

    //comment
    token_table[token_line]->comment = (char*)malloc(strlen(tokenList[3]) * sizeof(char) + 1);
    strcpy(token_table[token_line]->comment, tokenList[3]);

    ///////////////sym_table과 literal_table을 위한 주소 계산과 각각 테이블의 정보 저장///////////////
    //주소 계산
        //CSECT이면 주소 0으로 초기화
    if (strcmp(token_table[token_line]->operator, "CSECT") == 0)
        locctr = 0;
        //이전 주소값 저장
    prevLoc = locctr;

    opcode = search_opcode(token_table[token_line]->operator);
    //operator가 기계 명령어이면
    if (opcode != -1) {
        //기계 명령어의 format을 따라 주소 값 더하기
        locctr += inst_table[opcode]->format;
        if (token_table[token_line]->operator[0] == '+')
            locctr++;
    }
    //아니면 operator가
    else {
        //RESW인 경우
        if (strcmp(token_table[token_line]->operator, "RESW") == 0) {
            locctr += 3 * atoi(token_table[token_line]->operand[0]);
        }
        //RESB인 경우
        else if (strcmp(token_table[token_line]->operator, "RESB") == 0) {
            locctr += atoi(token_table[token_line]->operand[0]);
        }
        //EQU인 경우
        else if (strcmp(token_table[token_line]->operator, "EQU") == 0) {
            //피연산자에 *가 올 경우 주소 값 변동 없음
            if (strcmp(token_table[token_line]->operand[0], "*") == 0)
                locctr += 0;
            //수식인 경우
            else {
                //-가 들어간 수식이면
                char tempOperand[MAX_TOKEN_LENGTH] = { 0, };
                strcpy(tempOperand, token_table[token_line]->operand[0]);
                char* restString;
                char* token = strtok_s(tempOperand, "-", &restString);
                if (strlen(token) != strlen(token_table[token_line]->operand[0])) {
                    int var1, var2;
                    //각각의 주소값을 찾아서
                    var1 = search_symbol(token, -1);
                    var2 = search_symbol(restString, -1);
                    //Absolute Expression 계산
                    prevLoc = var1 - var2;
                }
                //단항이면(3byte 확보)
                else
                    locctr += 3;
            }
        }
        //BYTE인 경우
        else if (strcmp(token_table[token_line]->operator, "BYTE") == 0) {
            //X로 시작하는 경우
            if (token_table[token_line]->operand[0][0] == 'X')
                locctr += (strlen(token_table[token_line]->operand[0]) - 3) / 2;
            //C로 시작하는 경우
            else
                locctr += strlen(token_table[token_line]->operand[0]) - 3;
        }
        //WORD인 경우
        else if (strcmp(token_table[token_line]->operator, "WORD") == 0) {
            locctr += 3;
        }
        //LTORG 또는 END인 경우 현재까지 임시저장된 literal_table 완성(주소 할당)
        else if (strcmp(token_table[token_line]->operator, "LTORG") == 0 || strcmp(token_table[token_line]->operator, "END") == 0) {
            for (int i = literal_start; i < literal_index; i++) {
                literal_table[i].addr = prevLoc;
                if (literal_table[i].literal[1] == 'X')
                    locctr += (strlen(literal_table[i].literal) - 4) / 2;
                else
                    locctr += (strlen(literal_table[i].literal) - 4);
                prevLoc = locctr;
            }
            //다음 루틴의 literal을 위해 literal_start 업데이트
            literal_start = literal_index;
        }
    }

    //sym_table에 정보 저장
    if (strlen(token_table[token_line]->label) > 0 && strcmp(token_table[token_line]->label, ".") != 0) {
        //출력과 탐색을 위한 루틴별 구분(symbol이 개행문자, addr이 -1인 정보 추가)
        if (prevLoc == 0 && sym_table[sym_index - 1].addr != 0) {
            strcpy(sym_table[sym_index].symbol, "\n");
            sym_table[sym_index].addr = -1;
            sym_index++;
        }
        //정보 저장
        strcpy(sym_table[sym_index].symbol, token_table[token_line]->label);
        sym_table[sym_index].addr = prevLoc;
        sym_index++;
    }

    //리터럴 임시 저장(리터럴 이름만 저장하고 주소는 나중에 저장)
    if (token_table[token_line]->operand[0][0] == '=') {
        bool isNew = true;
        //리터럴 중복 검사
        for (int i = 0; i < literal_index; i++) {
            if (strcmp(literal_table[i].literal, token_table[token_line]->operand[0]) == 0) {
                isNew = false;
                break;
            }
        }
        //중복이 아니면 literal_table에 임시 저장(추가)
        if (isNew) {
            strcpy(literal_table[literal_index].literal, token_table[token_line]->operand[0]);
            literal_index++;
        }
    }

    return 0;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 입력 문자열이 기계어 코드인지를 검사하는 함수이다. 
 * 매계 : 토큰 단위로 구분된 문자열 
 * 반환 : 정상종료 = 기계어 테이블 인덱스, 에러 < 0
 * ----------------------------------------------------------------------------------
 */
int search_opcode(char *str)
{
    //연산자가 4-byte format을 나타내기 위해 맨 앞에 '+'를 사용한 경우 예외 처리
    if (str[0] == '+')
        str = str + 1;
    //입력받은 연산자가 inst_table에 존재하는지 검색
    inst_index = 0;
    while (inst_table[inst_index] != NULL && inst_index < MAX_LINES) {
        if (strcmp(str, inst_table[inst_index]->name) == 0) {
            return inst_index;  //존재할 경우 inst_table의 해당 연산자의 index값 리턴
        }
        inst_index++;
    }
    return -1;                  //존재하지 않을 경우 -1 리턴
}

/* ----------------------------------------------------------------------------------
 * 설명 : 입력 문자열이 sym_table에 속해있는지 검사하는 함수이다.
 * 매계 : symbol이라고 생각되는 문자열, 해당 루틴의 번호(-1이면 테이블 전체에서 검색)
 * 반환 : 정상종료 = 해당 symbol의 addr값, 에러 < 0
* -----------------------------------------------------------------------------------
*/
int search_symbol(char* str, int subRoutine)
{
    int sym_index = 0;
    int routineCnt = 0;
    bool searchAll = false;
    if (subRoutine == -1)
        searchAll = true;
    //입력받은 연산자가 해당 subRoutine의 sym_table에 존재하는지 검색
    while (strlen(sym_table[sym_index].symbol) != 0 && sym_index < MAX_LINES) {
        //루틴 찾아가기
        if (strcmp("\n", sym_table[sym_index].symbol) == 0)
            routineCnt++;
        if (!searchAll && routineCnt != subRoutine) {
            sym_index++;
            continue;
        }

        //해당 루틴에 symbol이 존재할 경우 symbol의 addr값 리턴
        if (strcmp(str, sym_table[sym_index].symbol) == 0)
            return sym_table[sym_index].addr;  
        sym_index++;
    }
    return -1;                  //존재하지 않을 경우 -1 리턴
}

/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 코드를 위한 패스1과정을 수행하는 함수이다.
*		   패스1에서는..
*		   1. 프로그램 소스를 스캔하여 해당하는 토큰단위로 분리하여 프로그램 라인별 토큰
*		   테이블을 생성한다.
*
* 매계 : 없음
* 반환 : 정상 종료 = 0 , 에러 = < 0
* 주의 : 현재 초기 버전에서는 에러에 대한 검사를 하지 않고 넘어간 상태이다.
*	  따라서 에러에 대한 검사 루틴을 추가해야 한다.
* -----------------------------------------------------------------------------------
*/
static int assem_pass1(void)
{
	/* input_data의 문자열을 한줄씩 입력 받아서 
	 * token_parsing()을 호출하여 token_unit에 저장
	 */
    line_num = 0;
    while (input_data[line_num] != NULL && line_num < MAX_LINES) {
        if (token_parsing(input_data[line_num]) < 0)
            return -1;
        line_num++;
    }

    return 0;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 명령어 옆에 OPCODE가 기록된 표(과제 5번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*        또한 과제 4번에서만 쓰이는 함수이므로 이후의 프로젝트에서는 사용되지 않는다.
* -----------------------------------------------------------------------------------
*/
// void make_opcode_output(char *file_name)
// {
// 	/* add your code here */

// }

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 SYMBOL별 주소값이 저장된 TABLE이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
* -----------------------------------------------------------------------------------
*/
void make_symtab_output(char *file_name)
{
    FILE* file;
    //출력 파일 열기
    if (file_name != NULL) {
        if ((file = fopen(file_name, "w")) == NULL)
            exit(1);
    }   //출력 파일 이름이 없는 경우(NULL)
    else
        file = stdout;  //표준출력으로 대체

    sym_index = 0;
    prevLoc = 0;
    //sym_table 정보 출력
    while (strlen(sym_table[sym_index].symbol) != 0 && sym_index < MAX_LINES) {
        //루틴별로 개행
        if (strcmp(sym_table[sym_index].symbol, "\n") == 0) {
            fprintf(file, "\n");
            sym_index++;
            continue;
        }

        fprintf(file, "%s\t\t%04X\n", sym_table[sym_index].symbol, sym_table[sym_index].addr);
        prevLoc = sym_table[sym_index].addr;
        sym_index++;
    }
    return;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 LITERAL별 주소값이 저장된 TABLE이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
* -----------------------------------------------------------------------------------
*/
void make_literaltab_output(char *file_name)
{
    FILE* file;
    //출력 파일 열기
    if (file_name != NULL) {
        if ((file = fopen(file_name, "w")) == NULL)
            exit(1);
    }   //출력 파일 이름이 없는 경우(NULL)
    else
        file = stdout;  //표준출력으로 대체

    literal_index = 0;
    //sym_table 정보 출력
    while (strlen(literal_table[literal_index].literal) != 0 && literal_index < MAX_LINES) {
        char tempLiteral[10];
        char* literalP = literal_table[literal_index].literal + 3;
        int i = 0;
        //"=C'ABC'"의 형태로 저장했기 때문에 리터럴만 출력하기 위해 처리
        for (; i < strlen(literal_table[literal_index].literal) - 4; i++, literalP++)
            tempLiteral[i] = *literalP;
        tempLiteral[i] = '\0';

        fprintf(file, "%s\t\t%04X\n", tempLiteral, literal_table[literal_index].addr);
        literal_index++;
    }
    return;
}

/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 코드를 기계어 코드로 바꾸기 위한 패스2 과정을 수행하는 함수이다.
*		   패스 2에서는 프로그램을 기계어로 바꾸는 작업은 라인 단위로 수행된다.
*		   다음과 같은 작업이 수행되어 진다.
*		   1. 실제로 해당 어셈블리 명령어를 기계어로 바꾸는 작업을 수행한다.
* 매계 : 없음
* 반환 : 정상종료 = 0, 에러발생 = < 0
* -----------------------------------------------------------------------------------
*/
static int assem_pass2(void)
{
    token_line = 0;
    code_index = 0;
    locctr = 0;
    prevLoc = 0;
    sym_index = 0;
    literal_index = 0;
    int subRoutine = -1;    //현재 루틴의 번호 저장
    char extrefList[3][MAX_TOKEN_LENGTH] = { 0, };  //EXTREF 변수 저장
    char tempLiteral[10];   //리터럴 임시 저장
    char* literalP;
    char tempSymbol[10];    //Symbol 임시 저장
    char* symbolP;
    int startIndex = 0;     //현재 루틴의 시작 index

    ///////////////token_table을 하나씩 읽어나가며 code_table에 정보 저장///////////////
    while (token_table[token_line] != NULL && token_line < MAX_LINES) {
        prevLoc = locctr;
        //루틴의 시작인 경우
        if (strcmp(token_table[token_line]->operator, "START") == 0 || strcmp(token_table[token_line]->operator, "CSECT") == 0) {
            //이전 루틴의 길이를 이전 H 레코드에 저장
            code_table[startIndex].addr = prevLoc;

            //현재 루틴의 H 레코드 정보 저장
            code_table[code_index].format = 0;
            code_table[code_index].line_index = token_line;
            code_table[code_index].record = 'H';

            startIndex = code_index;
            code_index++;
            subRoutine++;
            locctr = 0;
        }
        //EXTDEF인 경우
        else if (strcmp(token_table[token_line]->operator, "EXTDEF") == 0) {
            int i = 0;
            //개수 세기
            while (strlen(token_table[token_line]->operand[i]) != 0)
                i++;
            //D 레코드 정보 저장
            code_table[code_index].format = i;
            code_table[code_index].line_index = token_line;
            code_table[code_index].record = 'D';
            code_index++;
        }
        //EXTREF인 경우 extrefList에 정보 저장
        else if (strcmp(token_table[token_line]->operator, "EXTREF") == 0) {
            memset(extrefList, 0, sizeof(extrefList));
            int i = 0;
            //개수 세기
            while (strlen(token_table[token_line]->operand[i]) != 0) {
                strcpy(extrefList[i], token_table[token_line]->operand[i]);
                i++;
            }

            //R 레코드 정보 저장
            code_table[code_index].format = i;
            code_table[code_index].line_index = token_line;
            code_table[code_index].record = 'R';
            code_index++;
        }
        int opcode = search_opcode(token_table[token_line]->operator);
        //소스코드가 기계 명령어인 경우
            //nixbpe 파악하기
        if (opcode != -1) {
            //ni 비트 채우기
                //2-byte format이면
            if (inst_table[opcode]->format == 2) {
                token_table[token_line]->nixbpe = 0x00; //00 XXXX
            }
                //immediate addressing이면
            else if (token_table[token_line]->operand[0][0] == '#') {
                token_table[token_line]->nixbpe = 0x10; //01 XXXX
            }
                //indirect addressing이면
            else if (token_table[token_line]->operand[0][0] == '@') {
                token_table[token_line]->nixbpe = 0x20; //10 XXXX
            }
                //위의 모든 조건이 아니면(direct addressing)
            else {
                token_table[token_line]->nixbpe = 0x30; //11 XXXX
            }

            //xbpe 비트 채우기
                //2-byte format이면
            if (token_table[token_line]->nixbpe == 0x00) {
                token_table[token_line]->nixbpe |= 0x00;    //XX 0000
            }
            else {
                //X 레지스터를 사용하면
                if (strcmp(token_table[token_line]->operand[1], "X") == 0) {
                    token_table[token_line]->nixbpe |= 0x08;    //XX 1XXX
                }
                //4-byte format이면
                if (token_table[token_line]->operator[0] == '+') {
                    token_table[token_line]->nixbpe |= 0x01;    //XX XXX1
                }
                int i = 0;
                bool isExtref = false;
                //EXTREF를 통해 외부참조를 하는 경우
                while (strlen(extrefList[i]) != 0) {
                    if (strcmp(extrefList[i], token_table[token_line]->operand[0]) == 0) {
                        isExtref = true;
                        token_table[token_line]->nixbpe |= 0x00;    //XX XX0X
                    }
                    i++;
                }
                //immediate addressing
                if ((token_table[token_line]->nixbpe & 0x30) == 0x10) {
                    token_table[token_line]->nixbpe |= 0x00;    //XX XX0X
                }
                //위의 두 경우가 아닌경우
                else if (isExtref == false) {
                    token_table[token_line]->nixbpe |= 0x02;    //XX XX1X
                }
            }

            //뒷자리(displacement) 계산
            int format = inst_table[opcode]->format;    //2,3,4
            if ((token_table[token_line]->nixbpe & 0x01) == 0x01)
                format = 4;
            locctr += format;   //주소 계산
            int i = 0;
            char tempRegister[2] = { 0, };
            int tempCode = 0;
            char rList[10][3] = { "A", "X", "L", "B", "S", "T", "F", "", "PC", "SW" };
            int addr1 = 0;

            //byte format에 따라 계산 후 code_table에 정보 저장
            switch (format) {
                //2byte-format
            case 2:
                while (strlen(token_table[token_line]->operand[i]) != 0 && i < 2) {
                    for (int j = 0; j < 10; j++)
                        if (strcmp(token_table[token_line]->operand[i], rList[j]) == 0) {
                            tempRegister[i] = j;
                            break;
                        }
                    i++;
                }
                code_table[code_index].format = format;
                code_table[code_index].addr = prevLoc;
                code_table[code_index].code = (token_table[token_line]->nixbpe | (inst_table[opcode]->opcode << 4)) << 4;
                code_table[code_index].code |= tempRegister[0] << 4;
                code_table[code_index].code |= tempRegister[1];
                code_table[code_index].line_index = token_line;
                code_table[code_index].record = 'T';
                code_index++;
                break;
                //3byte-format
            case 3:
                    //RSUB과 같이 피연산자 개수가 0인 경우
                if (inst_table[opcode]->operandCnt == 0) {
                    code_table[code_index].format = format;
                    code_table[code_index].addr = prevLoc;
                    code_table[code_index].code = ((token_table[token_line]->nixbpe >> 4) | inst_table[opcode]->opcode) << 16;
                    code_table[code_index].line_index = token_line;
                    code_table[code_index].record = 'T';
                }
                else {
                    //immediate addressing이면
                    if (token_table[token_line]->operand[0][0] == '#') {
                        tempCode = atoi(token_table[token_line]->operand[0] + 1);
                    }
                    //일반적인 경우
                    else {
                        addr1 = search_symbol(token_table[token_line]->operand[0], subRoutine);
                        if (addr1 != -1)
                            tempCode = addr1 - locctr;
                        else {
                            i = 0;
                            while (strlen(literal_table[i].literal) != 0) {
                                if (strcmp(literal_table[i].literal, token_table[token_line]->operand[0]) == 0) {
                                    addr1 = literal_table[i].addr;
                                    tempCode = addr1 - locctr;
                                    break;
                                }
                                i++;
                            }
                        }
                    }
                    tempCode &= 0xFFF;
                    code_table[code_index].format = format;
                    code_table[code_index].addr = prevLoc;
                    code_table[code_index].code = (token_table[token_line]->nixbpe | (inst_table[opcode]->opcode << 4)) << 12;
                    code_table[code_index].code |= tempCode;    //뒤 12bit만 갖고 오기
                    code_table[code_index].line_index = token_line;
                    code_table[code_index].record = 'T';
                }
                code_index++;
                break;
                //4byte-format
            case 4:
                    //immediate addressing이면
                if (token_table[token_line]->operand[0][0] == '#') {
                    tempCode = atoi(token_table[token_line]->operand[0]);
                }
                else {
                    //외부 참조인 경우 M 레코드 저장
                    while (strlen(extrefList[i]) != 0) {
                        if (strcmp(extrefList[i], token_table[token_line]->operand[0]) == 0) {
                            tempCode = 0;
                            code_table[code_index].format = 5;
                            code_table[code_index].addr = prevLoc + 1;
                            code_table[code_index].line_index = token_line;
                            code_table[code_index].record = 'M';
                            code_table[code_index].modify = (char*)malloc(strlen(extrefList[i]) * sizeof(char) + 2);
                            code_table[code_index].modify[0] = '+';
                            strcpy(code_table[code_index].modify + 1, extrefList[i]);
                            code_index++;
                            break;
                        }
                        i++;
                    }
                }
                code_table[code_index].format = format;
                code_table[code_index].addr = prevLoc;
                code_table[code_index].code = (token_table[token_line]->nixbpe | (inst_table[opcode]->opcode << 4)) << 20;
                code_table[code_index].code |= tempCode;
                code_table[code_index].line_index = token_line;
                code_table[code_index].record = 'T';
                code_index++;
                break;
            }
        }
        //기계 명령어가 아닌 경우
        else {
            //LOTRG 또는 END
            int tempCode = 0;
            if (strcmp(token_table[token_line]->operator, "LTORG") == 0 || strcmp(token_table[token_line]->operator, "END") == 0) {
                while (locctr == literal_table[literal_index].addr) {
                    memset(tempLiteral, 0, sizeof(tempLiteral));
                    literalP = literal_table[literal_index].literal + 3;
                    int i = 0;
                    for (; i < strlen(literal_table[literal_index].literal) - 4; i++, literalP++)
                        tempLiteral[i] = *literalP;
                    tempLiteral[i] = '\0';

                    //X인 경우
                    if (literal_table[literal_index].literal[1] == 'X') {
                        locctr += (strlen(literal_table[literal_index].literal) - 4) / 2;
                        for (int i = 0; i < strlen(tempLiteral); i++) {
                            if (tempLiteral[i] >= 'A' && tempLiteral[i] <= 'F')
                                tempCode = (tempCode << 4) | (tempLiteral[i] - 'A' + 10);
                            else
                                tempCode = (tempCode << 4) | (tempLiteral[i] - '0');
                        }

                        code_table[code_index].format = (strlen(literal_table[literal_index].literal) - 4) / 2;
                        code_table[code_index].addr = prevLoc;
                        code_table[code_index].code = tempCode;
                        code_table[code_index].line_index = token_line;
                        code_table[code_index].record = 'T';
                        code_index++;
                    }
                    //C인 경우
                    else {
                        locctr += (strlen(literal_table[literal_index].literal) - 4);
                        for (int i = 0; i < strlen(tempLiteral); i++)
                            tempCode = (tempCode << 8) | tempLiteral[i];

                        code_table[code_index].format = (strlen(literal_table[literal_index].literal) - 4);
                        code_table[code_index].addr = prevLoc;
                        code_table[code_index].code = tempCode;
                        code_table[code_index].line_index = token_line;
                        code_table[code_index].record = 'T';
                        code_index++;
                    }
                    literal_index++;
                    prevLoc = locctr;
                }
            }
            //RESW
            else if (strcmp(token_table[token_line]->operator, "RESW") == 0)
                locctr += 3 * atoi(token_table[token_line]->operand[0]);
            //RESB
            else if (strcmp(token_table[token_line]->operator, "RESB") == 0)
                locctr += atoi(token_table[token_line]->operand[0]);
            //BYTE
            else if (strcmp(token_table[token_line]->operator, "BYTE") == 0) {
                memset(tempSymbol, 0, sizeof(tempSymbol));
                symbolP = token_table[token_line]->operand[0] + 2;
                int i = 0;
                for (; i < strlen(token_table[token_line]->operand[0]) - 3; i++, symbolP++)
                    tempSymbol[i] = *symbolP;
                tempSymbol[i] = '\0';
                //X인 경우
                if (token_table[token_line]->operand[0][0] == 'X') {
                    locctr += (strlen(token_table[token_line]->operand[0]) - 3) / 2;
                    for (int i = 0; i < strlen(tempSymbol); i++) {
                        if (tempSymbol[i] >= 'A' && tempSymbol[i] <= 'F')
                            tempCode = (tempCode << 4) | (tempSymbol[i] - 'A' + 10);
                        else
                            tempCode = (tempCode << 4) | (tempSymbol[i] - '0');
                    }

                    code_table[code_index].format = strlen(tempSymbol) / 2;
                    code_table[code_index].addr = prevLoc;
                    code_table[code_index].code = tempCode;
                    code_table[code_index].line_index = token_line;
                    code_table[code_index].record = 'T';
                    code_index++;
                }
                //C인 경우
                else {
                    locctr += strlen(token_table[token_line]->operand[0]) - 3;
                    for (int i = 0; i < strlen(tempSymbol); i++)
                        tempCode = (tempCode << 8) | tempSymbol[i];

                    code_table[code_index].format = strlen(tempSymbol);
                    code_table[code_index].addr = prevLoc;
                    code_table[code_index].code = tempCode;
                    code_table[code_index].line_index = token_line;
                    code_table[code_index].record = 'T';
                    code_index++;
                }
            }
            //WORD
            else if (strcmp(token_table[token_line]->operator, "WORD") == 0) {
                locctr += 3;
                //피연산자가 문자인 경우
                if (atoi(token_table[token_line]->operand[0]) == 0 && strlen(token_table[token_line]->operand[0]) > 1) {
                    //-가 들어간 수식이면
                    char tempOperand[MAX_TOKEN_LENGTH] = { 0, };
                    strcpy(tempOperand, token_table[token_line]->operand[0]);
                    char* restString;
                    char* token = strtok_s(tempOperand, "-", &restString);
                    if (strlen(token) != strlen(token_table[token_line]->operand[0])) {
                        int var1, var2;
                        //각각의 주소값을 찾아서
                        var1 = search_symbol(token, subRoutine);
                        var2 = search_symbol(restString, subRoutine);
                        //Absolute Expression 계산
                        if(var1 != -1 && var2 != -1)
                            tempCode = var1 - var2;
                        //외부 참조인 경우 M 레코드 정보 저장
                        else {
                            tempCode = 0;
                            code_table[code_index].format = 3 * 2;
                            code_table[code_index].addr = prevLoc + 1;
                            code_table[code_index].line_index = token_line;
                            code_table[code_index].record = 'M';
                            code_table[code_index].modify = (char*)malloc(strlen(token) * sizeof(char) + 2);
                            code_table[code_index].modify[0] = '+';
                            strcpy(code_table[code_index].modify + 1, token);
                            code_index++;

                            code_table[code_index].format = 3 * 2;
                            code_table[code_index].addr = prevLoc + 1;
                            code_table[code_index].line_index = token_line;
                            code_table[code_index].record = 'M';
                            code_table[code_index].modify = (char*)malloc(strlen(restString) * sizeof(char) + 2);
                            code_table[code_index].modify[0] = '-';
                            strcpy(code_table[code_index].modify + 1, restString);
                            code_index++;
                        }
                    }
                    //단항이면
                    else {
                        int var1 = search_symbol(token, subRoutine);
                        if (var1 != -1)
                            tempCode = var1;
                        else
                            tempCode = 0;
                    }
                    code_table[code_index].format = 3;
                    code_table[code_index].addr = prevLoc;
                    code_table[code_index].code = tempCode;
                    code_table[code_index].line_index = token_line;
                    code_table[code_index].record = 'T';
                    code_index++;
                }
                //피연산자가 숫자인 경우
                else {
                    code_table[code_index].format = 3;
                    code_table[code_index].addr = prevLoc;
                    code_table[code_index].code = atoi(token_table[token_line]->operand[0]);
                    code_table[code_index].line_index = token_line;
                    code_table[code_index].record = 'T';
                    code_index++;
                }
            }
        }
        token_line++;
    }
    //마지막 루틴의 길이를 H 레코드에 저장
    code_table[startIndex].addr = locctr;
    
    //E 레코드 추가
    code_table[code_index].format = 0;
    code_table[code_index].addr = locctr;
    code_table[code_index].line_index = token_line;
    code_table[code_index].record = 'E';

    return 0;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 object code (프로젝트 1번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
* -----------------------------------------------------------------------------------
*/
void make_objectcode_output(char *file_name)
{
    FILE* file;
    //출력 파일 열기
    if (file_name != NULL) {
        if ((file = fopen(file_name, "w")) == NULL)
            exit(1);
    }   //출력 파일 이름이 없는 경우(NULL)
    else
        file = stdout;  //표준출력으로 대체

    code_index = 0;
    int subRoutine = -1;
    int start_index = 0;
    ///////////////E 레코드가 나올때까지 Object Program 생성 및 출력///////////////
    while (code_table[code_index].record != 'E') {
        //루틴의 시작인 경우(H 레코드)
        if (code_table[code_index].record == 'H') {
            //이전 루틴의 M 레코드 출력
            if (subRoutine >= 0) {
                for (int i = start_index; i < code_index; i++) {
                    if (code_table[i].record == 'M')
                        fprintf(file, "M%06X%02X%s\n", code_table[i].addr, code_table[i].format, code_table[i].modify);
                }
            }
            //E 레코드 출력
            if (subRoutine == 0)
                fprintf(file, "E%06X\n\n", 0x0);
            else if (subRoutine > 0)
                fprintf(file, "E\n\n");
            fprintf(file, "H%-6s%06X%06X\n", token_table[code_table[code_index].line_index]->label, 0, code_table[code_index].addr);
            
            start_index = code_index;
            subRoutine++;
        }
        //EXTDEF인 경우(D 레코드)
        else if (code_table[code_index].record == 'D') {
            fprintf(file, "D");
            for (int i = 0; i < code_table[code_index].format; i++) {
                int addr = search_symbol(token_table[code_table[code_index].line_index]->operand[i], subRoutine);
                fprintf(file, "%-6s%06X", token_table[code_table[code_index].line_index]->operand[i], addr);
            }
            fprintf(file, "\n");
        }
        //EXTREF인 경우(R 레코드)
        else if (code_table[code_index].record == 'R') {
            fprintf(file, "R");
            for (int i = 0; i < code_table[code_index].format; i++)
                fprintf(file, "%-6s", token_table[code_table[code_index].line_index]->operand[i]);
            fprintf(file, "\n");
        }
        //T 레코드인 경우
        else if (code_table[code_index].record == 'T') {
            fprintf(file, "T");
            int maxLength = 0x1E;
            int length = code_table[code_index].format;
            int j = code_index+1;
            int prevJ = j - 1;
            //유효한 범위를 먼저 계산 후
            while (1) {
                //주소가 끊기거나 최대 길이(1E)를 넘어가면 중단
                if (code_table[j].record == 'M') {
                    j++;
                    continue;
                }
                if (code_table[j].record != 'T')
                    break;
                if (length + code_table[j].format > maxLength)
                    break;
                if (code_table[prevJ].addr + code_table[prevJ].format != code_table[j].addr)
                    break;
                length += code_table[j].format;
                prevJ = j;
                j++;
            }
            //시작 주소와 범위를 출력하고
            fprintf(file, "%06X%02X", code_table[code_index].addr, length);
            //object code를 출력한다
            for (; code_index < j; code_index++) {
                switch (code_table[code_index].format) {
                case 1:
                    fprintf(file, "%02X", code_table[code_index].code);
                    break;
                case 2:
                    fprintf(file, "%04X", code_table[code_index].code);
                    break;
                case 3:
                    fprintf(file, "%06X", code_table[code_index].code);
                    break;
                case 4:
                    fprintf(file, "%08X", code_table[code_index].code);
                    break;
                }
            }
            fprintf(file, "\n");
            code_index--;
        }
        code_index++;
    }

    //마지막 루틴의 M과 E 레코드 출력
    if (subRoutine >= 0) {
        for (int i = start_index; i < code_index; i++) {
            if (code_table[i].record == 'M') {
                fprintf(file, "M%06X%02X%s\n", code_table[i].addr, code_table[i].format, code_table[i].modify);
            }
        }
        fprintf(file, "E\n");
    }
}
