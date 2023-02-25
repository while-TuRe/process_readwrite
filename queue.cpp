#include<iostream>
#include<signal.h>
//fork
#include <sys/types.h>//fork
#include <unistd.h> //fork sleep
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
#include <mutex>//mutex unique_lock C++11
#include <shared_mutex>//shared_lock C++14
#include <thread>
#include <exception>
#include "./include/readPara.h"

#define SHM_KEY 0x121212

const int SIZE = getpagesize()/sizeof(int)-2;
using namespace std;

struct EmptyException : public exception
{
  const char * what () const throw ()
  {
    return "[read err]the queue is empty";
  }
};
struct FullException : public exception
{
  const char * what () const throw ()
  {
    return "[write err]the queue is full";
  }
};

class shmRWer{
    static int id;//sheared memory ctl id
    static int* a;//queue
    //写index是共享的，读index是独有的。
    int rindex;
    static int* windexp;//内容shmRWer::init后会改变，需存在共享内存中
    static int* cntp;//模仿sheard_ptr，计数并销毁 内容shmRWer::init后会改变，需存在共享内存中
    static shared_mutex mMutex;//锁 针对共享内存

public:
    static int initShm(){
        //创建共享内存(边界资源)
        id = shmget(SHM_KEY,SIZE+2,IPC_CREAT|0664);
        if(id<0){
            perror("shmger error ");
            return -1;
        }
        //映射
        windexp  = (int*)shmat(id,NULL,0);
        if(windexp<0){
            perror("shmat error ");
            return -1;
        }
        *windexp = 0;
        cntp = windexp +1;
        a = windexp + 2 ;
        *cntp =0;
        return 0;
    }
    shmRWer(){
        rindex=0;
        unique_lock guard(mMutex); 
        ++*cntp;
    }
    ~shmRWer(){
        unique_lock guard(mMutex); 
        int tmp = --*cntp;

        //分离
        shmdt(windexp);

        //cout<<"~shm and now exit process "<<tmp<<endl;
        if(tmp == 0){
            //删除
            if(shmctl(id,IPC_RMID,0)<0){
                perror("shmctl rm id error ");
                exit(-1);
            }
        }
    }
    int read(){
       std::shared_lock lock(mMutex);
        int windex = *windexp;
        if(rindex == windex){
            throw EmptyException();
        }
        int tmp = a[rindex];
        rindex =  (rindex+1)%SIZE;
        return tmp;
    }
    void write(int num){
        unique_lock guard(mMutex); 
        int windex = *windexp;
        if((rindex - windex + SIZE)%SIZE==1){
           throw FullException();
        }
        a[windex]=num;
        ++*windexp;
    }

};

//类外初始化，分配空间
int shmRWer::id =0;
int*shmRWer::a = nullptr;
int*shmRWer::windexp=nullptr;
int*shmRWer::cntp=nullptr;
shared_mutex shmRWer::mMutex = shared_mutex();//右值，移动构造函数
void usage(){
    cout<<"./queue --read_process_num 10 --write_process_num 1 --loop_time 3"<<endl;
}


int main(int argc, char**argv){
    //读取参数并处理
    Cmd cmd[] = {
        {"--read_process_num","read_process_num",IS_MUST,IS_PARA},
        {"--write_process_num","write_process_num",IS_MUST,IS_PARA},
        {"--loop_time","loop_time(s)",IS_MUST,IS_PARA},
        {NULL}
    };
    ostringstream e;
    if (read_para(e, cmd, argc, argv) < 0) {
        cerr << "【ERROR】" << e.str();
        usage();
        return -1;
    }

    int ret = shmRWer::initShm();//初始化
    if(ret<0)exit(-1);

    //fork write process
    for(int i=0;i<atoi(cmd[1].para);i++){
        pid_t pid = fork();
        if(pid<0){
            perror("fork error ");
            exit(-1);
        }else if(pid == 0){//子进程,write
            shmRWer shmW;
            //随机种子
	        srand((unsigned int)time(NULL));
            for(int j=0;j<atoi(cmd[2].para);j++){
                int tmp = rand()%100;
                
                try{
                    shmW.write(tmp);
                    cout<<"No. "<<i+1<<" write "<<tmp<<endl;
                }catch(FullException& e){
                    cout<<e.what()<<endl;
                    return 0;
                }
                sleep(1);
                
            }
            cout<<"No."<<i+1<<" return"<<endl;
            return 0;
        }

    }

    //fork read process
    for(int i=0;i<atoi(cmd[0].para);i++){
        pid_t pid = fork();
        if(pid<0){
            perror("fork error ");
            exit(-1);
        }else if(pid == 0){//子进程
            sleep(1);
            shmRWer shmR;
            for(int j=0;j<atoi(cmd[2].para);j++){
                try{
                    cout<<"No. "<<i+1<<" read "<<shmR.read()<<endl;
                }catch(EmptyException& e){
                    cout<<e.what()<<endl;
                    return 0;
                }
                sleep(1);
            }
            cout<<"No."<<i+1<<" read return"<<endl;
            return 0;
        }

    }
    wait(NULL);
    cout<<"parent return"<<endl;
    return 0;
}