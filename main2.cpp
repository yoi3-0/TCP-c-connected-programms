#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include <dns_util.h>
#include "sema.h"
#include <netdb.h>
#include "vector"
#include <sys/socket.h>

using namespace std;

rk_sema sem, sem2;

struct thrd_args
{
    bool flag;
};

int soc1=socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
int soc2=socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
vector <string> msglist;
pthread_t id1, id2, id3;
thrd_args f1, f2, f3;
int exitcode_f1, exitcode_f2, exitcode_f3 = 0;

void *proc_1(void *value)
{
    cout << "Поток передачи запросов запущен\n";
    thrd_args *tem = (thrd_args *)value;
    string msglist1;
    while (tem->flag)
    {
        msglist1="Дай айпишник!";
        int sentcount = send(soc2, &msglist1, sizeof(msglist1), 0);
        if (sentcount == -1) {
            perror("send");
        }else{
            cout<<"Запрос отправлен! Тело запроса: "<<msglist1<<endl;
        }
        sleep(1);
    }
    cout << "Поток передачи запросов закончен\n";
    pthread_exit((void*)1);
}

void *proc_2(void *value)
{
    cout << "Поток приема ответов запущен\n";
    thrd_args *tem = (thrd_args *)value;
    char buf[15];
    while (tem->flag)
    {
        int reccount = recv(soc2, buf, sizeof(buf),0);
          if (reccount == -1) {
              perror("recv");
              sleep(1);
          }else{
              cout<<"Получен ответ! Адрес: "<<buf<<endl;
          }
        sleep(1);
    }
    cout << "Поток приема ответов закончен\n";
    pthread_exit((void*)2);
}

void *proc_3(void *value)
{
    cout << "Поток установления соединения запущен\n";
    thrd_args *tem = (thrd_args *)value;
    struct sockaddr_in serverAddr;
    serverAddr.sin_family =		AF_INET;
    serverAddr.sin_port =		htons(7000);
    serverAddr.sin_addr.s_addr =	inet_addr("127.0.0.1");
    while (tem->flag)
    {
        int result = connect(soc2, (sockaddr *) &serverAddr, sizeof (serverAddr));
        if (result==-1)
        {
            cerr<<"не получилось соединиться!\n";
            sleep(1);
            continue;
        }
        else {
            int t1 = pthread_create(&id1, NULL, proc_1, &f1);
            int t2 = pthread_create(&id2, NULL, proc_2, &f2);
            if (t1) cerr << "Не удалось инициировать поток передачи запросов\n";
            if (t2) cerr << "Не удалось инициировать поток приема ответов\n";
            cout << "Поток установления соединения закончен\n";
            pthread_exit((void*)3);
        }
    }
    cout << "Поток установления соединения закончен\n";
    pthread_exit((void*)3);
}

int main()
{
    f1.flag = f2.flag = f3.flag =true;
    rk_sema_init(&sem,1);
    rk_sema_init(&sem2,1);
    int t3 = pthread_create(&id3, NULL, proc_3, &f3);
    if (t3) cerr << "Не удалось инициировать поток установления соединения\n";
    //cout<<"wdwd";
    getchar();
    //cout<<"wdwd";
    f1.flag = f2.flag=f3.flag = false;
    pthread_join(id1, (void **)&exitcode_f1);
    pthread_join(id2, (void **)&exitcode_f2);
    pthread_join(id3, (void **)&exitcode_f3);
//    cout << "Proc_1 exited with code : " << exitcode_f1 << endl;
//    cout << "Proc_2 exited with code : " << exitcode_f2 << endl;
//    cout << "Proc_3 exited with code : " << exitcode_f3 << endl;
    return 0;
}