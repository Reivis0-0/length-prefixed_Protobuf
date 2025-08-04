> [!ТЗ]
>  **<mark style="background: #FFB8EBA6;">Описание</mark>:**
>  Разработать функцию и класс (и модульные тесты к ним) по требованиям ниже, которые будут делать разбор потока prefixed Protobuf сообщений
>  

<mark style="background: #FFB8EBA6;">Протокол</mark>
```
syntax = "proto2";

package TestTask.Messages;

message WrapperMessage {
	optional FastResponse fast_response = 1;
	optional SlowResponse slow_response = 2;
	optional RequestForFastResponse request_for_fast_response = 3;
	optional RequestForSlowResponse request_for_slow_response = 4;
}

message FastResponse {
	required string current_date_time = 1;
}

message SlowResponse {
	required uint32 connected_client_count = 1;
}

message RequestForFastResponse {

}

message RequestForSlowResponse {
	required uint32 time_in_seconds_to_sleep = 1;
}
```

>[!Warning]
>Особенности разбора сообщений Protobuf
> - каждое сообщение будет предварять его размер (length-prefixed). Для размера должен быть использован тип [Varint32](https://developers.google.com/protocol-buffers/docs/techniques#streaming)
> - Использование всегда будет WrapperMessage(заполнено только одно из функциональных полей - только так можно понять при разборе какое именно у нас сообщение)
> - Время должно быть в ISO формате: YYYYMMDDThhmmss.fff например: 19851019Т050170.333 - 19 октября 1985 года 5 часов 1 минута 7 секунд 33 миллисекунды 
> - Следует обратить внимание, что при разборе сообщений необходимо учесть, что в общем случае может поступить на вход любая часть сообщения, несколько сообщений сразу - в общем любой кусок потока (нельзя даже рассчитывать, что предваряющий размер сообщения  поступит полностью). Поэтому необходимо при разборе обязательно использовать буфер

#### Описание требований к API
###### Интерфейс функции разбора одного сообщения из потока
```
/*!

	* \brief Расшифровывает сообщение, предваренное длиной из массива байтов.
	*
	* \tparam Message Тип сообщения, для работы с которым предназначена данная функция.
	*
	* \param data Указатель на буфер данных.
	* \param size Размер буфера данных.
	* \param bytesConsumed Количество байт, которое потребовалось для расшифровки сообщения в случае успеха.
	*
	* \return Умный указатель на сообщение. Если удалось расшифровать сообщение, то он не пустой.
	*/
template<typename Message>
std::shared_ptr<Message> parseDelimited(constvoid* data, size_t size, size_t* bytesConsumed = 0);
```
Эта функция должна возвращать первое сообщение из буфера data, если оно там есть

###### Интерфейс класса разбора потока сообщений
```
template<typename MessageType>
class DelimitedMessagesStreamParser
{
	public:
	typedef std::shared_ptr<const MessageType> PointerToConstValue;

	std::list<PointerToConstValue> parse(const std::string& data);

	private:
std::vector<char> m_buffer;
};
```
Этот класс должен возвращать список сообщений из строки data
Он должен использовать в реализации функции parse() функцию parseDelimited()

###### Вспомогательная функция для сериализации сообщений в поток байт (нужна для тестов)
```
#if GOOGLE_PROTOBUF_VERSION >= 3012004
#define PROTOBUF_MESSAGE_BYTE_SIZE(message) ((message).ByteSizeLong())
#else
#define PROTOBUF_MESSAGE_BYTE_SIZE(message) ((message).ByteSize())
#endif
typedef std::vector<char> Data;
typedef std::shared_ptr<const Data> PointerToConstData;
typedef std::shared_ptr<Data> PointerToData;
template <typename Message> PointerToConstData serializeDelimited(const Message& msg)
{
	constsize_t messageSize = PROTOBUF_MESSAGE_BYTE_SIZE(msg);
	constsize_t headerSize = google::protobuf::io::CodedOutputStream::VarintSize32(messageSize);

	const PointerToData& result = std::make_shared<Data>(headerSize + messageSize);
	google::protobuf::uint8* buffer = reinterpret_cast<google::protobuf::uint8*>(&*result->begin());
	google::protobuf::io::CodedOutputStream::WriteVarint32ToArray(messageSize, buffer);
	msg.SerializeWithCachedSizesToArray(buffer + headerSize);

	return result;
}
```
###### Пример использования объекта класса DelimitedMessagesStreamParser
```
std::vector<char> messages;
// тут код заполнения messages с помощью  serializeDelimited
typedef DelimitedMessagesStreamParser<WrapperMessage> Parser;
Parser parser;

// идем по одному байту по входному потоку сообщений
for(constchar byte, messages)
{
	const std::list<PointerToConstValue>& parsedMessages = parser.parse(std::string(1, byte));
	for(const PointerToConstValue& value, parsedMessages)
	{
		// добавляем куда-то все сообщения
	}
}

// тут код проверки, что все сообщения расшифровались верно
```

>[!Warning]
>- Приложение должно быть написано с использованием [соглашения о стиле (c++)](http://wiki.nic.etu/docuwikki/doku.php/languages:cpp:code_style:start "Пройти по ссылке"), принятом в НИЦ СПб ЭТУ
>- На функцию parseDelimited() и класс DelimitedMessagesStreamParser должны быть написаны модульные тесты с помощью Google Test
>- Проект должен собираться с помощью CMake
>- В исходниках не должно быть генерированных файлов - генерировать код из протокола нужно при сборке с помощью [функций cmake](https://cmake.org/cmake/help/latest/module/FindProtobuf.html "Пройти по ссылке")
>- Реализация должна обрабатывать любое Protobuf-сообщение(типа, переданного в шаблонном параметре), протокол в данном задании приведен для тестов и для общего понимания задачи


# Разработка
Реализовать систему для разбора потока Protobuf-сообщений в формате length-prefixed.
1. Формат данных
- Каждое сообщение предваряется [[Varint32]] - переменной длиной (1 - 5 байт), указывающей резмер сообщения
- Сообщения используют формат
```
WrapperMessage {
  FastResponse | SlowResponse | RequestForFastResponse | RequestForSlowResponse
}
```
- Формат времени: `YYYYMMDDThhmmss.fff` (например, `19851019T050107.333`)
- Данные могут приходить частями (неполными фрагментами)
2. Функция parserDelimited:
- Назначение: извлечь одно сообщение из буфера данных
- Поведение:
	- Анализирует начало буфера для поиска Varint-заголовка
	- Если данных недостаточно для полного сообщения -> возвращает nullptr
	- Если сообщение полное -> парсит его и возвращает shared_ptr
	- Обновляет bytesConsumed (сколько байт было обработано)
- Требования к обработке ошибок:
	- Некорректный Varint -> возврат nullptr
	- Несоответствие размера сообщения -> возврат nullptr
	- ошибка парсинга Prototbuf -> возврат nullptr
3. **Класс `DelimitedMessagesStreamParser`:**
- **Назначение:** Обрабатывать **поток** данных с частичными сообщениями
- **Механизм работы:**
    - Добавляет новые данные во внутренний буфер (`m_buffer`)
    -  Пытается извлечь все возможные сообщения из буфера
    - Удаляет обработанные данные из буфера
    - Возвращает список распарсенных сообщений
- **Ключевые особенности:**
    - Должен корректно обрабатывать случаи, когда:
        - Сообщение разбито на несколько частей
        - В буфере несколько сообщений подряд
        - Varint разбит между фрагментами данных
4. Модульные тесты Google Test
- **Что тестировать:**
    - Разбор полного сообщения
    - Обработка частичных данных (по 1 байту)
    - Последовательность из нескольких сообщений
    - Все типы сообщений (Fast/Slow/Request)
    - Корректность формата времени
    - Обработка поврежденных данных
    - Пограничные случаи (пустые данные, неполный Varint)
5. Требования к сборке
- Использовать **CMake**
- Генерация кода Protobuf **во время сборки**
- Отсутствие сгенерированных файлов в репозитории
- Соответствие стилю кодирования НИЦ СПб ЭТУ

#### Материалы
**1. Length-Prefixed Protobuf:**
- [Официальная документация](https://developers.google.com/protocol-buffers/docs/techniques#streaming)
- [Практические примеры streaming](https://cwiki.apache.org/confluence/display/GEODE/Delimiting+Protobuf+Messages)
    
**2. Кодирование Varint:**
- [Подробное объяснение формата](https://developers.google.com/protocol-buffers/docs/encoding#varints)
- [Реализация на C++](https://github.com/protocolbuffers/protobuf/blob/main/src/google/protobuf/io/coded_stream.h)

**3. Работа с Protobuf в C++:**
- [Парсинг из сырых байтов](https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.message_lite#ParseFromArray)
- [Официальное руководство](https://protobuf.dev/reference/cpp/cpp-generated/)

**4. Модульное тестирование:**
- [Google Test для начинающих](https://google.github.io/googletest/primer.html)
- [Тестирование Protobuf](https://protobuf.dev/reference/cpp/phpunit/)
    
**5. Интеграция Protobuf с CMake:**
- [Официальный пример](https://github.com/protocolbuffers/protobuf/tree/main/examples)
- [CMake + Protobuf туториал](https://cmake.org/cmake/help/latest/module/FindProtobuf.html)
    
**6. Форматы времени:**
- [ISO 8601 форматы](https://en.wikipedia.org/wiki/ISO_8601)
- [Реализации работы со временем в C++](https://en.cppreference.com/w/cpp/chrono)

#### Критические моменты реализации

1. **Обработка Varint:**
    - Реализуйте функцию для разбора Varint с проверкой на неполные данные
    - Учитывайте, что Varint может занимать 1-5 байт
2. **Буферизация данных:**
    - В классе `DelimitedMessagesStreamParser` используйте кольцевой буфер или `std::vector` с эффективным удалением обработанных данных
    - Оптимизируйте копирование данных
3. **Обработка частичных данных:**
    - Реализуйте state machine для трех состояний:
        - Ожидание заголовка (Varint)
        - Ожидание тела сообщения
        - Готовое сообщение
4. **Формат времени:**
    - Для генерации времени используйте библиотеку `<chrono>`
    - Пример преобразования:
        cpp
        Copy
        Download
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y%m%dT%H%M%S") << "." 
            << std::setfill('0') << std::setw(3) 
            << (now.time_since_epoch().count() % 1000);
        
5. **Оптимизация производительности:**
    - Избегайте лишнего копирования данных
    - Используйте move-семантику
    - Для больших сообщений предусмотрите ограничение размера буфера

#### Работа

1. **Настройка окружения:**
    - Установите protobuf-compiler и libprotobuf-dev
    - Настройте CMake проект с генерацией кода из .proto файла
2. **Реализация ядра:**
    - Создайте функцию для разбора Varint
    - Реализуйте `parseDelimited`
    - Протестируйте на полных сообщениях
3. **Потоковый парсер:**
    - Реализуйте буферизацию в классе
    - Добавьте обработку частичных данных
    - Протестируйте с фрагментированными сообщениями
4. **Тестирование:**
    - Напишите тесты для всех типов сообщений
    - Реализуйте тесты с поврежденными данными
    - Проверьте обработку пограничных случае        
5. **Оптимизация:**
    - Проведите нагрузочное тестирование
    - Убедитесь в отсутствии утечек памяти
    - Проверьте корректность формата времени
  

**Функция parseDelimitd**
 ```
 std::shared_ptr<Message> parseDelimited(constvoid* data, size_t size, size_t* bytesConsumed = 0);

data == nullptr -> nullptr
declaration messageSize;
declaration CodeInputStream (cast(uint8) <- data, size);
инициализировать позицию методом CurrentPosition

если не возможно считать Varint32 или траблы с размером
	присваить значению обработанных байтов 0

Устанавливаем ограничения на размер(распарсили до этого)
создаем указатель
пытаемся распарсить ParseFromCodeStream
- ошибка -> исключение
убираем лимит
если передан не пустой указатель на обработанные байты, присваиваем ему количество обраотанных байтов
возвращаем результат распарсенной строки
```
