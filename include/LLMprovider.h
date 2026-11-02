#pragma once

#include "common.h"
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace Ai_chat_sdk
{
    // LLMprovide类
    class LLMprovider
    {
        public:
            virtual ~LLMprovider() = default;

            // 初始化模型
            virtual bool initModel(const std::map<std::string, std::string>& modelConfig) = 0;
            // 模型名称
            virtual std::string getModelName() = 0;
            // 模型描述
            virtual std::string getModelDescription() = 0;
            // 模型是否有效
            virtual bool isAvailable() const = 0;
            // 发送消息 全量返回
            virtual void sendMessage(const std::vector<Message>&messages,
                                     const std::map<std::string,std::string>&requestConfig,
                                     std::function<void(const std::string&)> callback) = 0;
            // 发送信息 增量返还 流式响应
            virtual void sendMessageStream(const std::vector<Message>&messages,
                                           const std::map<std::string,std::string>&requestConfig,
                                           std::function<void(const std::string&,bool)> callback) = 0;
            protected:
                bool _isAvailable = false; // 模型是否可用
                std::string _apiKey; // API Key
                std::string _endpoint; // 模型接口地址
    }; // end LLMprovider

}
