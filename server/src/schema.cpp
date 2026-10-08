#include "schema.hpp"

namespace ndtsync
{
    void EnsureSchema(pqxx::connection &conn)
    {
        pqxx::work tx(conn);

        /// общий счётчик версий: каждая принятая запись и каждый файл получают следующий номер,
        /// и программа забирает изменения "всё, что новее N"
        tx.exec("CREATE SEQUENCE IF NOT EXISTS rev_seq;");

        /// записи всех таблиц программы - документами JSON: сервер не знает полей записей,
        /// новое поле в программе не требует правки сервера
        tx.exec(R"(
            CREATE TABLE IF NOT EXISTS records (
                tbl         TEXT   NOT NULL,
                id          TEXT   NOT NULL,
                rev         BIGINT NOT NULL,
                updated_at  BIGINT NOT NULL,
                deleted_at  BIGINT,
                device_id   TEXT   NOT NULL,
                data        JSONB  NOT NULL,
                PRIMARY KEY (tbl, id)
            );)");
        tx.exec("CREATE INDEX IF NOT EXISTS records_rev_idx ON records (rev);");

        /// прикреплённые файлы: содержимое под одним id не меняется
        tx.exec(R"(
            CREATE TABLE IF NOT EXISTS files (
                id          TEXT   PRIMARY KEY,
                rev         BIGINT NOT NULL,
                name        TEXT   NOT NULL,
                size        BIGINT NOT NULL,
                created_at  BIGINT NOT NULL,
                device_id   TEXT   NOT NULL,
                data        BYTEA  NOT NULL
            );)");
        tx.exec("CREATE INDEX IF NOT EXISTS files_rev_idx ON files (rev);");

        /// компьютеры, которым разрешена синхронизация; токен хранится только хэшем
        tx.exec(R"(
            CREATE TABLE IF NOT EXISTS devices (
                id          TEXT   PRIMARY KEY,
                name        TEXT   NOT NULL,
                token_hash  TEXT   NOT NULL UNIQUE,
                created_at  BIGINT NOT NULL,
                last_seen   BIGINT,
                revoked_at  BIGINT
            );)");

        /// версии, затёртые правкой с другого компьютера: остаётся последняя правка,
        /// но прежняя не пропадает безвозвратно
        tx.exec(R"(
            CREATE TABLE IF NOT EXISTS overwritten (
                tbl             TEXT   NOT NULL,
                id              TEXT   NOT NULL,
                rev             BIGINT NOT NULL,
                updated_at      BIGINT NOT NULL,
                deleted_at      BIGINT,
                device_id       TEXT   NOT NULL,
                data            JSONB  NOT NULL,
                overwritten_at  BIGINT NOT NULL,
                overwritten_by  TEXT   NOT NULL
            );)");

        tx.commit();
    }
}
