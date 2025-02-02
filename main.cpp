
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

namespace Logs {

class FileSink
{
private:
    std::vector<char> m_buffer;
    std::ofstream     m_output;
    std::size_t       m_offset;
    std::mutex        m_mutex;
    
public:
    FileSink(const std::string& file_name, std::size_t buffer_size = 4096u ) : m_buffer(buffer_size), m_output(file_name), m_offset(0u) 
    {
        if (!m_output.is_open()) 
        {
            throw std::runtime_error("File was not created");
        }
    }

    void write(const std::string& data)
    {
        const std::size_t data_size = data.size();
        std::lock_guard<std::mutex> lock(m_mutex);
        if (data_size + m_offset > m_buffer.size()) {
            flush();    
        }

        std::copy(std::begin(data), std::end(data), std::begin(m_buffer) + m_offset);
        m_offset += data_size;
    }

    ~FileSink()
    {
        flush();
    }

private:
    void flush()
    {
        if (m_offset != 0u) {
            m_output.write(m_buffer.data(), m_offset);
            m_offset = 0u;   
        }
    }
};

class Logger
{
    private:
        FileSink m_sink;
    public:
        enum Level {
            TRACE_LEVEL,
            DEBUG_LEVEL,
            INFO_LEVEL,
            WARN_LEVEL,
            ERROR_LEVEL,
            FATAL_LEVEL
        }; 

        static inline std::array<std::string, Level::FATAL_LEVEL + 1u> LevelStr = {
            "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
        };
    public:
        Logger(const std::string& file_name) : m_sink(file_name)
        {

        }
        void log(Level level, const char* source, const std::string& message) 
        {
            const std::string formatted_message = "[" + LevelStr[level] + "] - " + source + " - " + message + "\n";  
            m_sink.write(formatted_message);
        }
    };
    


inline Logs::Logger& getGlobalLogger()
{
    static Logs::Logger logger("/home/yukayu/data.log");
    return logger; 
}

#define STR_(x)
#define STR(x) STR_(x)
#define LOG_TRACE(message) Logs::getGlobalLogger().log(Logs::Logger::Level::TRACE_LEVEL, __FILE__ ":" STR(__LINE__), (message))
#define LOG_DEBUG(message) Logs::getGlobalLogger().log(Logs::Logger::Level::DEBUG_LEVEL, __FILE__ ":" STR(__LINE__), (message))
#define LOG_INFO(message) Logs::getGlobalLogger().log(Logs::Logger::Level::INFO_LEVEL, __FILE__ ":" STR(__LINE__), (message))
#define LOG_WARN(message) Logs::getGlobalLogger().log(Logs::Logger::Level::WARN_LEVEL, __FILE__ ":" STR(__LINE__), (message))
#define LOG_ERROR(message) Logs::getGlobalLogger().log(Logs::Logger::Level::ERROR_LEVEL, __FILE__ ":" STR(__LINE__), (message))
#define LOG_FATAL(message) Logs::getGlobalLogger().log(Logs::Logger::Level::FATAL_LEVEL, __FILE__ ":" STR(__LINE__), (message))
}


int main(int, char**) 
{

    std::thread worker([] {
        for (std::size_t i = 0; i < 100u; ++i) {
            LOG_DEBUG("Hello from another thread.");
        }  
    });

    for (std::size_t i = 0; i < 100u; ++i) {
        LOG_DEBUG("Hello from main thread.");
    }  

    worker.join();
    return 0;
}