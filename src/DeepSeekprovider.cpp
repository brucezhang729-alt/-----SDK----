#include "../include/DeepSeekprovider.h"
#include "../include/util/mylog.h"
#include <cmath>
#include <regex>
#include <stdexcept>
#include <sstream>
#include <json/json.h>
#include <httplib.h> // Include the HTTP library for making requests

namespace
{
    // endpoint 支持服务器地址、API 基础路径或完整的 chat/completions 地址。
    bool splitEndpoint(const std::string& endpoint, std::string& origin, std::string& path)
    {
        std::size_t authorityStart = 0;
        if (endpoint.compare(0, 7, "http://") == 0)
            authorityStart = 7;
        else if (endpoint.compare(0, 8, "https://") == 0)
            authorityStart = 8;
        else
            return false;

        if (endpoint.find_first_of(" \t\r\n?#@") != std::string::npos)
            return false;
        const auto pathStart = endpoint.find('/', authorityStart);
        const auto authorityEnd = pathStart == std::string::npos ? endpoint.size() : pathStart;
        if (authorityEnd == authorityStart)
            return false;

        origin = endpoint.substr(0, authorityEnd);
        static const std::regex originPattern(
            R"(^https?://(?:\[[0-9a-fA-F:]+\]|[^:/?#\s]+)(?::([0-9]{1,5}))?$)");
        std::smatch match;
        if (!std::regex_match(origin, match, originPattern))
            return false;
        if (match[1].matched)
        {
            const int port = std::stoi(match[1].str());
            if (port < 1 || port > 65535)
                return false;
        }
        path = pathStart == std::string::npos ? "" : endpoint.substr(pathStart);
        while (!path.empty() && path.back() == '/')
            path.pop_back();
        const std::string suffix = "/chat/completions";
        if (path.size() < suffix.size() ||
            path.compare(path.size() - suffix.size(), suffix.size(), suffix) != 0)
            path += suffix;
        return true;
    }
}

namespace Ai_chat_sdk
{
    bool DeepSeekprovider::initModel(const std::map<std::string, std::string> &modelConfig)
    {
        _isAvailable = false;
        _apiKey.clear();
        _endpoint.clear();
        _modelName = "deepseek-flash";

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

        std::string origin;
        std::string path;
        if (!splitEndpoint(endpoint->second, origin, path))
        {
            ERR("DeepSeekprovider::initModel - Endpoint must be an http(s) URL without query or fragment.");
            return false;
        }
        const auto modelName = modelConfig.find("modelName");
        if (modelName != modelConfig.end() && modelName->second.empty())
        {
            ERR("DeepSeekprovider::initModel - modelName must not be empty.");
            return false;
        }

        _apiKey = apiKey->second;
        _endpoint = endpoint->second;
        if (modelName != modelConfig.end())
            _modelName = modelName->second;
        _isAvailable = true; // 配置已就绪；尚未执行远端连通性验证。
        INFO("DeepSeekprovider::initModel - Model initialized successfully with API Key and Endpoint");
        return true;
    }

    std::string DeepSeekprovider::getModelName()
    {
        return _modelName;
    }

    std::string DeepSeekprovider::getModelDescription()
    {
        return "DeepSeek Model for AI chat"; // Replace with actual model description if available
    }

    bool DeepSeekprovider::isAvailable() const
    {
        return _isAvailable;
    }
    std::string DeepSeekprovider::sendMessage(const std::vector<Message>& messages,
                             const std::map<std::string, std::string>& requestConfig,
                             std::function<void(const std::string&)> callback)
    {
        // 1. 检查模型是否可用
        if(!_isAvailable)
        {
            ERR("DeepSeekprovider::sendMessage - Model is not available. Please initialize the model first.");
            return"";
        }
        if (messages.empty())
        {
            ERR("DeepSeekprovider::sendMessage - messages must not be empty.");
            return "";
        }
        // 2. 构造请求参数
        double temperature = 0.7; // 默认温度
        int maxTokens = 2048; // 默认最大token数
        if(requestConfig.find("temperature") != requestConfig.end())
        {
            try
            {
                const auto& value = requestConfig.at("temperature");
                std::size_t consumed = 0;
                temperature = std::stod(value, &consumed);
                if (consumed != value.size() || !std::isfinite(temperature) ||
                    temperature < 0.0 || temperature > 2.0)
                    throw std::invalid_argument("temperature must be a finite number between 0 and 2");
            }
            catch(const std::exception& e)
            {
                ERR("DeepSeekprovider::sendMessage - Invalid temperature value: {}", e.what());
                return "";
            }
        }
        if(requestConfig.find("maxTokens") != requestConfig.end())
        {
            try
            {
                const auto& value = requestConfig.at("maxTokens");
                std::size_t consumed = 0;
                maxTokens = std::stoi(value, &consumed);
                if (consumed != value.size() || maxTokens <= 0)
                    throw std::invalid_argument("maxTokens must be a positive integer");
            }
            catch(const std::exception& e)
            {
                ERR("DeepSeekprovider::sendMessage - Invalid maxTokens value: {}", e.what());
                return "";
            }
        }
        // 3. 构造历史消息
        Json::Value messageArray(Json::arrayValue);
        for(const auto& msg : messages)
        {
            Json::Value messageObj(Json::objectValue);
            messageObj["role"] = msg._role;
            messageObj["content"] = msg._content;
            messageArray.append(messageObj);
        }

        // 4. 构造请求体
        Json::Value requestBody(Json::objectValue);
        requestBody["model"] = getModelName();
        requestBody["messages"] = messageArray;
        requestBody["temperature"] = temperature;
        requestBody["max_tokens"] = maxTokens;
        requestBody["stream"] = false;

        // 5. 序列化
        Json::StreamWriterBuilder writer;
        std::string requestBodyStr = Json::writeString(writer, requestBody);

        // 6.使用httplib或其他HTTP库发送POST请求到DeepSeek API
        std::string origin;
        std::string path;
        if (!splitEndpoint(_endpoint, origin, path))
        {
            ERR("DeepSeekprovider::sendMessage - Invalid endpoint.");
            return "";
        }
#if !defined(CPPHTTPLIB_OPENSSL_SUPPORT) && !defined(CPPHTTPLIB_MBEDTLS_SUPPORT) && !defined(CPPHTTPLIB_WOLFSSL_SUPPORT)
        // HTTPS 构建须启用 cpp-httplib 的 TLS 后端并链接相应依赖。
        if (origin.compare(0, 8, "https://") == 0)
        {
            ERR("DeepSeekprovider::sendMessage - HTTPS requires cpp-httplib TLS support (for example CPPHTTPLIB_OPENSSL_SUPPORT).");
            return "";
        }
#endif
        httplib::Client client(origin);
        if (!client.is_valid())
        {
            ERR("DeepSeekprovider::sendMessage - HTTP client could not be initialized.");
            return "";
        }
        client.set_connection_timeout(5,0);
        client.set_read_timeout(120,0);
        client.set_write_timeout(5,0);

        // 7.设置请求头
        httplib::Headers headers = {
            {"Authorization", "Bearer " + _apiKey},
        };

        // 8. 发送请求
        auto res = client.Post(path, headers, requestBodyStr, "application/json");
        if(!res)
        {
            ERR("DeepSeekprovider::sendMessage - HTTP request failed: {}", httplib::to_string(res.error()));
            return "";
        }
        INFO("DeepSeekprovider::sendMessage - HTTP request sent successfully, status: {}", res->status);
        // 检测状态码是否为成功
        if(res->status != 200)
        {
            ERR("DeepSeekprovider::sendMessage - HTTP request failed with status: {}", res->status);
            return "";
        }

        // 9. 解析响应
        Json::Value responseBody;
        Json::CharReaderBuilder reader;
        reader["failIfExtra"] = true;
        reader["allowComments"] = false;
        std::string errs;
        std::istringstream responseStream(res->body);
        if(!Json::parseFromStream(reader, responseStream, &responseBody, &errs))
        {
            ERR("DeepSeekprovider::sendMessage - Failed to parse response JSON: {}", errs);
            return "";
        }
        else
        {
            // 获取Message数组
            if(responseBody.isObject() && responseBody.isMember("choices") && responseBody["choices"].isArray() && !responseBody["choices"].empty())
            {
                const auto& firstChoice = responseBody["choices"][0];
                if(firstChoice.isObject() && firstChoice.isMember("message") && firstChoice["message"].isObject())
                {
                    const auto& messageObj = firstChoice["message"];
                    if(messageObj.isMember("content") && messageObj["content"].isString())
                    {
                        std::string content = messageObj["content"].asString();
                        if (callback)
                            callback(content);
                        return content;
                    }
                }
            }
        }

        // 10. JSON解析失败或未找到预期字段
        ERR("DeepSeekprovider::sendMessage - Unexpected response format or missing fields.");
        return "";
    }
    // 发送信息 增量返还 流式响应
    std::string DeepSeekprovider::sendMessageStream(const std::vector<Message>&,
                                   const std::map<std::string, std::string>&,
                                   std::function<void(const std::string&, bool)>)
    {
        throw std::logic_error("DeepSeekprovider::sendMessageStream is not implemented.");
    }
} // end Ai_chat_sdk
