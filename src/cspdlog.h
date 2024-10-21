#ifndef CSPDLOG_H
#define CSPDLOG_H


#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/sinks/daily_file_sink.h"
#include <memory>
#include <string>
#include <sys/time.h>


class CSpdlog
{
protected:

        CSpdlog();
        
        static CSpdlog *m_instance;
        static inline int NowDateToInt();
        static inline int NowTimeToInt();
        static inline std::string NowDateTimeToString();
        static inline std::string NowTimeToString();


public:
        ~CSpdlog();

        static CSpdlog *GetInstance();
        static std::shared_ptr<spdlog::logger> getLogger();
        void Init(const std::string & name,const std::string &logPath, std::size_t max_size=10*1024*1024, std::size_t max_file = 1000);
        void SetConsoleLogLevel(spdlog::level::level_enum log_level);
        void SetFileLogLevel(spdlog::level::level_enum log_level);
        void Drop();
        void FlushLog();


private:
        std::vector<spdlog::sink_ptr> m_sinks;
        std::shared_ptr<spdlog::logger> m_logger;
        short int m_rotationHour;
        short int m_rotationMinute;
};



#define XTRACE(...) SPDLOG_LOGGER_CALL(CSpdlog::GetInstance()->getLogger().get(), spdlog::level::trace, __VA_ARGS__)
#define XDEBUG(...) SPDLOG_LOGGER_CALL(CSpdlog::GetInstance()->getLogger().get(), spdlog::level::debug, __VA_ARGS__)
#define XINFO(...) SPDLOG_LOGGER_CALL(CSpdlog::GetInstance()->getLogger().get(), spdlog::level::info, __VA_ARGS__)
#define XWARN(...) SPDLOG_LOGGER_CALL(CSpdlog::GetInstance()->getLogger().get(), spdlog::level::warn, __VA_ARGS__)
#define XERROR(...) SPDLOG_LOGGER_CALL(CSpdlog::GetInstance()->getLogger().get(), spdlog::level::err, __VA_ARGS__)
#define XCRITICAL(...) SPDLOG_LOGGER_CALL(CSpdlog::GetInstance()->getLogger().get(), spdlog::level::critical, __VA_ARGS__)
#define FLUSHLOG() CSpdlog::GetInstance()->FlushLog();
#define DROPLOG() CSpdlog::GetInstance()->Drop();


#endif // CSPDLOG_H