#include <sstream>
#include <iostream>
#include <string.h>
#include "../include/readPara.h"

/*参数处理*/
int read_para(ostringstream& sout,Cmd* cmd, int argc, char* argv[])
{
	for (int j = 0; cmd[j].cmdname; j++){
		cmd[j].time = 0;
		cmd[j].para = NULL;
	}

	for (int i = 1; i < argc; i++)
	{
		int j;
		for (j = 0; cmd[j].cmdname; j++)
			if (!strcmp(argv[i], cmd[j].cmdname))
				break;
		if (!cmd[j].cmdname)
		{
			sout << "参数[" << argv[i] << "]非法." << endl;
			return -1;
		}
		//参数是cmd[j]
		cmd[j].time++;
		//处理必须有参数而输入无参数
		if (cmd[j].is_para)
		{
			if (i == argc - 1 || !strncmp(argv[i + 1], "--", 2))
			{
				sout << "参数[" << argv[i] << "]缺少附加参数. (类型:string)" << endl;
				return -1;
			}
			cmd[j].para = argv[++i];
		}
	}
	//处理必须有但是没有的参数
	for (int j = 0; cmd[j].cmdname; j++)
		if (cmd[j].is_must && cmd[j].time==0)
		{
			sout << "必须指定[" << cmd[j].cmdname << "]" << endl;
			return -1;
		}
	
	return 0;
}