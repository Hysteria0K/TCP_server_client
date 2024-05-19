#define ID_SIZE 10
#define PW_SIZE 12
#define MSG_SIZE 256


class packet_init
{
	public:
		int size = 0;
		int header = 0;
};


class req_log_packet: public packet_init
{
	public:
		char id[ID_SIZE] = {};
		char password[PW_SIZE] = {};

};

class msg_packet : public packet_init
{
	public:
		char msg[MSG_SIZE] = {};
		int success = 0;
};

class client_character : public packet_init
{
	public:
		char id[ID_SIZE] = {};
		int x = 0;
		int y = 0;
		int z = 0;
};

class ch_move_packet : public client_character
{
	public:
		char msg[MSG_SIZE] = {};
};

typedef struct saved_mes
{
	char msg[MSG_SIZE] = {};

}saved_mes;