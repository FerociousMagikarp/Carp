#include "engine.h"
#include "protocol/option.h"

namespace Carp
{

void Engine::InitOptions(OptionContainer& container) const
{
	container.AddOption<OptionSpin>("Threads", 2, 1, 1024, [](const Option& option)->void {

		});
	container.AddOption<OptionSpin>("Hash", 16, 1, 33554432, [](const Option& option)->void {

		});
	container.AddOption<OptionButton>("Clear Hash", [](const Option& option)->void {

		});
	container.AddOption<OptionCheck>("Ponder", false);
	container.AddOption<OptionSpin>("MultiPV", 1, 1, 128);
	container.AddOption<OptionCombo>("Repetition Rule", "AsianRule", std::vector<std::string>{"AsianRule", "ChineseRule"});
	container.AddOption<OptionString>("EvalFile", "placeholder.txt");
}

} // namespace Carp
