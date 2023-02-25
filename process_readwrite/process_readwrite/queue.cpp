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
    //дindex�ǹ���ģ���index�Ƕ��еġ�
    int rindex;
    static int* windexp;//����shmRWer::init���ı䣬����ڹ����ڴ���
    static int* cntp;//ģ��sheard_ptr������������ ����shmRWer::init���ı䣬����ڹ����ڴ���
    static shared_mutex mMutex;//�� ��Թ����ڴ�

public:
    static int initShm(){
        //���������ڴ�(�߽���Դ)
        id = shmget(SHM_KEY,SIZE+2,IPC_CREAT|0664);
        if(id<0){
            perror("shmger error ");
            return -1;
        }
        //ӳ��
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

        //����
        shmdt(windexp);

        //cout<<"~shm and now exit process "<<tmp<<endl;
        if(tmp == 0){
            //ɾ��
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

//�����ʼ��������ռ�
int shmRWer::id =0;
int*shmRWer::a = nullptr;
int*shmRWer::windexp=nullptr;
int*shmRWer::cntp=nullptr;
shared_mutex shmRWer::mMutex = shared_mutex();//��ֵ���ƶ����캯��
void usage(){
    cout<<"./queue --read_process_num 10 --write_process_num 1 --loop_time 3"<<endl;
}


int main(int argc, char**argv){
    //��ȡ����������
    Cmd cmd[] = {
        {"--read_process_num","read_process_num",IS_MUST,IS_PARA},
        {"--write_process_num","write_process_num",IS_MUST,IS_PARA},
        {"--loop_time","loop_time(s)",IS_MUST,IS_PARA},
        {NULL}
    };
    ostringstream e;
    if (read_para(e, cmd, argc, argv) < 0) {
        cerr << "��ERROR��" << e.str();
        usage();
        return -1;
    }

    int ret = shmRWer::initShm();//��ʼ��
    if(ret<0)exit(-1);

    //fork write process
    for(int i=0;i<atoi(cmd[1].para);i++){
        pid_t pid = fork();
        if(pid<0){
            perror("fork error ");
            exit(-1);
        }else if(pid == 0){//�ӽ���,write
            shmRWer shmW;
            //�������
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
        }else if(pid == 0){//�ӽ���
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