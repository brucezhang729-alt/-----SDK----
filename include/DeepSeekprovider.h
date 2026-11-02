#pragma once

#include "LLMprovider.h"

namespace Ai_chat_sdk
{
    // DeepSeek provider 类
    class DeepSeekprovider : public LLMprovider
    {
        public:
            // 初始化模型
            bool initModel(const std::map<std::string, std::string>& modelConfig) override;
            // 模型名称
            std::string getModelName() override;
            // 模型描述
            std::string getModelDescription() override;
            // 模型是否有效
            bool isAvailable() const override;
            // 返回完整回复，并在成功时调用非空回调一次；请求失败返回空字符串。
            std::string sendMessage(const std::vector<Message>&messages,
                                     const std::map<std::string,std::string>&requestConfig,
                                     std::function<void(const std::string&)> callback) override;
            // 发送信息 增量返还 流式响应；当前尚未实现，调用会抛出 std::logic_error。
            std::string sendMessageStream(const std::vector<Message>&messages,
                                           const std::map<std::string,std::string>&requestConfig,
                                           std::function<void(const std::string&,bool)> callback) override;
        private:
            std::string _modelName = "deepseek-flash";
    }; // end DeepSeekprovider

}
