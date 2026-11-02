#pragma once
#include <string>
#include <ctime>
#include<mutex>
#include<vector>

namespace Ai_chat_sdk
{
    // 消息结构
    struct Message
    {
        std::string _messageID; // 消息ID
        std::string _role; // 角色
        std::string _content; // 内容
        std::time_t _timestamp; // 时间戳

        // 构造
        Message(const std::string& role, const std::string& content)
            : _role(role), _content(content), _timestamp(std::time(nullptr)) {}
    };


     // 模型公共信息
struct ModelConfig
{
    std::string _modelName; // 模型名称
    double _temperature = 0.7; // 采样温度参数，介于0和2之间，值越高，生成的文本越随机，值越低，生成的文本越确定
    int _maxTokens = 2048; // 最大token数，限制生成文本的长度
};
    // 通过API接入云端模型
struct ApiModelConfig : public ModelConfig
{
    std::string _apiKey; // API Key
};
    // LLM模型信息
struct ModelInfo
{
    std::string _modelName; // 模型名称
    std::string _modeldescription; // 模型描述
    std::string _modelProvider; // 模型提供者
    std::string _endpoint; // 模型接口地址
    bool _isactive = false; // 模型是否可用

    ModelInfo(const std::string& modelName="", const std::string& modeldescription="", const std::string& modelProvider="", const std::string& endpoint="", bool isactive=false)
        : _modelName(modelName), _modeldescription(modeldescription), _modelProvider(modelProvider), _endpoint(endpoint), _isactive(isactive) {}
};

    // 会话信息
struct Session
{
    std::string _sessionID; // 会话ID
    std::string _modelName; // 模型名称
    std::vector<Message> _messages; // 消息列表
    std::time_t _createTime; // 创建时间
    std::time_t _lastUpdateTime; // 最后更新时间

    Session(const std::string& modelName="")
        : _modelName(modelName), _createTime(std::time(nullptr)), _lastUpdateTime(_createTime)
    {}
};
    // 通过Ollama接入本地模型

}
