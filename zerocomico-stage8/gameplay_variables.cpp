/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "zerocomico-stage8/gameplay_variables.h"

#include "common/tokenizer.h"

namespace ZeroComico {

namespace {

static Common::String cleanValue(Common::String value) {
	value.trim();
	while (!value.empty() && (value.firstChar() == '"' || value.firstChar() == '\''))
		value.deleteChar(0);
	while (!value.empty() &&
	       (value.lastChar() == '"' || value.lastChar() == '\'' ||
	        value.lastChar() == ';' || value.lastChar() == '}'))
		value.deleteLastChar();
	return value;
}

} // namespace

void GameplayVariables::clear() {
	_variables.clear();
}

int32 GameplayVariables::find(const Common::String &name) const {
	for (uint32 i = 0; i < _variables.size(); ++i) {
		if (_variables[i].name.equalsIgnoreCase(name))
			return (int32)i;
	}
	return -1;
}

void GameplayVariables::parseDeclarations(const Common::String &decodedScript) {
	Common::StringTokenizer lines(decodedScript, "\r\n");
	while (!lines.empty()) {
		Common::String line = lines.nextToken();
		line.trim();
		if (line.empty() || line.hasPrefix("//"))
			continue;

		Common::StringTokenizer tokens(line);
		if (tokens.empty())
			continue;

		const Common::String opcode = tokens.nextToken();
		if (!opcode.equalsIgnoreCase("Variable") || tokens.empty())
			continue;

		const Common::String name = cleanValue(tokens.nextToken());
		Common::String value("0");
		if (!tokens.empty())
			value = cleanValue(tokens.nextToken());

		if (!name.empty())
			set(name, value);
	}
}

bool GameplayVariables::has(const Common::String &name) const {
	return find(name) >= 0;
}

Common::String GameplayVariables::get(const Common::String &name,
                                      const Common::String &fallback) const {
	const int32 id = find(name);
	return id >= 0 ? _variables[id].value : fallback;
}

void GameplayVariables::set(const Common::String &name, const Common::String &value) {
	const int32 id = find(name);
	if (id >= 0) {
		_variables[id].value = cleanValue(value);
		return;
	}

	GameplayVariable variable;
	variable.name = name;
	variable.value = cleanValue(value);
	_variables.push_back(variable);
}

bool GameplayVariables::equals(const Common::String &name,
                               const Common::String &value) const {
	return get(name).equalsIgnoreCase(cleanValue(value));
}

} // End of namespace ZeroComico
