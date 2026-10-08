#pragma once

#include <string>

namespace ndtsync
{
    /// @brief Прочитать файл вида KEY=VALUE (как /etc/ndtlab/db.env) в переменные окружения.
    /// Уже заданные переменные не перезаписываются: у службы их выставляет systemd (EnvironmentFile),
    /// а у команд администратора, запущенных вручную, - этот файл. Нет доступа к файлу - ничего не делает
    void LoadEnvFile(const std::string &path);

    /// @brief Значение переменной окружения или defaultValue, если она не задана
    std::string GetEnv(const char *name, const std::string &defaultValue = {});
}
