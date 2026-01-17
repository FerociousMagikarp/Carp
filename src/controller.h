#pragma once

#include <memory>

namespace Carp
{
class Engine;
class Command;
class OptionContainer;

class Controller
{
public:
	Controller();
	~Controller();
	Controller(const Controller&) = delete;
	Controller(Controller&&) = delete;
	Controller& operator=(const Controller&) = delete;
	Controller& operator=(Controller&&) = delete;

	void Loop();

private:
	const std::unique_ptr<Engine> m_engine;
	std::unique_ptr<Command> m_command;
	std::unique_ptr<OptionContainer> m_option_container;
};

} // namespace Carp

