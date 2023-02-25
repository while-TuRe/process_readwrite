#include <sstream>
#include <iostream>
#include <string.h>
#include "../include/readPara.h"

/*��������*/
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
			sout << "����[" << argv[i] << "]�Ƿ�." << endl;
			return -1;
		}
		//������cmd[j]
		cmd[j].time++;
		//��������в����������޲���
		if (cmd[j].is_para)
		{
			if (i == argc - 1 || !strncmp(argv[i + 1], "--", 2))
			{
				sout << "����[" << argv[i] << "]ȱ�ٸ��Ӳ���. (����:string)" << endl;
				return -1;
			}
			cmd[j].para = argv[++i];
		}
	}
	//��������е���û�еĲ���
	for (int j = 0; cmd[j].cmdname; j++)
		if (cmd[j].is_must && cmd[j].time==0)
		{
			sout << "����ָ��[" << cmd[j].cmdname << "]" << endl;
			return -1;
		}
	
	return 0;
}