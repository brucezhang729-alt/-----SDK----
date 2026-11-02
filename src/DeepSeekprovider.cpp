#include "../include/DeepSeekprovider.h"
#include "../include/util/mylog.h"
#include <stdexcept>

namespace Ai_chat_sdk
{
    bool DeepSeekprovider::initModel(const std::map<std::string, std::string> &modelConfig)
    {
        _isAvailable = false;
        _apiKey.clear();
        _endpoint.clear();

        const auto apiKey = modelConfig.find("apiKey");
        if (apiKey == modelConfig.end() || apiKey->second.empty())
        {
            ERR("DeepSeekprovider::initModel - API Key is missing or empty in modelConfig");
            return false; // API Key is required
        }
        // 初始化 endpoint
        const auto endpoint = modelConfig.find("endpoint");
        if (endpoint == modelConfig.end() || endpoint->second.empty())
        {
            ERR("DeepSeekprovider::initModel - Endpoint is missing or empty in modelConfig");
            return false; // Endpoint is required
        }

        _apiKey = apiKey->second;
        _endpoint = endpoint->second;
        _isAvailable = true; // 配置已就绪；尚未执行远端连通性验证。
        INFO("DeepSeekprovider::initModel - Model initialized successfully with API Key and Endpoint");
        return true;
    }

    std::string DeepSeekprovider::getModelName()
    {
        return "DeepSeek Model"; // Replace with actual model name if available
    }

    std::string DeepSeekprovider::getModelDescription()
    {
        return "DeepSeek Model for AI chat"; // Replace with actual model description if available
    }

    bool DeepSeekprovider::isAvailable() const
    {
        return _isAvailable;
    }
    void DeepSeekprovider::sendMessage(const std::vector<Message>&,
                             const std::map<std::string, std::string>&,
                             std::function<void(const std::string&)>)
    {
        throw std::logic_error("DeepSeekprovider::sendMessage is not implemented.");
    }
    // 发送信息 增量返还 流式响应
    void DeepSeekprovider::sendMessageStream(const std::vector<Message>&,
                                   const std::map<std::string, std::string>&,
                                   std::function<void(const std::string&, bool)>)
    {
        throw std::logic_error("DeepSeekprovider::sendMessageStream is not implemented.");
    }
} // end Ai_chat_sdk
