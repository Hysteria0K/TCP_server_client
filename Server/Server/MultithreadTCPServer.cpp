#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
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
#define Max_Client 6 // Function.h 의 total_account와 동일

// 시간 가져오기 출처 https://dev-astra.tistory.com/182
time_t timer;
struct tm* t;

SOCKET client_sock_list[Max_Client] = { 0 };
const char* client_id_list[Max_Client] = {NULL};

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
    int remain = 0;
    int state = 0; // 0 = 아이디, 비밀번호 받기 및 결과 전송

    SOCKET client_sock = (SOCKET)arg;
    int retval;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1];

    msg_packet login_msg;

    // 클라이언트 정보 얻기
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);

    int sock_number = 0; //소켓 번호 저장

    std::string tmp = ""; // 시간 저장 용 임시 스트링

    //로그인

    while (1) {

        while (state == 0) // 아이디, 비밀번호 받기
        {
#pragma region 아이디, 비밀번호 받기
            // 데이터 받기
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

#pragma region 성공 여부 보내기

            login_msg.header = 2; // 로그인 요청 결과 패킷 헤더 2

            strcpy_s(login_msg.msg, req_log.id);
            strcat_s(login_msg.msg, ",");

            bool already_connected = false;

            for (int i = 0; i < Max_Client; i++) // 이미 접속되어 있는지 체크
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

            int msg_length = strlen(login_msg.msg); // 그대로

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

            // 소켓 값, ID 저장
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

#pragma region 전체 메시지 보내기
                timer = time(NULL);
                t = localtime(&timer);

                int year = t->tm_year + 1900;
                int month = t->tm_mon + 1;
                int day = t->tm_mday;
                int hour = t->tm_hour;
                int min = t->tm_min;
                int sec = t->tm_sec;

                msg_packet connect_msg;

                strcpy_s(connect_msg.msg, ""); // 메시지 초기화

                tmp = std::to_string(year);
                strcat_s(connect_msg.msg, tmp.c_str());
                strcat_s(connect_msg.msg, "년 ");

                tmp = std::to_string(month);
                strcat_s(connect_msg.msg, tmp.c_str());
                strcat_s(connect_msg.msg, "월 ");

                tmp = std::to_string(day);
                strcat_s(connect_msg.msg, tmp.c_str());
                strcat_s(connect_msg.msg, "일 ");

                tmp = std::to_string(hour);
                strcat_s(connect_msg.msg, tmp.c_str());
                strcat_s(connect_msg.msg, "시 ");

                tmp = std::to_string(min);
                strcat_s(connect_msg.msg, tmp.c_str());
                strcat_s(connect_msg.msg, "분 ");

                tmp = std::to_string(sec);
                strcat_s(connect_msg.msg, tmp.c_str());
                strcat_s(connect_msg.msg, "초 ");

                strcat_s(connect_msg.msg, req_log.id);
                strcat_s(connect_msg.msg, ", Connected");

                printf("\n[TCP 서버] %s\n", connect_msg.msg);

                connect_msg.header = 3; // 로그인 성공 전체 메시지 헤더 3
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

                        //printf("%d에게 보냄\n", client_sock_list[i]);
                    }
                }

                strcat_s(connect_msg.msg, "\n"); //줄 바꿈 추가
                append_log(connect_msg.msg); // 로그 작성

                login_msg.size = 0;
                connect_msg.size = 0;

#pragma endregion

                state = 1;
            }
        }

        while (state == 1) // 캐릭터 이동 버퍼 받기, 메시지 전달, 프로그램 종료
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

            // 데이터 받기
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

            switch (temp.header) // 헤더로 패킷 분류
            {
            case 4: // 접속종료 요청
                
                printf("\n[TCP 서버] %s, disconnection request success\n", client_id_list[sock_number]);

                strcpy_s(discon_msg.msg, "OK"); // 메시지 설정

                msg_length = strlen(discon_msg.msg);

                discon_msg.header = 5; // 접속 해제 요청 수락 헤더 = 5

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

            case 7: // 캐릭터 이동 명령

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

                // 메시지 설정
                if (send_check == false) strcpy_s(none_id.msg, "일치하는 캐릭터 식별자가 없습니다."); 
                else strcpy_s(none_id.msg, "캐릭터 이동을 완료했습니다.");

                msg_length = strlen(none_id.msg);

                none_id.header = 8; // 이동 명령 완료 header = 8

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

            case 9: //대화메시지 전송

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

                for (int i = 0; i < Max_Client; i++) // 전체 메시지 발송
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
        printf("\n[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // 전체 메시지 전송

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

            discon_global_msg.header = 6; // 접속 해제 전체 알림 = 6

            strcpy_s(discon_global_msg.msg, ""); // 메시지 초기화

            tmp = std::to_string(year);
            strcat_s(discon_global_msg.msg, tmp.c_str());
            strcat_s(discon_global_msg.msg, "년 ");

            tmp = std::to_string(month);
            strcat_s(discon_global_msg.msg, tmp.c_str());
            strcat_s(discon_global_msg.msg, "월 ");

            tmp = std::to_string(day);
            strcat_s(discon_global_msg.msg, tmp.c_str());
            strcat_s(discon_global_msg.msg, "일 ");

            tmp = std::to_string(hour);
            strcat_s(discon_global_msg.msg, tmp.c_str());
            strcat_s(discon_global_msg.msg, "시 ");

            tmp = std::to_string(min);
            strcat_s(discon_global_msg.msg, tmp.c_str());
            strcat_s(discon_global_msg.msg, "분 ");

            tmp = std::to_string(sec);
            strcat_s(discon_global_msg.msg, tmp.c_str());
            strcat_s(discon_global_msg.msg, "초 ");

            strcat_s(discon_global_msg.msg, client_id_list[sock_number]);
            strcat_s(discon_global_msg.msg, ", Disconnected");

            printf("\n[TCP 서버] %s\n", discon_global_msg.msg);

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

            strcat_s(discon_global_msg.msg, "\n"); //줄 바꿈 추가
            append_log(discon_global_msg.msg); // 로그 작성

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

    // 윈속 초기화
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

    // 데이터 통신에 사용할 변수
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

        // 접속한 클라이언트 정보 출력
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // 스레드 생성
        hThread = CreateThread(NULL, 0, ProcessClient,
            (LPVOID)client_sock, 0, NULL);
        if (hThread == NULL) { closesocket(client_sock); }
        else { CloseHandle(hThread); }
    }

    // closesocket()
    closesocket(listen_sock);

    // 윈속 종료
    WSACleanup();
    return 0;
}
