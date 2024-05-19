#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string>
#include "Packet.h"


#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512

client_character character;

saved_mes saved_message[BUFSIZE];

int msg_stack = 0;

int state = 0; //�α��� = 0, �α��� ��� �ޱ� = 1, ���� = 2, Ŭ���̾�Ʈ ���� = 99

bool msg_check = false;


// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg)
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
void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

DWORD WINAPI Client_Thread(LPVOID sock)
{
    int retval = 0;
    int remain = 0;

    int msg_length = 0;
    int id_length = 0;

    char buf[BUFSIZE + 1];

    // �� �� ��ȯ�� ��Ʈ��
    std::string tmp_retval = "";
    std::string tmp_x = "";
    std::string tmp_y = "";
    std::string tmp_z = "";

    packet_init temp;

    msg_packet msgP;

    ch_move_packet ch_move;

    while (1)
    {
        // ������ �ޱ�
        retval = recv((SOCKET)sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            state = 99;
        }
        else if (retval == 0)
        {
            state = 99;
        }

        temp.header = 0;

        CopyMemory(&temp.header, buf + remain, sizeof(int));
        remain += sizeof(int);

        tmp_retval = std::to_string(retval);

        switch (temp.header)
        {
            case 2: // �α��� ��� �ޱ�
            {
                CopyMemory(&msg_length, buf + remain, sizeof(int));
                remain += sizeof(int);

                CopyMemory(&msgP.msg, buf + remain, sizeof(char) * msg_length);
                remain += sizeof(char) * msg_length;

                CopyMemory(&msgP.success, buf + remain, sizeof(int));
                remain += sizeof(int);

                msgP.msg[msg_length] = '\0';

                remain = 0;

                if (msgP.success == 1)
                {
                    printf("\n[�α��� ����] %s (%d����Ʈ)\n", msgP.msg, retval);
                    state = 2; // ���� ����
                }
                else
                {
                    printf("\n[�α��� ����] %s (%d����Ʈ)\n", msgP.msg, retval);
                    state = 99; // ���� ����

                    Sleep(10000);
                }

                break;
            }
            case 3:// ���� �� �ٸ� Ŭ���̾�Ʈ ���� �˸�
            {
                CopyMemory(&msg_length, buf + remain, sizeof(int));
                remain += sizeof(int);

                CopyMemory(&msgP.msg, buf + remain, sizeof(char) * msg_length);
                remain += sizeof(char) * msg_length;

                msgP.msg[msg_length] = '\0';

                remain = 0;

                strcpy_s(saved_message[msg_stack].msg, "\n[���� �˸�] ");
                strcat_s(saved_message[msg_stack].msg, msgP.msg);
                strcat_s(saved_message[msg_stack].msg, " (");
                strcat_s(saved_message[msg_stack].msg, tmp_retval.c_str());
                strcat_s(saved_message[msg_stack].msg, "����Ʈ)\n");

                msg_stack += 1;

                break;
            }
            case 5: // ���� ���� ��û ����
            {
                CopyMemory(&msg_length, buf + remain, sizeof(int));
                remain += sizeof(int);

                CopyMemory(&msgP.msg, buf + remain, sizeof(char) * msg_length);
                remain += sizeof(char) * msg_length;

                msgP.msg[msg_length] = '\0';

                printf("\n[���� ���� ���� �˸�] %s (%d����Ʈ)\n", msgP.msg, retval);

                remain = 0;

                state = 99;

                Sleep(10000);

                break;
            }
            case 6: // �ٸ� Ŭ���̾�Ʈ ���� ����
            {
                CopyMemory(&msg_length, buf + remain, sizeof(int));
                remain += sizeof(int);

                CopyMemory(&msgP.msg, buf + remain, sizeof(char) * msg_length);
                remain += sizeof(char) * msg_length;

                msgP.msg[msg_length] = '\0';

                remain = 0;

                strcpy_s(saved_message[msg_stack].msg, "\n[���� ���� �˸�] ");
                strcat_s(saved_message[msg_stack].msg, msgP.msg);
                strcat_s(saved_message[msg_stack].msg, " (");
                strcat_s(saved_message[msg_stack].msg, tmp_retval.c_str());
                strcat_s(saved_message[msg_stack].msg, "����Ʈ)\n");

                msg_stack += 1;

                break;
            }
            case 7: // Ŭ���̾�Ʈ ĳ���� �̵� �˸�
            {
                CopyMemory(&id_length, buf + remain, sizeof(int));
                remain += sizeof(int);

                CopyMemory(&ch_move.id, buf + remain, sizeof(char) * id_length);
                remain += sizeof(char) * id_length;

                CopyMemory(&character.x, buf + remain, sizeof(int));
                remain += sizeof(int);

                CopyMemory(&character.y, buf + remain, sizeof(int));
                remain += sizeof(int);

                CopyMemory(&character.z, buf + remain, sizeof(int));
                remain += sizeof(int);

                remain = 0;

                tmp_x = std::to_string(character.x);
                tmp_y = std::to_string(character.y);
                tmp_z = std::to_string(character.z);

                strcpy_s(saved_message[msg_stack].msg, "\n[�̵� �˸�] ��� ĳ���� = ");
                strcat_s(saved_message[msg_stack].msg, character.id);
                strcat_s(saved_message[msg_stack].msg, " \n[�̵� ��ǥ] x = ");
                strcat_s(saved_message[msg_stack].msg, tmp_x.c_str());
                strcat_s(saved_message[msg_stack].msg, ", y = ");
                strcat_s(saved_message[msg_stack].msg, tmp_y.c_str());
                strcat_s(saved_message[msg_stack].msg, ", z = ");
                strcat_s(saved_message[msg_stack].msg, tmp_z.c_str());
                strcat_s(saved_message[msg_stack].msg, " �� �̵� (");
                strcat_s(saved_message[msg_stack].msg, tmp_retval.c_str());
                strcat_s(saved_message[msg_stack].msg, "����Ʈ)\n");

                msg_stack += 1;

                break;
            }
            case 8: // �̵� ��� �Ϸ� �˸�
            {
                CopyMemory(&msg_length, buf + remain, sizeof(int));
                remain += sizeof(int);

                CopyMemory(&msgP.msg, buf + remain, sizeof(char) * msg_length);
                remain += sizeof(char) * msg_length;

                msgP.msg[msg_length] = '\0';

                remain = 0;

                strcpy_s(saved_message[msg_stack].msg, "\n[�̵� �˸�] ");
                strcat_s(saved_message[msg_stack].msg, msgP.msg);
                strcat_s(saved_message[msg_stack].msg, " (");
                strcat_s(saved_message[msg_stack].msg, tmp_retval.c_str());
                strcat_s(saved_message[msg_stack].msg, "����Ʈ)\n");

                msg_stack += 1;

                break;
            }
            case 9: // ��ȭ�޽���
            {
                CopyMemory(&msg_length, buf + remain, sizeof(int));
                remain += sizeof(int);

                CopyMemory(&msgP.msg, buf + remain, sizeof(char) * msg_length);
                remain += sizeof(char) * msg_length;

                msgP.msg[msg_length] = '\0';

                remain = 0;

                strcpy_s(saved_message[msg_stack].msg, "\n[��ȭ�޽���] ");
                strcat_s(saved_message[msg_stack].msg, msgP.msg);
                strcat_s(saved_message[msg_stack].msg, " (");
                strcat_s(saved_message[msg_stack].msg, tmp_retval.c_str());
                strcat_s(saved_message[msg_stack].msg, "����Ʈ)\n");

                msg_stack += 1;

                break;
            }
            default:
            {
                state = 99;
                break;
            }
        }
        id_length = 0;
        msg_length = 0;

        // ��� ���� �޽��� ���

        if (msg_check == false)
        {
            for (int i = 0; i < msg_stack; i++)
            {
                printf("%s", saved_message[i].msg);
                strcpy_s(saved_message[i].msg, "\0");
            }
            msg_stack = 0;
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    int remain = 0;
    char endNull = '\0';
    int retval;

    HANDLE hThread = NULL;

    req_log_packet req_log;
    packet_init req_discon;
    ch_move_packet ch_move;
    msg_packet send_msg;

    req_log.header = 1; // �α��� header = 1
    req_discon.header = 4; // �������� ��û header = 4
    ch_move.header = 7; // ĳ���� �̵� ��� header = 7
    send_msg.header = 9; // ��ü�޽��� ���� ��� header = 9

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // connect()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    // ������ ��ſ� ����� ����
    char buf[BUFSIZE + 1];

    int id_length = 0;
    int pw_length = 0;
    int msg_length = 0;

    bool input = false; // state 2 �϶� �������̽� üũ��

    bool check_special = false;

    while (1)
    {
        hThread = CreateThread(NULL, 0, Client_Thread, (LPVOID)sock, 0, NULL); // ������ �ޱ� �� ������ ����

        while (state == 0) // �α���
        {
            printf("[ID] : ");
            fflush(stdin);
            if (fgets(req_log.id, ID_SIZE, stdin) == NULL)
            {
                state = 99;
                break;
            }

            printf("[Password] : ");
            fflush(stdin);
            if (fgets(req_log.password, PW_SIZE, stdin) == NULL)
            {
                state = 99;
                break;
            }

            CopyMemory(buf + req_log.size, &req_log.header, sizeof(int));
            req_log.size += sizeof(int);

            id_length = strlen(req_log.id) - 1;
            pw_length = strlen(req_log.password) - 1;

            CopyMemory(buf + req_log.size, &id_length, sizeof(int));
            req_log.size += sizeof(int);

            CopyMemory(buf + req_log.size, &pw_length, sizeof(int));
            req_log.size += sizeof(int);

            CopyMemory(buf + req_log.size, &req_log.id, sizeof(char) * id_length);
            req_log.size += sizeof(char) * id_length;

            CopyMemory(buf + req_log.size, &req_log.password, sizeof(char) * pw_length);
            req_log.size += sizeof(char) * pw_length;

            CopyMemory(buf + req_log.size, &endNull, sizeof(char));
            req_log.size += sizeof(char);

            // ������ ������
            retval = send(sock, buf, req_log.size, 0);
            if (retval == SOCKET_ERROR) {
                err_display("send()");
                state = 99;
            }
            printf("\n[TCP Ŭ���̾�Ʈ] �α��� ��û�� ���½��ϴ�. (%d����Ʈ)\n", retval);
            req_log.size = 0;
            state = 1;
        }
        while (state == 1) //�α��� ��� ��� , ĳ���� ID ����
        {

            strcpy_s(character.id, req_log.id);

            strcpy_s(buf, "\n"); // ���� �ʱ�ȭ

            Sleep(1);
        }

        // ������ ������ ���
        while (state == 2)
        {

            if (input == false) // �������̽�
            {
                msg_check = true;

                printf("\n====================================================\n");
                printf("\n ������ �����Ͻÿ�. 1 or 2 or 3\n");
                printf("\n 1. ĳ������ ������\n");
                printf("\n 2. ��ȭ�޽���\n");
                printf("\n 3. ��������\n");
                printf("\n====================================================\n");
                
                msg_check = false;
                input = true;
            }

            if (_kbhit()) // �Է��� ���� Mutex�� ����ؼ� ����ȭ
            {
                // ������ �Է�
                msg_check = true;
                printf("\n[���� ����] ");
                fflush(stdin);

                while (strcmp(buf, "\n") == 0) // ����Ű �Է� ����
                {
                    if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
                    {
                        state = 99;
                        break;
                    }
                }

                if (strcmp(buf, "1\n") == 0) // ĳ���� �̵�
                {
                    printf("\n====================================================\n");
                    printf("\n ĳ���� ��ġ ��ȭ�� �Է��Ͻÿ�. \n");

                    printf("\n[ĳ���� �ĺ���] : ");
                    fflush(stdin);
                    if (fgets(ch_move.id, ID_SIZE, stdin) == NULL)
                        break;

                    printf("\n[x �� �̵�] : ");
                    scanf_s("%d", &ch_move.x);

                    printf("\n[y �� �̵�] : ");
                    scanf_s("%d", &ch_move.y);

                    printf("\n[z �� �̵�] : ");
                    scanf_s("%d", &ch_move.z);

                    printf("\n====================================================\n");

                    // ������ ����

                    CopyMemory(buf + ch_move.size, &ch_move.header, sizeof(int));
                    ch_move.size += sizeof(int);

                    id_length = strlen(ch_move.id) - 1;

                    CopyMemory(buf + ch_move.size, &id_length, sizeof(int));
                    ch_move.size += sizeof(int);

                    CopyMemory(buf + ch_move.size, &ch_move.id, sizeof(char) * id_length);
                    ch_move.size += sizeof(char) * id_length;

                    CopyMemory(buf + ch_move.size, &ch_move.x, sizeof(int));
                    ch_move.size += sizeof(int);

                    CopyMemory(buf + ch_move.size, &ch_move.y, sizeof(int));
                    ch_move.size += sizeof(int);

                    CopyMemory(buf + ch_move.size, &ch_move.z, sizeof(int));
                    ch_move.size += sizeof(int);

                    CopyMemory(buf + ch_move.size, &endNull, sizeof(char));
                    ch_move.size += sizeof(char);

                    retval = send(sock, buf, ch_move.size, 0);
                    if (retval == SOCKET_ERROR) {
                        err_display("send()");
                        state = 99;
                        break;
                    }

                    printf("\n[TCP Ŭ���̾�Ʈ] �̵� ��û�� ���½��ϴ�. (%d����Ʈ)\n", retval);

                    ch_move.size = 0;

                    input = false;
                }

                if (strcmp(buf, "2\n") == 0) // ��ȭ�޽��� �Է�
                {
                    printf("\n====================================================\n");

                    printf("\n[��ȭ�޽���] : ");
                    fflush(stdin);
                    if (fgets(send_msg.msg, MSG_SIZE, stdin) == NULL)
                        break;

                    msg_length = strlen(send_msg.msg) - 1;

                    // Ư�� ���� �˻�

                    for (int i = 0; i < msg_length; i++)
                    {
                        if (send_msg.msg[i] >= 33 && send_msg.msg[i] <= 47)
                        {
                            check_special = true;
                            break;
                        }

                        if (send_msg.msg[i] >= 58 && send_msg.msg[i] <= 64)
                        {
                            check_special = true;
                            break;
                        }

                        if (send_msg.msg[i] >= 91 && send_msg.msg[i] <= 96)
                        {
                            check_special = true;
                            break;
                        }

                        if (send_msg.msg[i] >= 123 && send_msg.msg[i] <= 126)
                        {
                            check_special = true;
                            break;
                        }

                    }

                    // ������ ����

                    if (check_special != true)
                    {
                        CopyMemory(buf + send_msg.size, &send_msg.header, sizeof(int));
                        send_msg.size += sizeof(int);

                        CopyMemory(buf + send_msg.size, &msg_length, sizeof(int));
                        send_msg.size += sizeof(int);

                        CopyMemory(buf + send_msg.size, &send_msg.msg, sizeof(char) * msg_length);
                        send_msg.size += sizeof(char) * msg_length;

                        CopyMemory(buf + send_msg.size, &endNull, sizeof(char));
                        send_msg.size += sizeof(char);

                        retval = send(sock, buf, send_msg.size, 0);
                        if (retval == SOCKET_ERROR) {
                            err_display("send()");
                            state = 99;
                            break;
                        }

                        printf("\n[TCP Ŭ���̾�Ʈ] �޽����� ���½��ϴ�. (%d����Ʈ)\n", retval);

                        send_msg.size = 0;
                    }

                    else
                    {
                        strcpy_s(send_msg.msg, "\0");

                        printf("\n[TCP Ŭ���̾�Ʈ] Ư�� ���ڴ� ������ �� �����ϴ�. \n");
                    }
                    input = false;
                    check_special = false;
                }

                if (strcmp(buf, "3\n") == 0) // ���� ����
                {
                    CopyMemory(buf + req_discon.size, &req_discon.header, sizeof(int));
                    req_discon.size += sizeof(int);

                    CopyMemory(buf + req_discon.size, &endNull, sizeof(char));
                    req_discon.size += sizeof(char);

                    retval = send(sock, buf, req_discon.size, 0);
                    if (retval == SOCKET_ERROR) {
                        err_display("send()");
                        state = 99;
                        break;
                    }

                    req_discon.size = 0;

                }

                strcpy_s(buf, "\n");
                msg_check = false;

                // ���� �Է��ϴ� ���߿� ������ ���� ���� �޽��� ���

                if (msg_stack != 0)
                {
                    for (int i = 0; i < msg_stack; i++)
                    {
                        printf("%s", saved_message[i].msg);
                        strcpy_s(saved_message[i].msg, "\0");
                    }
                    msg_stack = 0;
                }
            }

        }

        while (state == 99)
        {
            // close thread
            CloseHandle(hThread);

            // closesocket()
            closesocket(sock);

            // ���� ����
            WSACleanup();

            return 0;
        }
    }
}
