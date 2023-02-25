#include<iostream>
#include<signal.h>
//fork
#include <sys/types.h>
#include <unistd.h> //sleep
#include <time.h>//random
//errno
#include<string.h>
#include<error.h>
//popen fscanf
#include <stdio.h>
#include <stdlib.h>
//share memroy
#include <sys/ipc.h>
#include <sys/shm.h>
//wait
#include <sys/wait.h>

#include "./include/readPara.h"

#define SHM_KEY 0x121212

const int SIZE = getpagesize()/sizeof(int)-2;
using namespace std;

class shmRWer{
    static int id;//sheared memory ctl id
    static int* a;//queue
    //写index是共享的，读index是独有的。
    int rindex;
    static int* windexp;//内容shmRWer::init后会改变，需存在共享内存中
    static int* cntp;//模仿sheard_ptr，计数并销毁 内容shmRWer::init后会改变，需存在共享内存中
    static mutex mMutex;//锁 针对共享内存

public:
    static int initShm(){
        //创建共享内存(边界资源)
        id = shmget(SHM_KEY,SIZE+2,IPC_CREAT|0664);
        if(id<0){
            perror("shmger error ");
            return -1;
        }
        //映射
        windexp  = (char*)shmat(id,NULL,0);
        if(windexp<0){
            perror("shmat error ");
            return -1;
        }
        *windexp = 0;
        cntp = windexp +1;
        a = windexp + 2 ;
        *cntp =0;
    }
    shmRWer():{
        rindex=0;
        ++*cntp;
    }
    ~shmRWer(){
        //分离
        shmdt(data);
        --*cntp;
        if(*cntp == 0){
            //删除
            if(shmctl(id,IPC_RMID,0)<0){
                perror("shmctl rm id error ");
                exit(-1);
            }
        }
    }
    int* read(){
        shared_lock lock(mMutex); 
        int windex = *windexp;
        if(rindex == windex){
            cerr<<"[read error]process id: "<<getpid()<< "the queue is empty"<<endl;
            exit(-1);
        }
        int tmp = a[rindex];
        rindex =  (rindex+1)%SIZE;
        return tmp;
    }
    void write(int num){
        unique_lock guard(mMutex); 
        int windex = *windexp;
        if((rindex - windex + SIZE)%SIZE==1){
            cerr<<"[write err]process id: "<<getpid()<< "the queue is full"<<endl;
            exit(-1);
        }
        a[windex]=num;
        ++*windexp;
    }

}
//类外初始化，分配空间
int shmRWer::id =0;
int*shmRWer::a = nullptr;
int*shmRWer::windexp=nullptr;
int*shmRWer::cntp=nullptr;
mutex*shmRWer::mMutex = mutex();//右值，移动构造函数

int main(int argc, char**argv){
    //读取参数并处理
    Cmd cmd[] = {
        {"--read_process_num","read_process_num",IS_MUST,IS_PARA},
        {"--write_process_num","write_process_num",IS_MUST,IS_PARA},
        {NULL}
    };
    ostringstream e;
    if (read_para(e, cmd, argc, argv) < 0) {
        cerr << "【ERROR】" << e.str();
        usage();
        return -1;
    }

    shmRWer::initShm();//初始化

    //fork write process
    for(int i=0;i<cmd[1];i++){
        pid_t pid = fork();
        if(pid<0){
            perror("fork error ");
            exit(-1);
        }else if(pid == 0){//子进程,write
            shmRWer shmW(data,mMutex);
            randomize();
            for(int j=0;j<10;j++){
                int tmp = random();
                cout<<"No."<<i+1<<"write "<<tmp<<endl;
                shmW.write(tmp);
                sleep(1);
            }
        }
    }

    //fork read process
    for(int i=0;i<cmd[0];i++){
        pid_t pid = fork();
        if(pid<0){
            perror("fork error ");
            exit(-1);
        }else if(pid == 0){//子进程
            shmRWer shmW(data,mMutex);
            for(int j=0;j<10;j++){
                cout<<"No."<<i+1<<"read "<<shmW.read()<<endl;
                sleep(1);
            }
        }
    }

    return 0;
}