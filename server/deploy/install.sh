#!/bin/bash
# Собрать и (пере)установить сервер синхронизации. Запускать на сервере из папки с исходниками:
#   ./deploy/install.sh
# Нужны пакеты: build-essential cmake libpqxx-dev nlohmann-json3-dev libcpp-httplib-dev
set -e
cd "$(dirname "$0")/.."

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j1 # одно ядро и 1 ГБ памяти - параллельная сборка только упрётся в подкачку

id ndtsync >/dev/null 2>&1 || sudo useradd --system --no-create-home --shell /usr/sbin/nologin ndtsync

sudo install -m 755 build/ndtsync /usr/local/bin/ndtsync
sudo install -m 644 deploy/ndtsync.service /etc/systemd/system/ndtsync.service
sudo systemctl daemon-reload
sudo systemctl enable ndtsync >/dev/null 2>&1
sudo systemctl restart ndtsync

sleep 1
systemctl --no-pager --lines=5 status ndtsync
