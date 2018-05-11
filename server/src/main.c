struct clients_list;

struct msg_list {struct msg_list *prev;
	struct msg_list *next;
	struct msg_signed *msg;
};

struct topic_list {struct topic_list *prev;
	struct topic_list *next;
	struct topics info;
	struct msg_list *first_msg;
	struct clients_list *members;
	int fd;
	sem_t sem;
	sem_t msg_sem;
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
struct topic_list *no_topic;

int change_topic(int needed_index, struct clients_list *client)
//Присоединение к топику
{
	struct topic_list *new;
	//Новый топик
	struct topic_list *search = first_topic;
	//Будем просмотривать список

	IF_ERR(sem_wait(&topics_list_sem), -1, "Sem wait error", exit(errno););
	while (true) {
		if (search->info.index == needed_index) {
			new = search;
			break;
		}
		//Сравниваем индекс с нужным
		if (search->next == first_topic)
			return 1;
		//Если вернулись в начало, а индекса
		//нет, значит что-то не так
		search = search->next;
		//Идем к следующему
	}
	IF_ERR(sem_post(&topics_list_sem), -1, "Sem post error", exit(errno););
	//Нашли нужный

	IF_ERR(sem_wait(&client->topic->sem), -1, "Semwait err", exit(errno););
	if (client->next == client)
		client->topic->members = NULL;
		//Если были там один, можно просто
		//обнулить указатель на себя
	else {
		client->prev->next = client->next;
		client->next->prev = client->prev;
		//Если нет, связываем соседей
	}
	IF_ERR(sem_post(&client->topic->sem), -1, "Sempost err", exit(errno););
	//Удалемся из старого

	client->topic = new;
	//Сменим у клиента

	IF_ERR(sem_wait(&new->sem), -1, "Sem wait error", exit(errno););
	if (new->members == NULL)
		new->members = client;
	else {
		client->prev = new->members->prev;
		client->prev->next = client;
	}
	new->members->prev = client;
	client->next = new->members;
	IF_ERR(sem_post(&new->sem), -1, "Sem wait error", exit(errno););
	//Сунем клиента в новый список
	return 0;
}

void msg_init(struct topic_list *topic)
{
	IF_ERR(lseek(topic->fd, 0, SEEK_END), -1, "Lseek error", exit(errno););
	//Читаем последние с конца

	for (int i = 0; i < 20; i++) {
		unsigned char len, name_len, check;
		struct msg_list *temp = malloc(sizeof(struct msg_list));
		//Выделяем память под сообщение

		IF_ERR(temp, NULL, "Malloc error", exit(errno););

		//Сместимся из конца влево и
		//прочитаем
		check = lseek(topic->fd, 3 + sizeof(long) + sizeof(int), 1);
		//Длина текста, длина имени, байт
		//продолжения, цвет и время
		IF_ERR(check, -1, "Lseek error", exit(errno););
		if (check < (3 + sizeof(long) + sizeof(int)))
			break;
		//Дошли до начала

		check = read(topic->fd, &len, 1);
		//Считаем длину сообщения
		IF_ERR(check, -1, "Read error", exit(errno););

		check = read(topic->fd, &name_len, 1);
		//Считаем длину имени отправителя
		IF_ERR(check, -1, "Read error", exit(errno););

		check = read(topic->fd, &temp->msg->msg.time, sizeof(long));
		//Считаем время сообщения
		IF_ERR(check, -1, "Read error", exit(errno););

		check = read(topic->fd, &temp->msg->author.color, sizeof(int));
		//Считаем цвет отправителя
		IF_ERR(check, -1, "Read error", exit(errno););

		check = read(topic->fd, &temp->msg->msg.continues, 1);
		//Считаем байт продолжения
		IF_ERR(check, -1, "Read error", exit(errno););

		check = lseek(topic->fd, 3 + sizeof(long) + sizeof(int), 1);
		//Вернемся обратно
		IF_ERR(check, -1, "Lseek error", exit(errno););

		len++;
		name_len++;

		check = lseek(topic->fd, len + name_len, 1);
		//Сдвинемся к началу сообщения
		IF_ERR(check, -1, "Lseek error", exit(errno););

		check = read(topic->fd, temp->msg->msg.text, len);
		IF_ERR(check, -1, "Read error", exit(errno););
		temp->msg->msg.text[len] = 0;
		//Считали текст и обнулили конец

		check = read(topic->fd, temp->msg->author.name, name_len);
		IF_ERR(check, -1, "Read error", exit(errno););
		temp->msg->author.name[name_len] = 0;
		//Считали имя и обнулили конец

		check = lseek(topic->fd, len + name_len, 1);
		//На следующем цикле начнем здесь
		IF_ERR(check, -1, "Lseek error", exit(errno););

		if (topic->first_msg == NULL)
			topic->first_msg = temp;
		else {
			temp->prev = topic->first_msg->prev;
			temp->prev->next = temp;
		}
		topic->first_msg->prev = temp;
		temp->next = topic->first_msg;
	}
}

void *talk_to_client(void *arg)
{
	struct clients_list *temp = arg;
	char check, code;

	temp->topic = no_topic;
	//Топика нет
	temp->exited = false;
	//Сеанс не закончил
	IF_ERR(sem_wait(&no_topic->sem), -1, "Sem wait error", exit(errno););
	//Приступаем к редактированию списка
	//клиентов, не имеющих топика

	if (no_topic->members == NULL)
		no_topic->members = temp;
	else {
		temp->prev = no_topic->members->prev;
		temp->prev->next = temp;
	}
	no_topic->members->prev = temp;
	temp->next = no_topic->members;

	IF_ERR(sem_post(&no_topic->sem), -1, "Sem post error", exit(errno););
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
			check = sem_init(&temp->msg_sem, 0, 1);
			IF_ERR(check, -1, "Sem_init error", exit(errno););
			//Семафоры, чтобы менять
			//что-то мог одновременно
			//только один процесс

			temp->info.index = temp->prev->info.index + 1;

			strncpy(temp->info.topic, temp_f->d_name, 256);
			temp->info.topic[256] = 0;
			//Копируем имя из файла и
			//добавляем нулевой байт

			temp->fd = open(temp->info.topic, O_RDONLY);
			//Заодно отловим лишние
			//файлы в директории
			IF_ERR(temp->fd, -1, "Open error", exit(errno););

			check = read(temp->fd, &len, 1);
			//Считаем длину имени автора
			IF_ERR(check, -1, "Read error", exit(errno););
			len++;
			//Там записано вот так

			check = read(temp->fd, temp->info.starter.name, len);
			//Читаем имя
			IF_ERR(check, -1, "Read error", exit(errno););
		}
	}

	//Создадим список
	//клиентов, у которых еще нет топика
	no_topic->prev = NULL;
	no_topic->next = NULL;
	no_topic->first_msg = NULL;
	no_topic->members = NULL;
	//Он в списке один, сообщений нет,
	//клиентов еще нет
	check = sem_init(&no_topic->sem, 0, 1);
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
