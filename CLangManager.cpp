#include "CLangManager.h"
#include "CSetting.h"

CLangManager LangManager;
CLangManager *gLangManager = &LangManager;

map<const char*, const char*, CPrtCmp> LangUa = 
{
	{"Server management",				u8"Управління сервером"},
	{"Status: Disabled",				u8"Статус: Вимкнений"},
	{"Status: Enabled",					u8"Статус: Увімкнений"},
	{"Status: Reboot",					u8"Статус: Перезагрузка"},
	{"Map",								u8"Мапа"},
	{"Players",							u8"Гравці"},
	{"Server ID",						u8"ID Сервера"},
	{"Days until the end of the lease",	u8"Днів до закінчення оренди"},
	{"Management",						u8"Управління"},
	{"Start",							u8"Старт"},
	{"Stop",							u8"Стоп"},
	{"Restart",							u8"Рестарт"},
	{"Go to the server",				u8"Зайти на сервер"},
	{"Change map",						u8"Змінити карту"},
	{"Player name",						u8"Ім'я гравця"},
	{"Frags",							u8"Фраги"},
	{"Time in game",					u8"Час у грі"},
	{"The server is already enabled",	u8"Сервер уже увімкнений"},
	{"The server reboots",				u8"Сервер перезагружається"},
	{"The server is already down",		u8"Сервер уже вимкнений"},
	{"The server is down",				u8"Сервер вимкнений"},
	{"The server is already rebooting",	u8"Сервер уже перезагружається"},

	{"Server console",					u8"Консоль сервера"},
	{"Enter the command",				u8"Ввести команду"},

	{"File",							u8"Файл"},
	{"Settings",						u8"Настройки"},
	{"Exit",							u8"Вихід"},

	{"Color theme",						u8"Кольорова тема"},
	{"Dark theme",						u8"Темна"},
	{"Bright theme",					u8"Світла"},
	{"Classic theme",					u8"Класична"},
	{"Font size",						u8"Розмір шрифту"},
	{"Change the data update time",		u8"Змінити час оновлення даних"},
	{"Console",							u8"Консоль"},
	{"Server information",				u8"Інформація про сервер"},
	{"Ukrainian",						u8"Українська"},
	{"English",							u8"English"},
	{"Russian",							u8"Руская"},
	{"Language",						u8"Мова"},
	
	{"Enter your server token",			u8"Введіть свій токен від сервера"},
	{"Save token",						u8"Зберегти токен"},
	{"Not recommended for public computers!!!",		u8"Не Рекомендовано для публічних компютерів!!!"},
	{"Connection error, try again later!",			u8"Ошибка зєднання, спробуйте пізніше!"},
	{"Incorrect answer!",				u8"Некоректна відповідь!"},
	{"Correctly led token!",			u8"Коректно ведений токен!"},
	{"Incorrect token entered!",		u8"Неправильно введений токен!"},

	{"Version of the components used in the program:", u8"Версія компонентів,\r\nщо використовуються в програмі:"},
	{"About",							u8"Про програму"},
	{"Help",							u8"Справка"},
};
map<const char*, const char*, CPrtCmp> LangRu = 
{	
	{"Server management",				u8"Управление сервером"},
	{"Status: Disabled",				u8"Статус: Отключен"},
	{"Status: Enabled",					u8"Статус: Включен"},
	{"Status: Reboot",					u8"Статус: Перезагрузка"},
	{"Map",								u8"Карта"},
	{"Players",							u8"Игроки"},
	{"Server ID",						u8"ID Сервера"},
	{"Days until the end of the lease",	u8"Дней до окончания аренды"},
	{"Management",						u8"Управление"},
	{"Start",							u8"Старт"},
	{"Stop",							u8"Стоп"},
	{"Restart",							u8"Рестарт"},
	{"Go to the server",				u8"Зайти на сервер"},
	{"Change map",						u8"Изменить карту"},
	{"Player name",						u8"Ник"},
	{"Frags",							u8"Фраги"},
	{"Time in game",					u8"Время в игре"},
	{"The server is already enabled",	u8"Сервер уже включен"},
	{"The server reboots",				u8"Сервер перезагружается"},
	{"The server is already down",		u8"Сервер уже выключен"},
	{"The server is down",				u8"Сервер выключен"},
	{"The server is already rebooting",	u8"Сервер уже перезагружается"},

	{"Server console",					u8"Консоль сервера"},
	{"Enter the command",				u8"Ввести команду"},

	{"File",							u8"Файл"},
	{"Settings",						u8"Настройки"},
	{"Exit",							u8"Выход"},

	{"Color theme",						u8"Цветовая тема"},
	{"Dark theme",						u8"Темная"},
	{"Bright theme",					u8"Светлая"},
	{"Classic theme",					u8"Классическая"},
	{"Font size",						u8"Размер шрифта"},
	{"Change the data update time",		u8"Изменить время обновления данных"},
	{"Console",							u8"Консоль"},
	{"Server information",				u8"Информация о сервере"},
	{"Ukrainian",						u8"Українська"},
	{"Russian",							u8"Руская"},
	{"English",							u8"English"},
	{"Language",						u8"Язык"},
	
	{"Enter your server token",			u8"Введите токен от сервера"},
	{"Save token",						u8"Сохранить токен"},
	{"Not recommended for public computers!!!",		u8"Не рекомендуется для публичных компьютеров!!!"},
	{"Connection error, try again later!",			u8"Ошибка соединения, попробуйте позже!"},
	{"Incorrect answer!",				u8"Некорректный ответ!"},
	{"Correctly led token!",			u8"Правильно веденный токен!"},
	{"Incorrect token entered!",		u8"Введен неправильный токен!"},

	{"Version of the components used in the program:", u8"Версия компонентов,\r\nиспользуемых в программе:"},
	{"About",							u8"О програме"},
	{"Help",							u8"Справка"},
};

const char *CLangManager::GetLang(const char *str) const
{
	switch (g_pGlob->enumLang)
	{
	case LANG::UA:
		return LangUa[str];
	case LANG::RU:
		return LangRu[str];
	case LANG::EN:
		return str;
	}
	return '\0';
}
