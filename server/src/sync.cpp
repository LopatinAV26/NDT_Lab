#include "sync.hpp"

#include <array>
#include <algorithm>
#include <cstddef>
#include <vector>

using json = nlohmann::json;

namespace
{
    /// таблицы программы, которые синхронизируются. Замеры и дефекты отдельно не ходят -
    /// они внутри документа заключения
    constexpr std::array<std::string_view, 9> knownTables = {
        "laboratory_info",
        "employees",
        "inspectors",
        "masters",
        "welders",
        "equipment",
        "control_maps",
        "normative_documents",
        "reports",
    };

    /// ключ блокировки, под которой пачки записываются строго по очереди: номера версий
    /// выдаются при записи, а видны другим только после COMMIT. Без очереди пачка с меньшим
    /// номером могла бы завершиться позже пачки с большим - и её не забрал бы никто,
    /// кто уже спросил "всё новее большего номера"
    constexpr std::int64_t pushLockKey = 0x4E445453594E43; // "NDTSYNC"

    constexpr const char *nowSql = "extract(epoch from now())::bigint";

    /// запись из пачки, уже проверенная - до транзакции, чтобы ошибка в последней записи
    /// не оставила половину пачки в базе
    struct IncomingRecord
    {
        std::string tbl;
        std::string id;
        std::int64_t baseRev = 0;
        std::int64_t updatedAt = 0;
        std::optional<std::int64_t> deletedAt;
        std::string data; ///< JSON-объект текстом - в базу уходит как jsonb
    };

    std::int64_t GetInt64(const json &record, const char *key, size_t index)
    {
        const auto it = record.find(key);
        if (it == record.end() || !it->is_number_integer())
            throw ndtsync::BadRequest("records[" + std::to_string(index) + "]." + key + ": нужно целое число");
        return it->get<std::int64_t>();
    }

    std::string GetString(const json &record, const char *key, size_t index)
    {
        const auto it = record.find(key);
        if (it == record.end() || !it->is_string() || it->get_ref<const std::string &>().empty())
            throw ndtsync::BadRequest("records[" + std::to_string(index) + "]." + key + ": нужна непустая строка");
        return it->get<std::string>();
    }

    IncomingRecord ParseRecord(const json &record, size_t index)
    {
        if (!record.is_object())
            throw ndtsync::BadRequest("records[" + std::to_string(index) + "]: нужен объект");

        IncomingRecord r;
        r.tbl = GetString(record, "tbl", index);
        if (std::ranges::find(knownTables, r.tbl) == knownTables.end())
            throw ndtsync::BadRequest("records[" + std::to_string(index) + "].tbl: неизвестная таблица " + r.tbl);

        r.id = GetString(record, "id", index);
        r.baseRev = GetInt64(record, "base_rev", index);
        r.updatedAt = GetInt64(record, "updated_at", index);

        const auto deleted = record.find("deleted_at");
        if (deleted != record.end() && !deleted->is_null())
            r.deletedAt = GetInt64(record, "deleted_at", index);

        const auto data = record.find("data");
        if (data == record.end() || !data->is_object())
            throw ndtsync::BadRequest("records[" + std::to_string(index) + "].data: нужен объект");
        r.data = data->dump();

        return r;
    }

    std::basic_string_view<std::byte> AsBytes(std::string_view data)
    {
        return {reinterpret_cast<const std::byte *>(data.data()), data.size()};
    }
}

namespace ndtsync
{
    json Push(pqxx::connection &conn, const std::string &deviceId, const json &body)
    {
        const auto records = body.find("records");
        if (!body.is_object() || records == body.end() || !records->is_array())
            throw BadRequest("ожидается {\"records\": [...]}");

        std::vector<IncomingRecord> incoming;
        incoming.reserve(records->size());
        for (size_t i = 0; i < records->size(); ++i)
            incoming.push_back(ParseRecord(records->at(i), i));

        json accepted = json::array();

        pqxx::work tx(conn);
        tx.exec_params("SELECT pg_advisory_xact_lock($1);", pushLockKey);

        for (const IncomingRecord &r : incoming)
        {
            const pqxx::result current = tx.exec_params(
                "SELECT rev, device_id FROM records WHERE tbl = $1 AND id = $2 FOR UPDATE;", r.tbl, r.id);

            /// конфликт - только правка поверх версии с ДРУГОГО компьютера: повтор собственной
            /// отправки (ответ потерялся) или две правки подряд с одного компьютера конфликтом не считаются
            const bool conflict = !current.empty() &&
                                  current[0][0].as<std::int64_t>() != r.baseRev &&
                                  current[0][1].as<std::string>() != deviceId;

            if (conflict)
                tx.exec_params(
                    std::string("INSERT INTO overwritten (tbl, id, rev, updated_at, deleted_at, device_id, data, overwritten_at, overwritten_by) "
                                "SELECT tbl, id, rev, updated_at, deleted_at, device_id, data, ") +
                        nowSql + ", $3 FROM records WHERE tbl = $1 AND id = $2;",
                    r.tbl, r.id, deviceId);

            const std::int64_t rev = tx.exec1("SELECT nextval('rev_seq');")[0].as<std::int64_t>();

            tx.exec_params(
                "INSERT INTO records (tbl, id, rev, updated_at, deleted_at, device_id, data) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7::jsonb) "
                "ON CONFLICT (tbl, id) DO UPDATE SET rev = excluded.rev, updated_at = excluded.updated_at, "
                "deleted_at = excluded.deleted_at, device_id = excluded.device_id, data = excluded.data;",
                r.tbl, r.id, rev, r.updatedAt, r.deletedAt, deviceId, r.data);

            accepted.push_back({{"tbl", r.tbl}, {"id", r.id}, {"rev", rev}, {"conflict", conflict}});
        }

        tx.commit();
        return {{"accepted", accepted}};
    }

    json Pull(pqxx::connection &conn, std::int64_t since, int limit)
    {
        pqxx::read_transaction tx(conn);
        const pqxx::result rows = tx.exec_params(
            "SELECT tbl, id, rev, updated_at, deleted_at, data::text FROM records "
            "WHERE rev > $1 ORDER BY rev LIMIT $2;",
            since, limit);

        json records = json::array();
        std::int64_t lastRev = since;

        for (const pqxx::row &row : rows)
        {
            const std::int64_t rev = row[2].as<std::int64_t>();
            lastRev = rev; /// строки идут по возрастанию rev

            records.push_back({
                {"tbl", row[0].as<std::string>()},
                {"id", row[1].as<std::string>()},
                {"rev", rev},
                {"updated_at", row[3].as<std::int64_t>()},
                {"deleted_at", row[4].is_null() ? json(nullptr) : json(row[4].as<std::int64_t>())},
                {"data", json::parse(row[5].as<std::string>())},
            });
        }

        /// ровно limit строк - возможно, есть ещё: клиент спросит следующую порцию с since = last_rev
        const bool more = static_cast<int>(rows.size()) == limit;
        return {{"records", records}, {"last_rev", lastRev}, {"more", more}};
    }

    void PutFile(pqxx::connection &conn, const std::string &deviceId, const std::string &id,
                 const std::string &name, std::string_view data)
    {
        if (name.empty())
            throw BadRequest("нужно имя файла: ?name=...");

        pqxx::work tx(conn);
        tx.exec_params(
            std::string("INSERT INTO files (id, rev, name, size, created_at, device_id, data) "
                        "VALUES ($1, nextval('rev_seq'), $2, $3, ") +
                nowSql + ", $4, $5) ON CONFLICT (id) DO NOTHING;",
            id, name, static_cast<std::int64_t>(data.size()), deviceId, AsBytes(data));
        tx.commit();
    }

    std::optional<StoredFile> GetFile(pqxx::connection &conn, const std::string &id)
    {
        pqxx::read_transaction tx(conn);
        const pqxx::result rows = tx.exec_params("SELECT name, data FROM files WHERE id = $1;", id);
        if (rows.empty())
            return std::nullopt;

        const auto bytes = rows[0][1].as<std::basic_string<std::byte>>();
        return StoredFile{rows[0][0].as<std::string>(),
                          std::string(reinterpret_cast<const char *>(bytes.data()), bytes.size())};
    }
}
