#!/bin/bash
# Выполняется на сервере - его запускают deploy.sh и deploy.ps1 из распакованного архива.
# Заменяет ~/ndtlab-server свежей копией из git, собирает и перезапускает службу
set -e

commit="$1"
src="$(cd "$(dirname "$0")/.." && pwd)" # распакованная папка server/ во временном каталоге
target="$HOME/ndtlab-server"

# сборка каждый раз с нуля: около минуты, зато на сервере точно ровно то, что в коммите,
# без остатков прошлой сборки
rm -rf "$target"
mv "$src" "$target"
rm -rf "$(dirname "$src")"
echo "$commit" > "$target/COMMIT"

cd "$target"
./deploy/install.sh

echo
echo "Выложен коммит $commit"
echo "/health: $(curl -s http://127.0.0.1:8080/health)"
