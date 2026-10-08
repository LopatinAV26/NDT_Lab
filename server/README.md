# Сервер синхронизации NDT Lab (`ndtsync`)

Хранит записи программы и прикреплённые файлы. Компьютеры лаборатории отправляют свои изменения
и забирают чужие. Сервер — C++20: cpp-httplib, libpqxx, nlohmann/json, PostgreSQL.

Рабочий сервер — Ubuntu 24.04. Служба слушает только `127.0.0.1:8080`, снаружи к ней обращаются
через SSH-туннель (позже — через nginx с HTTPS).

## Запросы

Все, кроме `/health`, требуют заголовок `Authorization: Bearer <токен компьютера>`.

| Запрос | Что делает |
|---|---|
| `GET /health` | проверка: сервер и база работают |
| `GET /whoami` | какой компьютер владеет токеном |
| `POST /sync/push` | принять пачку изменённых записей `{"records": [...]}` |
| `GET /sync/pull?since=N&limit=M` | записи с номером версии больше N |
| `PUT /files/{id}?name=...` | загрузить файл, `Content-Type: application/octet-stream` |
| `GET /files/{id}` | скачать файл; имя — в заголовке `X-File-Name` (%XX) |

## Разработка на Arch Linux

```bash
sudo pacman -S base-devel cmake postgresql libpqxx nlohmann-json cpp-httplib

# PostgreSQL - один раз
sudo -u postgres initdb -D /var/lib/postgres/data
sudo systemctl enable --now postgresql
sudo -u postgres createuser "$USER"
sudo -u postgres createdb -O "$USER" ndtlab

# сборка и запуск
cd server
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
PGDATABASE=ndtlab ./build/ndtsync serve

# в другом терминале
curl http://127.0.0.1:8080/health
PGDATABASE=ndtlab ./build/ndtsync add-device "Ноутбук"
```

Подключение к базе — стандартными переменными `PGHOST`, `PGDATABASE`, `PGUSER`, `PGPASSWORD`.
На рабочем сервере их задаёт `/etc/ndtlab/db.env`.

На Ubuntu вместо `pacman`:
`sudo apt install build-essential cmake postgresql libpqxx-dev nlohmann-json3-dev libcpp-httplib-dev`.

## Выкладка на сервер

Файл для сервера собирается на самом сервере: в Arch библиотеки новее, чем в Ubuntu 24.04,
и собранная на ноутбуке программа там не запустится. Выкладывается только закоммиченное:

```bash
./server/deploy/deploy.sh          # Linux
```
```powershell
.\server\deploy\deploy.ps1         # Windows
```

Скрипт упаковывает `server/` из текущего коммита, копирует на сервер, собирает, перезапускает службу
и проверяет `/health`. Хэш выложенного коммита — в `~/ndtlab-server/COMMIT` на сервере.

Нужен доступ `ssh ndtlab` (хост в `~/.ssh/config`).

## Администрирование на сервере

```bash
sudo ndtsync add-device "Ноутбук Лопатина"   # выдаёт токен один раз
sudo ndtsync list-devices
sudo ndtsync revoke-device <id>
sudo journalctl -u ndtsync -f                 # журнал запросов
ls /var/backups/ndtlab                        # ежедневные выгрузки базы, 14 дней
```

Подключение к туннелю с рабочего компьютера: `ssh -N -L 8080:127.0.0.1:8080 ndtlab`,
после чего сервер доступен как `http://127.0.0.1:8080`.
