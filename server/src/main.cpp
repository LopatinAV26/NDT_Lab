/// Сервер синхронизации NDT Lab.
///   ndtsync serve                     - обслуживать запросы (так запускает systemd)
///   ndtsync add-device "Название"     - разрешить компьютеру синхронизацию, выдать токен
///   ndtsync revoke-device <id>        - отозвать доступ
///   ndtsync list-devices              - список компьютеров
/// Подключение к PostgreSQL - стандартными переменными PG* (PGHOST, PGUSER, PGPASSWORD...)
/// из /etc/ndtlab/db.env

#include <cstdio>
#include <exception>
#include <string>

#include <pqxx/pqxx>

#include "devices.hpp"
#include "env.hpp"
#include "httpServer.hpp"
#include "schema.hpp"

namespace
{
    constexpr const char *envFile = "/etc/ndtlab/db.env";

    int PrintUsage()
    {
        std::fprintf(stderr,
                     "Использование:\n"
                     "  ndtsync serve\n"
                     "  ndtsync add-device \"Название\"\n"
                     "  ndtsync revoke-device <id>\n"
                     "  ndtsync list-devices\n");
        return 2;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return PrintUsage();

    const std::string command = argv[1];

    ndtsync::LoadEnvFile(envFile);

    try
    {
        pqxx::connection conn;
        ndtsync::EnsureSchema(conn);

        if (command == "serve")
        {
            conn.close(); /// дальше у каждого запроса своё соединение

            const std::string host = ndtsync::GetEnv("NDTSYNC_HOST", "127.0.0.1");
            const int port = std::stoi(ndtsync::GetEnv("NDTSYNC_PORT", "8080"));
            if (!ndtsync::RunServer(host, port))
            {
                std::fprintf(stderr, "ndtsync: не удалось занять %s:%d\n", host.c_str(), port);
                return 1;
            }
            return 0;
        }

        if (command == "add-device" && argc == 3)
        {
            ndtsync::AddDevice(conn, argv[2]);
            return 0;
        }

        if (command == "revoke-device" && argc == 3)
        {
            if (!ndtsync::RevokeDevice(conn, argv[2]))
            {
                std::fprintf(stderr, "Действующего компьютера с id %s нет\n", argv[2]);
                return 1;
            }
            std::printf("Доступ отозван\n");
            return 0;
        }

        if (command == "list-devices")
        {
            ndtsync::PrintDevices(conn);
            return 0;
        }
    }
    catch (const std::exception &e)
    {
        std::fprintf(stderr, "ndtsync: %s\n", e.what());
        return 1;
    }

    return PrintUsage();
}
