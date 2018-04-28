#define LIST 1
//Передача списка топиков
#define MSG 2
//Передача сообщения
#define INFO_ABOUT 3
//Передача информации о пользователе
#define JOIN 4
//Присоединиться к топику
#define GTFO 5
//"Извините, вы в черном списке"
#define CREATE 6
//Создать топик
#define GET_LAST_MSGS 7
//Передача сервером последних сообщений
#define LEAVE 8
//Отсоединение клиента
#define ERROR 9
//Кто-то сломался
#define MEMBERS_LIST 10

//Эти сигналы будут интерпретироваться
//По-разному, в зависимости от того,
//Получены они сервером или клиентом

#define TEXT_SIZE 257
//256 символов и 0

struct message {char text[TEXT_SIZE]; char continues;};
//Большое сообщение можно передать за
//Несколько раз. Continues == 1 значит
//Продолжение есть

struct client_info {char name[TEXT_SIZE];};
//Информация о клиенте

struct topics {int index; char topic[TEXT_SIZE]; char next;};
//Список передается поэлементно

struct incoming_msg {struct client_info auth; char text[TEXT_SIZE]; char last;};
//То, что сервер отправляет участникам топика


