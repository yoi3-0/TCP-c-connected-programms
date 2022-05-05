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
    cout << "Поток приёма запросов запущен\n";
    thrd_args *tem = (thrd_args *)value;
    char buf[50];
    cout<<"Вошёл в поток приёма";
    while (tem->flag)
    {
        cout<<"Вошёл в поток приёма";
        int reccount = recv(soc2, buf, sizeof(buf),0);
        cout<<"Принял"<<reccount;
        if (reccount == -1) {
            perror("recv");
            sleep(1);
        } else if (reccount == 0) {
            shutdown(soc2,SHUT_RDWR);
            sleep(1);
        }else{
            rk_sema_wait(&sem);
            msglist.push_back(buf);
            rk_sema_post(&sem);
        }
        sleep(1);
    }
    cout << "Поток приёма запросов закончен\n";
    pthread_exit((void*)1);
}

void *proc_2(void *value)
{
    cout << "Поток обработки и отправки ответов запущен\n";
    thrd_args *tem = (thrd_args *)value;
    string msglist1;
    cout<<"Вошёл в поток отправки";
    while (tem->flag)
    {
        char buf[15];
        rk_sema_wait(&sem);
        if (!msglist.empty()) {
            msglist1 = msglist.back();
            msglist.pop_back();
            rk_sema_post(&sem);
            cout << "Запрос " << msglist1 << " прочитан\n";
            hostent *host1;
            host1 = gethostbyname("www.google.ru");
            strcpy(buf, inet_ntoa(*(struct in_addr *) host1->h_addr));
            int sentcount = send(soc2, buf, sizeof(buf), 0);
            if (sentcount == -1) {
                perror("send");
            }else{
                cout<<"Сообщение с айпи-адресом отправлено! Адрес: "<<buf<<endl;
            }
            sleep(1);
        }else{
            rk_sema_post(&sem);
            sleep(1);
        }
    }
    cout << "Поток обработки и отправки ответов закончен\n";
    pthread_exit((void*)2);
}

void *proc_3(void *value)
{
    cout << "Поток ожидания соединений запущен\n";
    //cout<<"aaaa";
    thrd_args *tem = (thrd_args *)value;
    //cout<<tem->flag;
    //cout<<"aaaa";
    struct sockaddr_in serverAddr;
    serverAddr.sin_family =		AF_INET;
    serverAddr.sin_port =		htons(7000);
    serverAddr.sin_addr.s_addr =	inet_addr("127.0.0.1");
    bind(soc1, (sockaddr *) &serverAddr, sizeof (serverAddr));
    //cout<<tem->flag;
    //cout<<"aaaa";
    while (tem->flag)
    {
        listen(soc1, 1);
        soc2=accept(soc1,NULL,NULL);
        if (soc2==-1)
        {
            cerr<<"не получилось принять!\n";
            sleep(1);
            continue;
        }
        else {
            int t1 = pthread_create(&id1, NULL, proc_1, &f1);
            int t2 = pthread_create(&id2, NULL, proc_2, &f2);
            if (t1) cerr << "Не удалось инициировать поток приёма запросов\n";
            if (t2) cerr << "Не удалось инициировать поток обработки и отправки ответов\n";
            cout << "Поток ожидания соединений закончен\n";
            pthread_exit((void*)3);
        }
    }
    cout << "Поток ожидания соединений закончен\n";
    pthread_exit((void*)3);
}

int main()
{
    f1.flag = f2.flag = f3.flag =true;
    rk_sema_init(&sem,1);
    rk_sema_init(&sem2,1);
    int t3 = pthread_create(&id3, NULL, proc_3, &f3);
    if (t3) cerr << "Не удалось инициировать поток ожидания соединений\n";
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