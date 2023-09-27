#include <iostream>
#include <string>

#include <libmirai/mirai.hpp>

namespace
{

std::string GetTimestamp()
{
	using namespace std::chrono;
	auto tp = system_clock::now();
	std::time_t t = system_clock::to_time_t(tp);

	constexpr size_t BUFFER_SIZE = 128;
	char buf[BUFFER_SIZE]; // NOLINT(*-avoid-c-arrays)
	std::strftime(buf, BUFFER_SIZE, "\x1b[95m%Y-%m-%d %H:%M:%S\x1b[0m", std::localtime(&t));
	return {buf};
}

constexpr std::string_view GetLevelStr(Mirai::LoggingLevels level)
{
	switch (level)
	{
	case Mirai::LoggingLevels::TRACE:
		return " \x1b[37m[TRACE]\x1b[0m ";
	case Mirai::LoggingLevels::DEBUG:
		return " \x1b[34m[DEBUG]\x1b[0m ";
	case Mirai::LoggingLevels::INFO:
		return " \x1b[32m[INFO]\x1b[0m ";
	case Mirai::LoggingLevels::WARN:
		return " \x1b[33m[WARN]\x1b[0m ";
	case Mirai::LoggingLevels::ERROR:
		return " \x1b[31m[ERROR]\x1b[0m ";
	case Mirai::LoggingLevels::FATAL:
		return " \x1b[91m[FATAL]\x1b[0m ";
	default:
		return "";
	}
}

} // namespace

class Logger : public Mirai::ILogger
{
public:
	using ILogger::ILogger;
	void log(const std::string& msg, Mirai::LoggingLevels level) final
	{
		std::string text;
		text = GetTimestamp() + std::string(GetLevelStr(level)) + msg + '\n';
		std::cout << text;
		std::cout.flush();
	}
};

std::shared_ptr<Logger> GetLoggerPtr()
{
	static std::shared_ptr<Logger> logger = std::make_shared<Logger>(Mirai::LoggingLevels::TRACE);
	return logger;
}

inline Logger& GetLogger()
{
	return *GetLoggerPtr();
};


int main()
{
	using namespace Mirai;

	MiraiClient client;
	HttpWsAdaptorConfig config;

	/* Set your other configs here
	config.FromJsonFile("config.json");
	config.FromTOMLFile("config.toml");
	*/
	config.BotQQ = 12345_qq;
	config.VerifyKey = "VerifyKey";
	config.AutoReconnect = false;

	client.SetAdaptor(MakeHttpWsAdaptor(std::move(config)));
	client.SetLogger(GetLoggerPtr());



	client.On<ClientConnectionEstablishedEvent>(
		[](ClientConnectionEstablishedEvent event)
		{
			LOG_INFO(GetLogger(), "成功建立连接, session-key: " + event.SessionKey);
		}
	);
	client.On<ClientConnectionClosedEvent>(
		[](ClientConnectionClosedEvent event)
		{
			LOG_INFO(GetLogger(), "连接关闭：" + event.reason + " <" + std::to_string(event.code) + ">");
		}
	);
	client.On<ClientConnectionErrorEvent>(
		[](ClientConnectionErrorEvent event)
		{
			LOG_WARN(GetLogger(), "连接时出现错误: " + event.reason + "，重试次数: " + std::to_string(event.RetryCount));
		}
	);
	client.On<ClientParseErrorEvent>(
		[](ClientParseErrorEvent event)
		{
			LOG_WARN(GetLogger(), "解析事件时出现错误: " + std::string(event.error.what()));
		}
	);



	client.On<FriendMessageEvent>(
		[](FriendMessageEvent event)
		{
			try
			{
				event.GetMiraiClient().SendFriendMessage(
					event.GetSender().id, 
					event.GetMessage(),
					std::nullopt
				);
			}
			catch(std::exception& e)
			{
				std::cout << "回复消息时发生错误: " << e.what() << std::endl;
			}
		}
	);



	try
	{
		client.Connect();
	}
	catch(std::exception& e)
	{
		std::cout << "连接到mirai时发生错误: " << e.what() << std::endl;
	}

	std::cout << "成功建立连接"<< std::endl;

	try
	{
		std::string mah_version = client.GetMiraiApiHttpVersion();
		std::string mc_version = std::string(client.GetCompatibleVersion());
		LOG_INFO(GetLogger(), "mirai-api-http 的版本: " + mah_version + "; cpp-mirai-client 支持的版本: " + mc_version);
		if (mah_version != mc_version)
		{
			LOG_WARN(GetLogger(), "你的 mirai-api-http 插件的版本与 cpp-mirai-client 支持的版本不同，可能存在兼容性问题。");
		}
	}
	catch (const std::exception& e)
	{
		LOG_WARN(GetLogger(), e.what());
	}

	std::string cmd;
	while (std::cin >> cmd)
	{
		if (cmd == "exit")
		{
			client.Disconnect();
			break;
		}
	}

	return 0;
}