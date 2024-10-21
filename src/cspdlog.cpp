#include "cspdlog.h"

#include <cstdio>
#include <iostream>
#include "spdlog/sinks/stdout_color_sinks.h" // or "../stdout_sinks.h" if no color needed
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"




CSpdlog::CSpdlog()
{
    m_rotationHour = 0;
    m_rotationMinute = 0;

}

CSpdlog::~CSpdlog()
{
    spdlog::drop_all();
}

int CSpdlog::NowDateToInt()
{
    time_t now;
    time(&now);

    // choose thread save version in each platform
    tm p;
#ifdef _WIN32
    localtime_s(&p, &now);
#else
    localtime_r(&now, &p);
    
 #endif // _WIN32
    int now_date = (1900 + p.tm_year) * 10000 + (p.tm_mon + 1) * 100 + p.tm_mday;
    return now_date;
}

int CSpdlog::NowTimeToInt()
{
    time_t now;
    time(&now);
    // choose thread save version in each platform
    tm p;
#ifdef _WIN32
    localtime_s(&p, &now);
#else
    localtime_r(&now, &p);
#endif // _WIN32

    int now_int = p.tm_hour * 10000 + p.tm_min * 100 + p.tm_sec;
    return now_int;
}

std::string getTime()
{
    time_t timep;
    time (&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&timep));
    return tmp;
}


std::string CSpdlog::NowDateTimeToString()
{
    time_t now;
    time(&now);
    timeval now_mill;
    gettimeofday(&now_mill, NULL);
    // choose thread save version in each platform
    tm p;
// #ifdef _WIN32
//     localtime_s(&p, &now);
// #else
//     localtime_r(&now, &p);
// #endif // _WIN32

    
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d_%H:%M:%S",localtime(&now));
    std::string strTime = tmp;
    std::string strMillSecond = std::to_string((now_mill.tv_usec/1000));
    strTime.append(":");
    strTime.append(strMillSecond);

    return strTime;

    //int now_int = p.tm_hour * 10000 + p.tm_min * 100 + p.tm_sec;
    //return now_int;
    //std::string strTime = std::to_string(p.tm_hour) + std::to_string(p.tm_hour);
}

std::string CSpdlog::NowTimeToString()
{
    time_t now;
    time(&now);
    timeval now_mill;
    gettimeofday(&now_mill, NULL);
    // choose thread save version in each platform
    tm p;
// #ifdef _WIN32
//     localtime_s(&p, &now);
// #else
//     localtime_r(&now, &p);
// #endif // _WIN32

    
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%H_%M_%S",localtime(&now));
    std::string strTime = tmp;
    std::string strMillSecond = std::to_string((now_mill.tv_usec/1000));
    strTime.append("_");
    strTime.append(strMillSecond);

    return strTime;

    //int now_int = p.tm_hour * 10000 + p.tm_min * 100 + p.tm_sec;
    //return now_int;
    //std::string strTime = std::to_string(p.tm_hour) + std::to_string(p.tm_hour);
}

void CSpdlog::Init(const std::string & name, const std::string &log_path, std::size_t max_size, std::size_t max_file )
{
    try
    {
        // logger name with timestamp
        //int date = NowDateToInt();
        //int time = NowTimeToInt();

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);
        console_sink->set_pattern("%^[%Y-%m-%d %H:%M:%S:%e] [%n] [tid:%t] [%l] %v%$");
        //console_sink->set_pattern("%Y-%m-%d %H:%M:%S.%f <thread %t> [%l] [%@] %v");

        //const std::string logger_name = name +"_"+ std::to_string(date) + "_" + std::to_string(time);
        std::string logger_name = name;// + "_" +NowTimeToString();

        std::string logFile = log_path + "/" + logger_name + ".txt";

        //auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink.txt", false);
        //auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFile, max_size, max_file);

        auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logFile, this->m_rotationHour, this->m_rotationMinute);

        //file_sink->set_pattern("[%Y-%m-%d %H:%M:%S:%e] [%n] [tid: %t] [%l] %v");
        //file_sink->set_pattern("[%Y-%m-%d %H:%M:%S:%e] <thread %t> [%l] [%@] %v");
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S:%e] <thread %t> [%l] [%s:%#,%!]: %v");
        file_sink->set_level(spdlog::level::warn);

        m_sinks.push_back(console_sink);
        m_sinks.push_back(file_sink);

        //spdlog::logger *logger = new spdlog::logger("multi_sink", {console_sink, file_sink});
        m_logger = std::make_shared<spdlog::logger>(name, begin( m_sinks ), end( m_sinks ));

        //spdlog::set_error_handler([](const std::string& msg){printf("*****Custom log error handler, %s*****%\n", msg.c_str());});

        //注册到spdlog里
        spdlog::register_logger(m_logger);
        //m_logger->info("log init done.");
        m_logger->flush_on(spdlog::level::level_enum::warn);
        spdlog::flush_every(std::chrono::seconds(1)); // 每隔1秒刷新一次日志

        

    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cout<<"Log initialization faild"<<ex.what()<<std::endl;
    }
}

void CSpdlog::SetConsoleLogLevel(spdlog::level::level_enum log_level)
{
    m_logger->set_level(log_level);
}

void CSpdlog::SetFileLogLevel(spdlog::level::level_enum log_level)
{
    m_sinks[1]->set_level(log_level);
}

CSpdlog* CSpdlog::m_instance = NULL;

CSpdlog* CSpdlog::GetInstance()
{
    if ( m_instance == NULL )
    {
        m_instance = new CSpdlog;
        m_instance->Init("rdc","./var"); //初始化日志
        m_instance->SetConsoleLogLevel(spdlog::level::trace); //设置终端界面输出级别
        m_instance->SetFileLogLevel(spdlog::level::trace);     //设置log文件输出级别

    }

    return m_instance;
}

std::shared_ptr<spdlog::logger> CSpdlog::getLogger()
{
    return m_instance->m_logger;
}

void CSpdlog::Drop()
{
    spdlog::drop_all();
}

void CSpdlog::FlushLog()
{
    CSpdlog::GetInstance()->getLogger().get()->flush();
}