#pragma once

#include <string>

namespace ndtsync
{
    /// @brief Запустить HTTP-сервер и обслуживать запросы до остановки процесса.
    /// Шифрования здесь нет: сервер слушает только 127.0.0.1, снаружи к нему ходят
    /// через SSH-туннель, а позже - через nginx с HTTPS
    /// @return false - не удалось занять адрес (порт занят и т.п.)
    bool RunServer(const std::string &host, int port);
}
