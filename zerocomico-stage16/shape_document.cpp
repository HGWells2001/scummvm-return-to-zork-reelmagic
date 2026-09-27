/* ScummVM - Graphic Adventure Engine
 *
 * Experimental Zero Comico engine work.
 */

#include "common/tokenizer.h"

#include "zerocomico-stage16/shape_document.h"

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
			Common::String result = line.substr(0, i);
			result.trim();
			return result;
		}
	}
	Common::String result = line;
	result.trim();
	return result;
}

static void tokenizeLine(const Common::String &line,
                         Common::Array<Common::String> &tokens) {
	tokens.clear();
	Common::StringTokenizer tokenizer(line);
	while (!tokenizer.empty()) {
		Common::String token = cleanToken(tokenizer.nextToken());
		if (!token.empty())
			tokens.push_back(token);
	}
}

} // namespace

ShapeDefinitionKind shapeDefinitionKind(const Common::String &type) {
	if (type.equalsIgnoreCase("Position"))
		return kShapeDefinitionPosition;
	if (type.equalsIgnoreCase("Portal"))
		return kShapeDefinitionPortal;
	if (type.equalsIgnoreCase("Range"))
		return kShapeDefinitionRange;
	if (type.equalsIgnoreCase("Entity"))
		return kShapeDefinitionEntity;
	return kShapeDefinitionUnknown;
}

const char *shapeDefinitionKindName(ShapeDefinitionKind kind) {
	switch (kind) {
	case kShapeDefinitionPosition: return "Position";
	case kShapeDefinitionPortal: return "Portal";
	case kShapeDefinitionRange: return "Range";
	case kShapeDefinitionEntity: return "Entity";
	default: return "Unknown";
	}
}

void ShapeScriptDocument::clear() {
	_shapes.clear();
	_vectors.clear();
}

const ShapeDefinition *ShapeScriptDocument::shape(
		const Common::String &name) const {
	for (uint32 i = 0; i < _shapes.size(); ++i) {
		if (_shapes[i].name.equalsIgnoreCase(name))
			return &_shapes[i];
	}
	return nullptr;
}

const VectorDefinition *ShapeScriptDocument::vector(
		const Common::String &name) const {
	for (uint32 i = 0; i < _vectors.size(); ++i) {
		if (_vectors[i].name.equalsIgnoreCase(name))
			return &_vectors[i];
	}
	return nullptr;
}

bool ShapeScriptParser::parse(const Common::String &decodedShapeScript,
                              ShapeScriptDocument &out,
                              Common::String &errorMessage) const {
	out.clear();
	errorMessage.clear();

	if (decodedShapeScript.empty()) {
		errorMessage = "Shape script is empty";
		return false;
	}

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

				ShapeDefinition shape;
				shape.lineNumber = physicalLine;
				shape.name = tokens[1];
				shape.rawType = tokens[2];
				shape.kind = shapeDefinitionKind(tokens[2]);
				for (uint32 i = 3; i < tokens.size(); ++i)
					shape.declarationTail.push_back(tokens[i]);
				out._shapes.push_back(shape);
			} else if (!tokens.empty() &&
			           tokens[0].equalsIgnoreCase("ge_Vector")) {
				if (tokens.size() < 2) {
					errorMessage = Common::String::format(
						"ge_Vector on line %u has no name",
						(uint)physicalLine);
					return false;
				}

				VectorDefinition vector;
				vector.lineNumber = physicalLine;
				vector.name = tokens[1];
				for (uint32 i = 2; i < tokens.size(); ++i)
					vector.declarationTail.push_back(tokens[i]);
				out._vectors.push_back(vector);
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

	if (out._shapes.empty() && out._vectors.empty()) {
		errorMessage = "No ge_Shape or ge_Vector declarations found";
		return false;
	}

	return true;
}

} // End of namespace ZeroComico
