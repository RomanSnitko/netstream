# Gateway - Центральный сервер обработки событий

## Зона ответственности
Gateway принимает события от агентов по TCP, парсит, валидирует, нормализует, классифицирует 
и прогоняет через rule engine. Публикует события и алерты в Kafka.

## Что делает
1. **Принимает TCP соединения** - от множества агентов одновременно
2. **Парсит length-prefixed протокол** - читает `[4 bytes length][json data]`
3. **Валидирует JSON** - проверяет структуру и обязательные поля
4. **Нормализует события** - приводит к единому формату
5. **Классифицирует** - определяет категорию события (auth, kernel, network, etc)
6. **Rule Engine** - проверяет правила и генерирует алерты
7. **Публикует в Kafka** - события в `netstream.events`, алерты в `netstream.alerts`
8. **Метрики** - количество принятых событий, ошибок, активных соединений

## Архитектура
- **Gateway** - основной класс, координирует компоненты
- **TcpServer** - принимает соединения, парсит фреймы
- **RuleEngine** - проверяет правила и создает алерты
- **KafkaProducer** - публикует события в топики

## Обработка события
```
TCP → Frame Parser → JSON Parser → Validator → Normalizer → Classifier → Rule Engine → Kafka
```

## Правила (примеры)
- `Failed password` → алерт "Failed authentication"
- `kernel: Out of memory` → алерт "OOM detected"
- `sudo:` → категория "auth"



Поток выполнения TcpServer:

  1. Конструктор:
     - Создает acceptor
     - Привязывается к порту
     - Начинает слушать

  2. run():
     - running = true
     - start_accept() → регистрирует async_accept
     - io_context.run() → блокируется, обрабатывает события

  3. Клиент подключается:
     - io_context вызывает accept callback
     - handle_client(socket) → создает shared_ptr
     - async_read_length(socket) → регистрирует чтение 4 байт
     - start_accept() → принимаем следующего клиента

  4. Клиент отправляет данные:
     - io_context вызывает read callback
     - Парсим length
     - async_read_data(socket, length) → регистрирует чтение N байт

  5. Данные прочитаны:
     - io_context вызывает read callback
     - on_message(json) → обрабатываем сообщение
     - async_read_length(socket) → читаем следующее сообщение

  6. shutdown():
     - running = false
     - Закрывает acceptor
     - io_context.stop() → завершает run()

