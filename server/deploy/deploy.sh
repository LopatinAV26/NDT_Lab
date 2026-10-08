#!/bin/bash
# Выложить сервер синхронизации из текущего коммита (Linux):
#   ./server/deploy/deploy.sh [хост из ~/.ssh/config, по умолчанию ndtlab]
# На сервер уходит только закоммиченное: незакоммиченные правки в server/ останавливают выкладку
set -e

host="${1:-ndtlab}"
cd "$(git -C "$(dirname "$0")" rev-parse --show-toplevel)"

if [ -n "$(git status --porcelain -- server)" ]; then
    echo "В server/ есть незакоммиченные правки - закоммитьте их или отмените:"
    git status --short -- server
    exit 1
fi

commit=$(git rev-parse --short HEAD)
archive=$(mktemp --suffix=.tar)
trap 'rm -f "$archive"' EXIT

git archive -o "$archive" HEAD server
scp -q "$archive" "$host:/tmp/ndtlab-server.tar"
ssh "$host" "rm -rf /tmp/ndtlab-deploy && mkdir /tmp/ndtlab-deploy && tar -xmf /tmp/ndtlab-server.tar -C /tmp/ndtlab-deploy && rm /tmp/ndtlab-server.tar && bash /tmp/ndtlab-deploy/server/deploy/remote-update.sh $commit"
