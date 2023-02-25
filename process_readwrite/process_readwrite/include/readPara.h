#include<sstream>
using namespace std;

#define IS_MUST 1
#define IS_PARA 1
#define PATH_MAX 1024

struct Cmd
{
	const char* cmdname, * ps;
	const int is_must, is_para;
	const char* para;
	int time;
};

/*参数处理*/
int read_para(ostringstream& sout, Cmd* cmd, int argc, char* argv[]);

