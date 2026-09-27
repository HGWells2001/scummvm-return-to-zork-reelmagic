/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "common/tokenizer.h"

#include "zerocomico-stage19/shape_geometry.h"

namespace ZeroComico {

namespace {

static Common::String cleanToken(Common::String token) {
	token.trim();
	while (!token.empty() &&
	       (token.lastChar() == ':' || token.lastChar() == ';' ||
	        token.lastChar() == '{' || token.lastChar() == '}')) {
		token.deleteLastChar();
	}
	while (!token.empty() && (token.firstChar() == '"' || token.firstChar() == '\''))
		token.deleteChar(0);
	while (!token.empty() && (token.lastChar() == '"' || token.lastChar() == '\''))
		token.deleteLastChar();
	return token;
}

static Common::String stripComment(const Common::String &line) {
	bool inQuote = false;
	for (uint32 i = 0; i + 1 < line.size(); ++i) {
		if (line[i] == '"')
			inQuote = !inQuote;
		if (!inQuote && line[i] == '/' && line[i + 1] == '/') {
			Common::String out = line.substr(0, i);
			out.trim();
			return out;
		}
	}
	Common::String out = line;
	out.trim();
	return out;
}

static void tokenizeLine(const Common::String &line,
                         Common::Array<Common::String> &tokens) {
	tokens.clear();
	Common::StringTokenizer tok(line);
	while (!tok.empty()) {
		Common::String t = cleanToken(tok.nextToken());
		if (!t.empty())
			tokens.push_back(t);
	}
}

static bool parseFloatToken(const Common::String &value, float &out) {
	if (value.empty())
		return false;

	uint32 i = 0;
	bool negative = false;
	if (value[i] == '+' || value[i] == '-') {
		negative = value[i] == '-';
		++i;
	}
	if (i >= value.size())
		return false;

	double result = 0.0;
	bool haveDigits = false;
	while (i < value.size() && value[i] >= '0' && value[i] <= '9') {
		result = result * 10.0 + (double)(value[i] - '0');
		++i;
		haveDigits = true;
	}

	if (i < value.size() && value[i] == '.') {
		++i;
		double place = 0.1;
		while (i < value.size() && value[i] >= '0' && value[i] <= '9') {
			result += (double)(value[i] - '0') * place;
			place *= 0.1;
			++i;
			haveDigits = true;
		}
	}

	if (!haveDigits)
		return false;

	int exponent = 0;
	bool exponentNegative = false;
	if (i < value.size() && (value[i] == 'e' || value[i] == 'E')) {
		++i;
		if (i < value.size() && (value[i] == '+' || value[i] == '-')) {
			exponentNegative = value[i] == '-';
			++i;
		}
		if (i >= value.size() || value[i] < '0' || value[i] > '9')
			return false;
		while (i < value.size() && value[i] >= '0' && value[i] <= '9') {
			exponent = exponent * 10 + (int)(value[i] - '0');
			if (exponent > 38)
				return false;
			++i;
		}
	}

	if (i != value.size())
		return false;

	double factor = 1.0;
	for (int n = 0; n < exponent; ++n)
		factor *= 10.0;
	if (exponentNegative)
		result /= factor;
	else
		result *= factor;

	if (negative)
		result = -result;

	out = (float)result;
	return true;
}

static void fillEndpoint(uint32 lineNumber,
                         const Common::Array<Common::String> &tokens,
                         ShapeEndpoint &endpoint) {
	endpoint = ShapeEndpoint();
	endpoint.lineNumber = lineNumber;
	endpoint.label = tokens.empty() ? Common::String() : tokens[0];

	for (uint32 i = 1; i < tokens.size(); ++i)
		endpoint.rawValues.push_back(tokens[i]);

	if (endpoint.rawValues.size() != 3)
		return;

	float x = 0.0f, y = 0.0f, z = 0.0f;
	if (!parseFloatToken(endpoint.rawValues[0], x) ||
	    !parseFloatToken(endpoint.rawValues[1], y) ||
	    !parseFloatToken(endpoint.rawValues[2], z))
		return;

	endpoint.numeric = true;
	endpoint.value = ShapeVec3(x, y, z);
}

static bool startsNewTopLevel(const Common::Array<Common::String> &tokens) {
	if (tokens.empty())
		return false;
	return tokens[0].equalsIgnoreCase("ge_Shape") ||
	       tokens[0].equalsIgnoreCase("ge_Polygon") ||
	       tokens[0].equalsIgnoreCase("ge_Vector");
}

} // namespace

void ShapeGeometryDocument::clear() {
	_shapes.clear();
}

const ShapeGeometryRecord *ShapeGeometryDocument::shape(
		const Common::String &name) const {
	for (uint32 i = 0; i < _shapes.size(); ++i) {
		if (_shapes[i].name.equalsIgnoreCase(name))
			return &_shapes[i];
	}
	return nullptr;
}

bool ShapeGeometryParser::parse(const Common::String &decodedShapeScript,
                                ShapeGeometryDocument &out,
                                Common::String &errorMessage) const {
	out.clear();
	errorMessage.clear();

	if (decodedShapeScript.empty()) {
		errorMessage = "Shape script is empty";
		return false;
	}

	int32 currentShape = -1;
	uint32 physicalLine = 1;
	uint32 lineStart = 0;

	while (lineStart <= decodedShapeScript.size()) {
		uint32 lineEnd = lineStart;
		while (lineEnd < decodedShapeScript.size() &&
		       decodedShapeScript[lineEnd] != '\r' &&
		       decodedShapeScript[lineEnd] != '\n') {
			++lineEnd;
		}

		const Common::String raw =
			decodedShapeScript.substr(lineStart, lineEnd - lineStart);
		const Common::String code = stripComment(raw);

		if (!code.empty()) {
			Common::Array<Common::String> tokens;
			tokenizeLine(code, tokens);

			if (!tokens.empty() &&
			    tokens[0].equalsIgnoreCase("ge_Shape")) {
				if (tokens.size() < 3) {
					errorMessage = Common::String::format(
						"ge_Shape on line %u is incomplete",
						(uint)physicalLine);
					return false;
				}

				ShapeGeometryRecord shape;
				shape.lineNumber = physicalLine;
				shape.name = tokens[1];
				shape.rawType = tokens[2];
				shape.kind = shapeDefinitionKind(tokens[2]);
				out._shapes.push_back(shape);
				currentShape = (int32)out._shapes.size() - 1;
			} else if (startsNewTopLevel(tokens)) {
				currentShape = -1;
			} else if (currentShape >= 0 &&
			           !tokens.empty() &&
			           (tokens[0].equalsIgnoreCase("A") ||
			            tokens[0].equalsIgnoreCase("B"))) {
				ShapeGeometryRecord &shape = out._shapes[currentShape];
				if (tokens[0].equalsIgnoreCase("A")) {
					if (shape.hasA) {
						errorMessage = Common::String::format(
							"Shape '%s' has more than one A record",
							shape.name.c_str());
						return false;
					}
					fillEndpoint(physicalLine, tokens, shape.a);
					shape.hasA = true;
				} else {
					if (shape.hasB) {
						errorMessage = Common::String::format(
							"Shape '%s' has more than one B record",
							shape.name.c_str());
						return false;
					}
					fillEndpoint(physicalLine, tokens, shape.b);
					shape.hasB = true;
				}
			}
		}

		if (lineEnd >= decodedShapeScript.size())
			break;

		if (decodedShapeScript[lineEnd] == '\r' &&
		    lineEnd + 1 < decodedShapeScript.size() &&
		    decodedShapeScript[lineEnd + 1] == '\n') {
			lineStart = lineEnd + 2;
		} else {
			lineStart = lineEnd + 1;
		}
		++physicalLine;
	}

	if (out._shapes.empty()) {
		errorMessage = "No ge_Shape declarations found";
		return false;
	}

	return true;
}

bool resolveCharacterStartGeometry(
		const Common::String &characterName,
		const Common::String &helperName,
		const ShapeGeometryDocument &geometry,
		CharacterStartGeometry &out,
		Common::String &errorMessage) {
	out = CharacterStartGeometry();
	errorMessage.clear();

	const ShapeGeometryRecord *shape = geometry.shape(helperName);
	if (!shape) {
		errorMessage = Common::String::format(
			"Start helper '%s' for character '%s' has no geometry record",
			helperName.c_str(), characterName.c_str());
		return false;
	}
	if (shape->kind != kShapeDefinitionPosition) {
		errorMessage = Common::String::format(
			"Start helper '%s' for character '%s' is %s, not Position",
			helperName.c_str(), characterName.c_str(),
			shapeDefinitionKindName(shape->kind));
		return false;
	}

	out.characterName = characterName;
	out.helperName = helperName;
	out.geometry = shape;
	return true;
}

} // End of namespace ZeroComico
