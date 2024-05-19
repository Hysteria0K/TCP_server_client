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

class id_check
{
	public:
		char id[ID_SIZE] = {};

};

class send_msg_packet : public msg_packet
{
	public:
		char id[ID_SIZE] = {};
		char id_msg[MSG_SIZE] = {};
};