
struct msg_list {struct msg_list *prev;
	struct msg_list *next;
	struct client_info author;
	struct message msg;
};

struct topic_list {struct topic_list *prev;
	struct topic_list *next;
	struct topics info;
	struct msg_list *first_msg;
	int file;
	sem_t sem;
};

struct clients_list {struct clients_list *prev;
	struct clients_list *next;
	struct client_info client;
	struct topic_list topic;
	struct msg_list *last_msg;
	char exited;
	//Закончил сеанс, надо убрать
	pthread_t thread;
};

sem_t topics_list_sem;
struct topic_list *first_topic;

void *talk_to_client(void *arg);
void initialization(void)
{
	DIR *dir;
	struct topic_list *temp = NULL;

	first_topic = NULL;
	//Это указатель на список

	dir = opendir("topics");
	//Открываем текущую траекторию
	IF_ERR(dir, NULL, "Dirent error", exit(errno););

	while (true) {
		struct dirent *temp_f;

		temp_f = readdir(dir);
		//Читаем очередной файл
		if (temp_f == NULL) {
			IF_ERR(!errno, 0, "Readdir error", exit(errno););
			break;
			//Ошибка или конец
		} else {
			char check, len;

			if (!strcmp(temp_f->d_name, "."))
				continue;
			if (!strcmp(temp_f->d_name, ".."))
				continue;
			//Пропускаем вот это

			temp = malloc(sizeof(struct topic_list));
			//Выделяем память под
			//очередной элемент
			IF_ERR(temp, NULL, "Malloc error", exit(errno););

			if (first_topic == NULL) {
			//Если это первый топик
				first_topic = temp;
				//То он еще и дескриптор
				temp->info.index = -1;
				//Потом будет 0
			} else {
			//Если не первый
				temp->prev = first_topic->prev;
				//Делаем в текущем
				//ссылку на предыдущий
				temp->prev->next = temp;
			}
			first_topic->prev = temp;
			//В первом ссылку на этот
			temp->next = first_topic;
			//А в текущем на первый
			//Список замкнулся
			temp->first_msg = NULL;
			//Потом считаем, если надо

			check = sem_init(&temp->sem, 0, 1);
			IF_ERR(check, -1, "Sem_init error", exit(errno););
			//Семафор, чтобы менять
			//что-то мог одновременно
			//только один процесс

			temp->info.index = temp->prev->info.index + 1;

			strncpy(temp->info.topic, temp_f->d_name, 256);
			temp->info.topic[256] = 0;
			//Копируем имя из файла и
			//добавляем нулевой байт

			temp->file = open(temp->info.topic, O_RDONLY);
			//Заодно отловим лишние
			//файлы в директории
			IF_ERR(temp->file, -1, "Open error", exit(errno););

			check = read(temp->file, &len, 1);
			//Считаем длину имени автора
			IF_ERR(check, -1, "Read error", exit(errno););
			len++;
			//Там записано вот так

			check = read(temp->file, temp->info.starter.name, len);
			//Читаем имя
			IF_ERR(check, -1, "Read error", exit(errno););
		}
	}
}

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

	initialization();
	IF_ERR(listen(descriptor, 5), -1, "Listen error", exit(errno););
	while (true) {
		printf("WAITING\n");
		check = accept(descriptor, NULL, 0);
		IF_ERR(check, -1, "Accept error", exit(errno););
	}
	exit(EXIT_SUCCESS);
}
