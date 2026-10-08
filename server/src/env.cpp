#include "env.hpp"

#include <cstdlib>
#include <fstream>

namespace
{
    std::string Trim(const std::string &s)
    {
        const size_t begin = s.find_first_not_of(" \t\r");
        if (begin == std::string::npos)
            return {};
        const size_t end = s.find_last_not_of(" \t\r");
        return s.substr(begin, end - begin + 1);
    }
}

namespace ndtsync
{
    void LoadEnvFile(const std::string &path)
    {
        std::ifstream file(path);
        if (!file)
            return;

        std::string line;
        while (std::getline(file, line))
        {
            line = Trim(line);
            if (line.empty() || line.front() == '#')
                continue;

            const size_t eq = line.find('=');
            if (eq == std::string::npos)
                continue;

            const std::string key = Trim(line.substr(0, eq));
            const std::string value = Trim(line.substr(eq + 1));
            setenv(key.c_str(), value.c_str(), 0);
        }
    }

    std::string GetEnv(const char *name, const std::string &defaultValue)
    {
        const char *value = std::getenv(name);
        return value ? value : defaultValue;
    }
}
