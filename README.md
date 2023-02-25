# process_readwrite
主要思路：
- 用fork分裂进程
- 用共享内存分享数据，共享内存前4字节是循环队列写指针，第二个4字节是使用共享内存的shmRWer数，之后为循环队列的数据存放区域。仿照智能指针的思路，在cnt归零后删除该共享内存。
- 用C++14标准的shared_mutex锁，shared_lock和unique_lock函数保护数据安全
使用说明：
- 登录Linux虚拟机，将本仓库文件夹下载至其中
- make 编译
- ./queue --read_process_num 10 --write_process_num 1 --loop_time 3 运行，三个参数意思分别是读进程数、写进程数、每个进程读写循环次数
- make clean 删除中间文件

