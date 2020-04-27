/* 
 * my_assembler 함수를 위한 변수 선언 및 매크로를 담고 있는 헤더 파일이다. 
 */
#define MAX_INST 256
#define MAX_LINES 5000
#define MAX_OPERAND 3

/*
 * instruction 목록 파일로 부터 정보를 받아와서 생성하는 구조체 변수이다.
 * 구조는 각자의 instruction set의 양식에 맞춰 직접 구현하되
 * 라인 별로 하나의 instruction을 저장한다.
 */
struct inst_unit
{
    char* name;     //기계 명령어 이름
    int format;     //바이트 형식
    int opcode;     //옵코드
    int operandCnt; //피연산자 개수
};

// instruction의 정보를 가진 구조체를 관리하는 테이블 생성
typedef struct inst_unit inst;
inst *inst_table[MAX_INST];
int inst_index;

/*
 * 어셈블리 할 소스코드를 입력받는 테이블이다. 라인 단위로 관리할 수 있다.
 */
char *input_data[MAX_LINES];
static int line_num;

/*
 * 어셈블리 할 소스코드를 토큰단위로 관리하기 위한 구조체 변수이다.
 * operator는 renaming을 허용한다.
 * nixbpe는 8bit 중 하위 6개의 bit를 이용하여 n,i,x,b,p,e를 표시한다.
 */
struct token_unit
{
	char *label;				//명령어 라인 중 label
	char *operator;				//명령어 라인 중 operator
	char *operand[MAX_OPERAND]; //명령어 라인 중 operand
	char *comment;				//명령어 라인 중 comment
	char nixbpe;				//하위 6bit 사용 : _ _ n i x b p e
};

typedef struct token_unit token;
token *token_table[MAX_LINES];
static int token_line;

/*
 * 심볼을 관리하는 구조체이다.
 * 심볼 테이블은 심볼 이름, 심볼의 위치로 구성된다.
 */
struct symbol_unit
{
	char symbol[10];
	int addr;
    int code_num;
};

typedef struct symbol_unit symbol;
symbol sym_table[MAX_LINES];
static int sym_index;       //sym_table에 접근하기 위한 index 변수

/*
* 리터럴을 관리하는 구조체이다.
* 리터럴 테이블은 리터럴의 이름, 리터럴의 위치로 구성된다.
*/
struct literal_unit
{
	char literal[10];
	int addr;
};

typedef struct literal_unit literal;
literal literal_table[MAX_LINES];
static int literal_start;   //루틴별 시작 index 정보를 저장하기 위한 변수
static int literal_index;   //literal_table에 접근하기 위한 index 변수

/*
* 오브젝트 코드를 관리하는 구조체이다.
* 오브젝트 코드 테이블은 오브젝트 코드의 포맷, 주소, 코드, token_table에서의 index, 레코드 등으로 구성된다.
*/
struct object_code
{
    int format;     //포맷
    int addr;       //주소
    int code;       //코드
    int line_index; //token_table에서의 index
    char record;    //레코드
    char* modify;   //M 레코드일 때 정보
};

typedef struct object_code code;
code code_table[MAX_LINES];     //오브젝트 코드 테이블
static int code_index;          //code_table에 접근하기 위한 index 변수

static int prevLoc;         //이전 주소를 저장하는 변수
static int locctr;
//--------------

static char *input_file;
static char *output_file;
int init_my_assembler(void);
int init_inst_file(char *inst_file);
int init_input_file(char *input_file);
int token_parsing(char *str);
int search_opcode(char *str);
//추가된 함수 : sym_table에서 해당 루틴의 symbol을 찾아 주소값을 리턴해주는 함수 search_symbol()
int search_symbol(char* str, int subRoutine);
static int assem_pass1(void);
//void make_opcode_output(char *file_name);
void make_symtab_output(char *file_name);
void make_literaltab_output(char *file_name);
static int assem_pass2(void);
void make_objectcode_output(char *file_name);
