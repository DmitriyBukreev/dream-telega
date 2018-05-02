struct clients_list;

struct msg_list {struct msg_list *prev;
	struct msg_list *next;
	struct client_info author;
	struct message msg;
};

struct topic_list {struct topic_list *prev;
	struct topic_list *next;
	struct topics info;
	struct msg_list *first_msg;
	struct clients_list *members;
	int file;
	sem_t sem;
};

struct clients_list {struct clients_list *prev;
	struct clients_list *next;
	struct client_info client;
	struct topic_list *topic;
	char exited;
	int fd;
	//Закончил сеанс, надо убрать
	pthread_t thr;
};

sem_t topics_list_sem;
struct topic_list *first_topic;
struct clients_list *no_topic;
sem_t no_topic_sem;

void *talk_to_client(void *arg)
{
	struct clients_list *temp = arg;
	char check, code;

	temp->topic = NULL;
	//Топика нет
	temp->exited = false;
	//Сеанс не закончил
	sem_wait(&no_topic_sem);
	//Приступаем к редактированию списка
	//клинетов, не имеющих топика

	temp->next = no_topic;
	temp->prev = no_topic->prev;
	temp->prev->next = temp;
	no_topic->prev = temp;

	sem_post(&no_topic_sem);
	//Закончили редактировать

	IF_ERR(read(temp->fd, &code, 1), -1, "Socket read error", exit(errno););
	//Хотим считать информацию о клиенте
	if (code != INFO_ABOUT)  {
	//Если он прислал не то, больше
	//с ним не разговариваем
		temp->exited = true;
		//Говорим остальным, что мы все
		code = ERROR;
		write(temp->fd, &code, 1);
		//Говорим клиенту об ошибке
		IF_ERR(close(temp->fd), -1, "Socket close error", exit(errno););
		//Закрываем его сокет
		return NULL;
		//Выходим
	}
	check = read(temp->fd, &temp->client, sizeof(struct client_info));
	IF_ERR(check, -1, "Socket read error", exit(errno););
	return NULL;
}
void initialization(void)
{
	DIR *dir;
	struct topic_list *temp = NULL;
	char check;

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
			char len;

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
	check = sem_init(&no_topic_sem, 0, 1);
	IF_ERR(check, -1, "Sem_init error", exit(errno););
}

int main(int argc, char **argv)
{
	int descriptor;
	int check;
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
		struct clients_list *temp;

		printf("WAITING\n");
		check = accept(descriptor, NULL, 0);
		IF_ERR(check, -1, "Accept error", exit(errno););

		temp = malloc(sizeof(struct clients_list));
		IF_ERR(temp, NULL, "Malloc error", exit(errno););
		//Память под запись о клиенте

		temp->fd = check;
		//Даем дескриптор
		pthread_create(&temp->thr, NULL, talk_to_client, temp);
		//Поток обрабатывает клиента
	}
	exit(EXIT_SUCCESS);
}
