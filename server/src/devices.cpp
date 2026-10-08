#include "devices.hpp"

#include <cstdio>
#include <random>

namespace
{
    /// хэш считает PostgreSQL - сервер обходится без OpenSSL
    constexpr const char *tokenHashSql = "encode(sha256(convert_to($1, 'UTF8')), 'hex')";

    /// время в той же форме, что и в программе: секунды Unix по UTC
    constexpr const char *nowSql = "extract(epoch from now())::bigint";

    /// @brief 32 случайных байта в шестнадцатеричном виде. std::random_device в Linux читает
    /// /dev/urandom - криптографически стойкий источник
    std::string GenerateToken()
    {
        std::random_device random;
        std::string token;
        token.reserve(64);

        constexpr char digits[] = "0123456789abcdef";
        for (int i = 0; i < 32; ++i)
        {
            const unsigned byte = random() & 0xFF;
            token += digits[byte >> 4];
            token += digits[byte & 0x0F];
        }
        return token;
    }
}

namespace ndtsync
{
    std::string AddDevice(pqxx::connection &conn, const std::string &name)
    {
        const std::string token = GenerateToken();

        pqxx::work tx(conn);
        const pqxx::row row = tx.exec_params1(
            std::string("INSERT INTO devices (id, name, token_hash, created_at) VALUES (gen_random_uuid()::text, $2, ") +
                tokenHashSql + ", " + nowSql + ") RETURNING id;",
            token, name);
        tx.commit();

        std::printf("Компьютер добавлен: %s\nid:    %s\nтокен: %s\n\nТокен показывается один раз - сразу внесите его в программу.\n",
                    name.c_str(), row[0].c_str(), token.c_str());
        return token;
    }

    bool RevokeDevice(pqxx::connection &conn, const std::string &id)
    {
        pqxx::work tx(conn);
        const pqxx::result result = tx.exec_params(
            std::string("UPDATE devices SET revoked_at = ") + nowSql + " WHERE id = $1 AND revoked_at IS NULL;", id);
        tx.commit();
        return result.affected_rows() > 0;
    }

    void PrintDevices(pqxx::connection &conn)
    {
        pqxx::read_transaction tx(conn);
        const pqxx::result rows = tx.exec(
            "SELECT id, name, to_timestamp(created_at)::date, "
            "coalesce(to_char(to_timestamp(last_seen), 'YYYY-MM-DD HH24:MI'), '-'), "
            "CASE WHEN revoked_at IS NULL THEN 'действует' ELSE 'отозван' END "
            "FROM devices ORDER BY created_at;");

        if (rows.empty())
        {
            std::printf("Компьютеров нет. Добавить: ndtsync add-device \"Название\"\n");
            return;
        }

        for (const pqxx::row &row : rows)
            std::printf("%s  %-30s  добавлен %s  последний раз %s  %s\n",
                        row[0].c_str(), row[1].c_str(), row[2].c_str(), row[3].c_str(), row[4].c_str());
    }

    std::optional<Device> Authenticate(pqxx::connection &conn, const std::string &token)
    {
        if (token.empty())
            return std::nullopt;

        pqxx::work tx(conn);
        const pqxx::result rows = tx.exec_params(
            std::string("UPDATE devices SET last_seen = ") + nowSql +
                " WHERE token_hash = " + tokenHashSql + " AND revoked_at IS NULL RETURNING id, name;",
            token);
        tx.commit();

        if (rows.empty())
            return std::nullopt;

        return Device{rows[0][0].as<std::string>(), rows[0][1].as<std::string>()};
    }
}
