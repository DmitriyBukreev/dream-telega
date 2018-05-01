
struct msg_list {struct msg_list *prev;
	struct msg_list *next;
	struct client_info author;
	struct message msg;
};

struct topic_list {struct topic_list *prev;
	struct topic_list *next;
	struct topics topic;
	struct msg_list *first;
	sem_t sem;
};

struct clients_list {struct clients_list *prev;
	struct clients_list *next;
	struct client_info client;
	struct topic_list topic;
	struct msg_list *last;
	char exited;
	//Закончил сеанс, надо убрать
	pthread_t thread;
};

sem_t topics_list_sem;

void *talk_to_client(void *arg);
//void initialization();

int main(int argc, char **argv)
{
	int descriptor;
	char check;
	struct sockaddr_in server;

	descriptor = socket(AF_INET, SOCK_STREAM, 0);
	IF_ERR(descriptor, -1, "Socket error", exit(errno););
	//Создаем сокет

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htons(INADDR_ANY);
	server.sin_port = htons(1637);
	check = bind(descriptor, (struct sockaddr *) &server, sizeof(server));
	IF_ERR(check, -1, "Bind error", exit(errno););
	//Именуем

	IF_ERR(listen(descriptor, 5), -1, "Listen error", exit(errno););
	while (true) {
		printf("WAITING\n");
		check = accept(descriptor, NULL, 0);
		IF_ERR(check, -1, "Accept error", exit(errno););
	}
	return 0;
}
