#ifndef TOOLS_MESSAGES_H
#define TOOLS_MESSAGES_H

#if defined _MSC_VER && _MSC_VER >= 1020u
#pragma once
#endif

#include <string> 
#include <boost/format.hpp>

#define FAILED_TO_SET_LOCK_ON_FILE "Не удалось установить блокировку на файл"
#define FAILED_TO_SET_FILE_DESCRIPTOR "Не удалось получить дескриптор файла"

namespace tools
{
    namespace details
    {
        namespace messages
        {
            inline std::string error_while_trying_to_lock_file(std::string const &path_to_file, std::string const &error_descr)
            {
                return (boost::format{"При попытке заблокировать возможность записи в файл %1% произошла ошибка: %2%"}
                    % path_to_file
                    % error_descr
                ).str();
            }
        }
    }
}

#endif
