#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <iostream>
#include <string>
#include "Function.h"
#include "Packet.h"

#define SERVERPORT 9000
#define BUFSIZE    512
#define Max_Client 6 // Function.h �� total_account�� ����

// �ð� �������� ��ó https://dev-astra.tistory.com/182
time_t timer;
struct tm* t;

SOCKET client_sock_list[Max_Client] = { 0 };
const char* client_id_list[Max_Client] = {NULL};

// ���� �Լ� ���� ��� �� ����
void err_quit(const char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    LocalFree(lpMsgBuf);
    exit(1);
}

// ���� �Լ� ���� ���
void err_display(const char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char *)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(LPVOID arg)
{
    int remain = 0;
    int state = 0; // 0 = ���̵�, ��й�ȣ �ޱ� �� ��� ����

    SOCKET client_sock = (SOCKET)arg;
    int retval;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1];

    msg_packet login_msg;

    // Ŭ���̾�Ʈ ���� ���
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);

    int sock_number = 0; //���� ��ȣ ����

    std::string tmp = ""; // �ð� ���� �� �ӽ� ��Ʈ��

    //�α���

    while (1) {

        while (state == 0) // ���̵�, ��й�ȣ �ޱ�
        {
#pragma region ���̵�, ��й�ȣ �ޱ�
            // ������ �ޱ�
            retval = recv(client_sock, buf, BUFSIZE, 0);
            if (retval == SOCKET_ERROR) {
                err_display("recv()");
                break;
            }
            else if (retval == 0)
                break;

            req_log_packet req_log;

            CopyMemory(&req_log.header, buf + remain, sizeof(int));
            remain += sizeof(int);

            int id_length;
            int pw_length;

            CopyMemory(&id_length, buf + remain, sizeof(int));
            remain += sizeof(int);

            CopyMemory(&pw_length, buf + remain, sizeof(int));
            remain += sizeof(int);

            CopyMemory(&req_log.id, buf + remain, sizeof(char) * id_length);
            remain += sizeof(char) * id_length;

            CopyMemory(&req_log.password, buf + remain, sizeof(char) * pw_length);
            remain += sizeof(char) * pw_length;

            req_log.id[id_length] = '\0';
            req_log.password[pw_length] = '\0';

            remain = 0;
#pragma endregion

#pragma region ���� ���� ������

            login_msg.header = 2; // �α��� ��û ��� ��Ŷ ��� 2

            strcpy_s(login_msg.msg, req_log.id);
            strcat_s(login_msg.msg, ",");

            bool already_connected = false;

            for (int i = 0; i < Max_Client; i++) // �̹� ���ӵǾ� �ִ��� üũ
            {
                if (client_id_list[i] != NULL)
                {
                    if (strcmp(req_log.id, client_id_list[i]) == 0)
                    {
                        already_connected = true;
                        break;
                    }
                }
            }

            if (check_account(req_log.id, req_log.password) == true && already_connected == false)
            {
                strcat_s(login_msg.msg, " connection request success");

                login_msg.success = 1;
            }
            else
            {
                strcat_s(login_msg.msg, " connection request failure");

                login_msg.success = 0;
            }

            int msg_length = strlen(login_msg.msg); // �״��

            CopyMemory(buf + login_msg.size, &login_msg.header, sizeof(int));
            login_msg.size += sizeof(int);

            CopyMemory(buf + login_msg.size, &msg_length, sizeof(int));
            login_msg.size += sizeof(int);

            CopyMemory(buf + login_msg.size, &login_msg.msg, sizeof(char) * msg_length);
            login_msg.size += sizeof(char) * msg_length;

            CopyMemory(buf + login_msg.size, &login_msg.success, sizeof(int));
            login_msg.size += sizeof(int);

            CopyMemory(buf + login_msg.size, &endNull, sizeof(char));
            login_msg.size += sizeof(char);

            retval = send(client_sock, buf, login_msg.size, 0);
            if (retval == SOCKET_ERROR) {
                err_display("send()");
                break;
            }
            login_msg.size = 0;

            // ���� ��, ID ����
            for (int i = 0; i < Max_Client; i++)
            {
                if (client_sock_list[i] == 0)
                {
                    client_sock_list[i] = client_sock;
                    client_id_list[i] = req_log.id;
                    sock_number = i;
                    break;
                }
            }

#pragma endregion
            if (login_msg.success == 1)
            {

#pragma region ��ü �޽��� ������
                timer = time(NULL);
                t = localtime(&timer);

                int year = t->tm_year + 1900;
                int month = t->tm_mon + 1;
                int day = t->tm_mday;
                int hour = t->tm_hour;
                int min = t->tm_min;
                int sec = t->tm_sec;

                msg_packet connect_msg;

                strcpy_s(connect_msg.msg, ""); // �޽��� �ʱ�ȭ

                tmp = std::to_string(year);
                strcat_s(connect_msg.msg, tmp.c_str());
                strcat_s(connect_msg.msg, "�� ");

                tmp = std::to_string(month);
                strcat_s(connect_msg.msg, tmp.c_str());
                strcat_s(connect_msg.msg, "�� ");

                tmp = std::to_string(day);
                strcat_s(connect_msg.msg, tmp.c_str());
                strcat_s(connect_msg.msg, "�� ");

                tmp = std::to_string(hour);
                strcat_s(connect_msg.msg, tmp.c_str());
                strcat_s(connect_msg.msg, "�� ");

                tmp = std::to_string(min);
                strcat_s(connect_msg.msg, tmp.c_str());
                strcat_s(connect_msg.msg, "�� ");

                tmp = std::to_string(sec);
                strcat_s(connect_msg.msg, tmp.c_str());
                strcat_s(connect_msg.msg, "�� ");

                strcat_s(connect_msg.msg, req_log.id);
                strcat_s(connect_msg.msg, ", Connected");

                printf("\n[TCP ����] %s\n", connect_msg.msg);

                connect_msg.header = 3; // �α��� ���� ��ü �޽��� ��� 3
                msg_length = strlen(connect_msg.msg);

                CopyMemory(buf + connect_msg.size, &connect_msg.header, sizeof(int));
                connect_msg.size += sizeof(int);

                CopyMemory(buf + connect_msg.size, &msg_length, sizeof(int));
                connect_msg.size += sizeof(int);

                CopyMemory(buf + connect_msg.size, &connect_msg.msg, sizeof(char) * msg_length);
                connect_msg.size += sizeof(char) * msg_length;

                CopyMemory(buf + connect_msg.size, &endNull, sizeof(char));
                connect_msg.size += sizeof(char);

                for (int i = 0; i < Max_Client; i++)
                {
                    if (client_sock_list[i] != 0)
                    {
                        retval = send(client_sock_list[i], buf, connect_msg.size, 0);
                        if (retval == SOCKET_ERROR) {
                            err_display("send()");
                            state = 99;
                            break;
                        }

                        //printf("%d���� ����\n", client_sock_list[i]);
                    }
                }

                strcat_s(connect_msg.msg, "\n"); //�� �ٲ� �߰�
                append_log(connect_msg.msg); // �α� �ۼ�

                login_msg.size = 0;
                connect_msg.size = 0;

#pragma endregion

                state = 1;
            }
        }

        while (state == 1) // ĳ���� �̵� ���� �ޱ�, �޽��� ����, ���α׷� ����
        {
            packet_init temp;
            msg_packet discon_msg;
            id_check temp_id;
            msg_packet none_id;
            send_msg_packet send_msg;

            temp.header = 0;

            int msg_length = 0;

            int id_length = 0;
            bool send_check = false;

            // ������ �ޱ�
            retval = recv(client_sock, buf, BUFSIZE, 0);
            if (retval == SOCKET_ERROR) {
                err_display("recv()");
                break;
            }
            else if (retval == 0)
                break;

            int save_retval = retval;

            CopyMemory(&temp.header, buf + remain, sizeof(int));
            remain += sizeof(int);

            switch (temp.header) // ����� ��Ŷ �з�
            {
            case 4: // �������� ��û
                
                printf("\n[TCP ����] %s, disconnection request success\n", client_id_list[sock_number]);

                strcpy_s(discon_msg.msg, "OK"); // �޽��� ����

                msg_length = strlen(discon_msg.msg);

                discon_msg.header = 5; // ���� ���� ��û ���� ��� = 5

                CopyMemory(buf + discon_msg.size, &discon_msg.header, sizeof(int));
                discon_msg.size += sizeof(int);

                CopyMemory(buf + discon_msg.size, &msg_length, sizeof(int));
                discon_msg.size += sizeof(int);

                CopyMemory(buf + discon_msg.size, &discon_msg.msg, sizeof(char)* msg_length);
                discon_msg.size += sizeof(char) * msg_length;

                CopyMemory(buf + discon_msg.size, &endNull, sizeof(char));
                discon_msg.size += sizeof(char);

                retval = send(client_sock, buf, discon_msg.size, 0);
                if (retval == SOCKET_ERROR) {
                    err_display("send()");
                    state = 99;
                    break;
                }

                discon_msg.size = 0;

                break;

            case 7: // ĳ���� �̵� ���

                CopyMemory(&id_length, buf + remain, sizeof(int));
                remain += sizeof(int);

                CopyMemory(&temp_id.id, buf + remain, sizeof(char)* id_length);
                remain += sizeof(char) * id_length;

                temp_id.id[id_length] = '\0';

                for (int i = 0; i < Max_Client; i++)
                {
                    if (client_id_list[i] != NULL)
                    {
                        if (strcmp(client_id_list[i], temp_id.id) == 0)
                        {
                            retval = send(client_sock_list[i], buf, save_retval, 0);
                            if (retval == SOCKET_ERROR) {
                                err_display("send()");
                                state = 99;
                                break;
                            }

                            send_check = true;
                            break;
                        }
                    }
                }

                // �޽��� ����
                if (send_check == false) strcpy_s(none_id.msg, "��ġ�ϴ� ĳ���� �ĺ��ڰ� �����ϴ�."); 
                else strcpy_s(none_id.msg, "ĳ���� �̵��� �Ϸ��߽��ϴ�.");

                msg_length = strlen(none_id.msg);

                none_id.header = 8; // �̵� ��� �Ϸ� header = 8

                CopyMemory(buf + none_id.size, &none_id.header, sizeof(int));
                none_id.size += sizeof(int);

                CopyMemory(buf + none_id.size, &msg_length, sizeof(int));
                none_id.size += sizeof(int);

                CopyMemory(buf + none_id.size, &none_id.msg, sizeof(char)* msg_length);
                none_id.size += sizeof(char) * msg_length;

                CopyMemory(buf + none_id.size, &endNull, sizeof(char));
                none_id.size += sizeof(char);

                retval = send(client_sock, buf, none_id.size, 0);
                if (retval == SOCKET_ERROR) {
                    err_display("send()");
                    state = 99;
                    break;
                }

                none_id.size = 0;

                break;

            case 9: //��ȭ�޽��� ����

                CopyMemory(&msg_length, buf + remain, sizeof(int));
                remain += sizeof(int);

                CopyMemory(&send_msg.msg, buf + remain, sizeof(char) * msg_length);
                remain += sizeof(char) * id_length;

                send_msg.msg[msg_length] = '\0';

                strcpy_s(send_msg.id_msg, "< ");
                strcat_s(send_msg.id_msg, client_id_list[sock_number]);
                strcat_s(send_msg.id_msg, " > ");
                strcat_s(send_msg.id_msg, send_msg.msg);

                //printf("%s", send_msg.id_msg);

                msg_length = strlen(send_msg.id_msg);

                CopyMemory(buf + send_msg.size, &temp.header, sizeof(int));
                send_msg.size += sizeof(int);

                CopyMemory(buf + send_msg.size, &msg_length, sizeof(int));
                send_msg.size += sizeof(int);

                CopyMemory(buf + send_msg.size, &send_msg.id_msg, sizeof(char) * msg_length);
                send_msg.size += sizeof(char) * msg_length;

                for (int i = 0; i < Max_Client; i++) // ��ü �޽��� �߼�
                {
                    if (client_sock_list[i] != 0)
                    {
                        retval = send(client_sock_list[i], buf, send_msg.size, 0);
                        if (retval == SOCKET_ERROR) {
                            err_display("send()");
                            state = 99;
                            break;
                        }
                    }
                }

                send_msg.size = 0;

                break;

            default:
                state = 99;
                break;

            }

            send_check = false;
            remain = 0;
        }

        // closesocket()
        closesocket(client_sock);
        printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // ��ü �޽��� ����

        if (login_msg.success == 1)
        {
            msg_packet discon_global_msg;

            timer = time(NULL);
            t = localtime(&timer);

            int year = t->tm_year + 1900;
            int month = t->tm_mon + 1;
            int day = t->tm_mday;
            int hour = t->tm_hour;
            int min = t->tm_min;
            int sec = t->tm_sec;

            discon_global_msg.header = 6; // ���� ���� ��ü �˸� = 6

            strcpy_s(discon_global_msg.msg, ""); // �޽��� �ʱ�ȭ

            tmp = std::to_string(year);
            strcat_s(discon_global_msg.msg, tmp.c_str());
            strcat_s(discon_global_msg.msg, "�� ");

            tmp = std::to_string(month);
            strcat_s(discon_global_msg.msg, tmp.c_str());
            strcat_s(discon_global_msg.msg, "�� ");

            tmp = std::to_string(day);
            strcat_s(discon_global_msg.msg, tmp.c_str());
            strcat_s(discon_global_msg.msg, "�� ");

            tmp = std::to_string(hour);
            strcat_s(discon_global_msg.msg, tmp.c_str());
            strcat_s(discon_global_msg.msg, "�� ");

            tmp = std::to_string(min);
            strcat_s(discon_global_msg.msg, tmp.c_str());
            strcat_s(discon_global_msg.msg, "�� ");

            tmp = std::to_string(sec);
            strcat_s(discon_global_msg.msg, tmp.c_str());
            strcat_s(discon_global_msg.msg, "�� ");

            strcat_s(discon_global_msg.msg, client_id_list[sock_number]);
            strcat_s(discon_global_msg.msg, ", Disconnected");

            printf("\n[TCP ����] %s\n", discon_global_msg.msg);

            int msg_length = strlen(discon_global_msg.msg);

            CopyMemory(buf + discon_global_msg.size, &discon_global_msg.header, sizeof(int));
            discon_global_msg.size += sizeof(int);

            CopyMemory(buf + discon_global_msg.size, &msg_length, sizeof(int));
            discon_global_msg.size += sizeof(int);

            CopyMemory(buf + discon_global_msg.size, &discon_global_msg.msg, sizeof(char) * msg_length);
            discon_global_msg.size += sizeof(char) * msg_length;

            CopyMemory(buf + discon_global_msg.size, &endNull, sizeof(char));
            discon_global_msg.size += sizeof(char);

            for (int i = 0; i < Max_Client; i++)
            {
                if (client_sock_list[i] != 0 && i != sock_number)
                {
                    retval = send(client_sock_list[i], buf, discon_global_msg.size, 0);
                    if (retval == SOCKET_ERROR) {
                        err_display("send()");
                        state = 99;
                        break;
                    }
                }
            }

            strcat_s(discon_global_msg.msg, "\n"); //�� �ٲ� �߰�
            append_log(discon_global_msg.msg); // �α� �ۼ�

            discon_global_msg.size = 0;
        }
        client_sock_list[sock_number] = 0;
        client_id_list[sock_number] = NULL;

        return 0;
    }
}

int main(int argc, char *argv[])
{
    int retval;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    // ������ ��ſ� ����� ����
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    HANDLE hThread;

    while (1) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // ������ Ŭ���̾�Ʈ ���� ���
        printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // ������ ����
        hThread = CreateThread(NULL, 0, ProcessClient,
            (LPVOID)client_sock, 0, NULL);
        if (hThread == NULL) { closesocket(client_sock); }
        else { CloseHandle(hThread); }
    }

    // closesocket()
    closesocket(listen_sock);

    // ���� ����
    WSACleanup();
    return 0;
}
