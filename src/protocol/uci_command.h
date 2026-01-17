#pragma once

#include <iostream>
#include <span>
#include <string_view>
#include <string>
#include <unordered_map>

namespace Carp
{

class Engine;
class OptionContainer;

class UciCommand
{
public:
	UciCommand(Engine& engine, OptionContainer& option_cont);
	~UciCommand();
	UciCommand(const UciCommand&) = delete;
	UciCommand(UciCommand&&) = delete;
	UciCommand& operator=(const UciCommand&) = delete;
	UciCommand& operator=(UciCommand&&) = delete;

	friend std::ostream& operator<< (std::ostream& os, const UciCommand& uci);

	// 通过给定命令解析，返回需要输出的字符串
	std::string AnalyzeCommand(std::span<std::string_view> commands);

private:
	Engine& m_engine;
	OptionContainer& m_option_container;
	using command_func = std::string(UciCommand::*)(std::span<std::string_view>);
	const std::unordered_map<std::string_view, command_func> m_commands;

	// 这里记录了所有uci协议会用到的控制命令
	std::string C_SetOption(std::span<std::string_view> commands);
	std::string C_IsReady(std::span<std::string_view> commands);
	std::string C_Position(std::span<std::string_view> commands);
	std::string C_Go(std::span<std::string_view> commands);
	std::string C_Stop(std::span<std::string_view> commands);
	std::string C_PonderHit(std::span<std::string_view> commands);
};

} // namespace Carp
