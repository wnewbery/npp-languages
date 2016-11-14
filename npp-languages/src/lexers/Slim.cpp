// This file is part of Extra Languages for Notepad++.
// Copyright (C) 2016 William Newbery <wnewbery@hotmail.co.uk>
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>.

#include "Slim.h"
#include <unordered_set>

namespace
{
	//TODO: Use Notepad++ keywords
	std::unordered_set<std::string> ENGINES =
	{
		"ruby", "javascript", "css", "sass", "scss", "less", "styl", "coffee", "asciidoc",
		"markdown", "textile", "creole", "wiki", "mediawiki", "rdoc", "nokogiri", "none"
	};
	// Engines with interpolation done by Slim
	std::unordered_set<std::string> INTERPOLATED_ENGINES =
	{
		"markdown"
	};
}

void Slim::style(StyleStream &stream)
{
	_currentIndent = stream.advanceIndent();
	while (!stream.eof()) line(stream);
}
void SCI_METHOD Slim::Lex(unsigned int startPos, int lengthDoc, int initStyle, IDocument *doc)
{
	//return BaseLexer::Lex(startPos, lengthDoc, initStyle, doc);
	int startLine = doc->LineFromPosition(startPos);
	if (startLine > 0) --startLine; //if the edited lines indent changed, then its meaning may depend on the previous item
	while (startLine > 0 && doc->GetLineState(startLine) != SAFE_START) --startLine;
	unsigned actualStartPos = (unsigned)doc->LineStart(startLine);
	assert(actualStartPos <= startPos);

	unsigned end = startPos + lengthDoc;
	end = doc->LineStart(doc->LineFromPosition(startPos + lengthDoc) + 1);

	char last[100];
	doc->GetCharRange(last, end - 10, 100);

	DocumentStyleStream stream(doc, startLine, end - actualStartPos);
	style(stream);
}

void Slim::line(StyleStream &stream)
{
	stream.lineState(SAFE_START);
	stream.foldIndent(_currentIndent);
	if (stream.eof()) return;
	switch (stream.peek())
	{
	case '/':
		if (stream.peek(1) == '!') return htmlCommentBlock(stream);
		else if (stream.peek(1) == '[') return ieConditionalCommentBlock(stream);
		else return commentBlock(stream);
	case '<': return textBlock(stream);
	case '|':
	case '\'':
		stream.advance(OPERATOR);
		return textBlock(stream);
	case '=': return rubyLine(stream);
	case '-': return rubyLine(stream);
	case '*': return dynamicTag(stream);
	case '#': return tagId(stream);
	case '.': return tagCls(stream);
	default:
	{
		auto n = xmlNameLen(stream);
		if (stream.peek(n) == ':')
		{
			auto name = stream.peekStr(n);
			if (ENGINES.count(name)) return filterBlock(stream, name);
			else return tagName(stream);
		}
		else if (stream.matches("include"))
			return includeLine(stream);
		else if (stream.matches("doctype"))
			return doctypeLine(stream);
		else return tagName(stream);
	}
	}
}
void Slim::tagOrFilter(StyleStream &stream)
{
	auto n = xmlNameLen(stream);
	if (stream.peek(n) == ':')
	{
		auto name = stream.peekStr(n);
		if (ENGINES.count(name)) return filterBlock(stream, name);
		else return tagName(stream);
	}
	else return tagName(stream);
}

void Slim::commentBlock(StyleStream &stream, Style style)
{
	while (!stream.eof())
	{
		stream.advanceLine(style);
		unsigned indent = stream.advanceIndent();
		if (indent <= _currentIndent)
		{
			_currentIndent = indent;
			return;
		}
		stream.foldIndent(_currentIndent + 1);
	}
}
void Slim::commentBlock(StyleStream &stream)
{
	assert(stream.matches("/"));
	commentBlock(stream, COMMENT);
}
void Slim::htmlCommentBlock(StyleStream &stream)
{
	assert(stream.matches("/!"));
	commentBlock(stream, HTMLCOMMENT);
}
void Slim::ieConditionalCommentBlock(StyleStream &stream)
{
	assert(stream.matches("/["));
	commentBlock(stream, HTMLIECOMMENT);
}

void Slim::dynamicTag(StyleStream &stream)
{
	assert(stream.peek() == '*');
	stream.advance(TAG);
	_ruby.name(stream, (Ruby::Style)TAG, true);
	stream.advanceLine(DEFAULT);
	_currentIndent = stream.advanceNextIndent();
}
void Slim::tagStart(StyleStream &stream)
{
	stream.advanceSpTab();
	if (stream.eof()) return;
	char c = stream.peek();
	if (c == '#') return tagId(stream);
	if (c == '.') return tagCls(stream);
	while (true)
	{
		if (c == '<' || c == '>')
		{
			stream.advance(OPERATOR);
			if (stream.eof()) return;
			c = stream.peek();
		}
		else break;
	}
	stream.advanceSpTab();

	c = stream.peek();
	if (c < 0) return;
	else if (c == '(' || c == '[' || c == '{') tagWrappedAttrs(stream);
	else tagAttrs(stream);
}
void Slim::tagName(StyleStream &stream)
{
	advanceXmlName(stream, TAG);
	tagStart(stream);
}
void Slim::tagId(StyleStream &stream)
{
	assert(stream.peek() == '#');
	stream.advance(ID);
	advanceXmlName(stream, ID);
	tagStart(stream);
}
void Slim::tagCls(StyleStream &stream)
{
	assert(stream.peek() == '.');
	stream.advance(CLS);
	advanceXmlName(stream, CLS);
	tagStart(stream);
}
void Slim::tagAttrs(StyleStream &stream)
{
	while (true)
	{
		stream.advanceSpTab();
		char c = stream.peek();
		if (c < 0) return;
		else if (c == '=')
		{
			return tagEnd(stream);
		}
		else if (c == '*')
		{
			splatAttrs(stream);
		}
		else
		{
			auto n = xmlNameLen(stream);
			if (stream.peek(n) == '=')
			{
				stream.advance(HTMLATTRIBUTE, n);
				stream.advance(OPERATOR);
				if (stream.peek() == '=') stream.advance(OPERATOR);
				attrValue(stream);
			}
			else return tagEnd(stream);
		}
	}
}
void Slim::tagWrappedAttrs(StyleStream &stream)
{
	char delimL = stream.peek();
	char delimR;
	int depth = 1;
	switch (delimL)
	{
	case '(': delimR = ')'; break;
	case '[': delimR = ']'; break;
	case '{': delimR = '}'; break;
	default: assert(false); return;
	}
	stream.advance(OPERATOR);

	while (depth)
	{
		stream.advanceSpTab();
		auto c = stream.peek();
		if (c < 0) return;
		else if (c == delimL)
		{
			++depth;
			stream.advance(OPERATOR);
		}
		else if (c == delimR)
		{
			--depth;
			stream.advance(OPERATOR);
		}
		else if (c == '\r' || c == '\n')
		{
			stream.advanceEol();
		}
		else
		{
			auto n = xmlNameLen(stream);
			if (n == 0) stream.advance(ERROR);
			else
			{
				stream.advance(HTMLATTRIBUTE, n);
				stream.advanceSpTab();
				if (stream.peek() != '=') continue;
				else
				{
					stream.advance(OPERATOR);
					if (stream.peek() == '=') stream.advance(OPERATOR);
					stream.advanceSpTab();
					attrValue(stream);
				}
			}
		}
	}
	tagEnd(stream);
}
void Slim::tagEnd(StyleStream &stream)
{
	stream.advanceSpTab();
	char c = stream.peek();
	if (c == ':')
	{
		stream.advance(OPERATOR);
		stream.advanceSpTab();
		return tagOrFilter(stream);
	}
	else if (c == '=')
	{
		stream.advance(OPERATOR);
		return rubyLine(stream);
	}
	else return textLine(stream);
}
void Slim::splatAttrs(StyleStream &stream)
{
	stream.advance(HTMLATTRIBUTE);
	attrValue(stream);
}
void Slim::attrValue(StyleStream &stream)
{
	int delimL = -1, delimR = -1;
	int depth = 1;
	unsigned n = 0;
	auto c = stream.peek();
	switch (c)
	{
	case '\'': delimL = '\''; delimR = '\''; ++n; break;
	case '\"': delimL = '\"'; delimR = '\"'; ++n; break;
	case '[': delimL = '['; delimR = ']'; ++n; break;
	case '{': delimL = '{'; delimR = '}'; ++n; break;
	case '(': delimL = '('; delimR = ')'; ++n; break;
	}

	if (delimL >= 0)
	{
		while (depth)
		{
			c = stream.peek(n);
			if (c < 0) break;
			else if (c == delimR) --depth;
			else if (c == delimL) ++depth;
			++n;
		}
	}
	else //until space
	{
		while (true)
		{
			c = stream.peek(n);
			if (c < 0 || c == ' ' || c == '\t' || c == '\r' || c == '\n') break;
			++n;
		}
	}

	StyleStream sub;
	sub.addSection(stream, n);
	_ruby.style(sub);
}

void Slim::textBlock(StyleStream &stream)
{
	StyleStream htmlStream;
	unsigned indent = _currentIndent;
	while (true)
	{
		auto interp = _ruby.findNextInterp(stream);
		if (interp > 0) htmlStream.addSection(stream, interp);

		auto c = stream.peek();
		if (c == '#') _ruby.stringInterp(stream);
		else if (c < 0) break;
		else
		{
			assert(c == '\r' || c == '\n');
			stream.advanceEol();
			indent = stream.advanceNextIndent();
			if (indent <= _currentIndent)
			{
				_currentIndent = indent;
				break;
			}
		}
	}
	_html.line(htmlStream);
}

void Slim::rubyBlock(StyleStream &stream)
{
	unsigned indent = _currentIndent;
	unsigned n = 0;
	while (true)
	{
		auto n2 = stream.fullLineLen(n);
		assert(n2 > 0 || stream.peek(n) < 0);
		if (!n2) break;
		n += n2;

		indent = stream.countSp(n);
		n += indent;
		if (indent <= _currentIndent) break;
	}
	if (n > 0)
	{
		_currentIndent = indent;
		StyleStream blockStream;
		blockStream.addSection(stream, n);
		_ruby.style(blockStream);
	}
}

void Slim::filterBlock(StyleStream &stream, const std::string &engine)
{
	assert(stream.matches(engine.c_str()) && stream.peek((unsigned)engine.size()) == ':');
	stream.advance(FILTER, engine.size() + 1);
	unsigned indent = _currentIndent;
	StyleStream blockStream(stream);

	bool interpolate = INTERPOLATED_ENGINES.count(engine) > 0;
	// Create stream of segments
	while (true)
	{
		if (interpolate)
		{
			while (true)
			{
				unsigned i = _ruby.findNextInterp(stream);
				if (stream.peekEol(i))
				{
					blockStream.addLineWithEol(stream); // Rest of this line
					break;
				}
				else
				{
					if (i > 0) blockStream.addSection(stream, i); // Text before
					assert(stream.peek() == '#' && stream.peek(1) == '{');
					_ruby.stringInterp(stream);
				}
			}
		}
		else blockStream.addLineWithEol(stream); // Rest of this line
		if (stream.peekNextIndent() <= indent)
		{
			_currentIndent = stream.advanceNextIndent();
			break;
		}
		else
		{
			stream.advance(FILTER, indent);
		}
	}

	blockStream.foldHeader(0);
	blockStream.baseFoldLevel(indent + 1);
	blockStream.foldNext(1);

	if (engine == "ruby") _ruby.style(blockStream);
	else if (engine == "markdown") _markdown.style(blockStream);
	else if (engine == "scss") _scss.style(blockStream);
	else while (!blockStream.eof()) blockStream.advanceLine(FILTER);
}

void Slim::includeLine(StyleStream &stream)
{
	stream.advanceLine(INCLUDE);
	_currentIndent = stream.advanceNextIndent();
}
void Slim::doctypeLine(StyleStream &stream)
{
	stream.advanceLine(DOCTYPE);
	_currentIndent = stream.advanceNextIndent();
}
void Slim::textLine(StyleStream &stream)
{
	StyleStream htmlStream;
	unsigned indent = _currentIndent;
	while (true)
	{
		auto interp = _ruby.findNextInterp(stream);
		if (interp > 0) htmlStream.addSection(stream, interp);

		auto c = stream.peek();
		if (c == '#') _ruby.stringInterp(stream);
		else if (c < 0) break;
		else
		{
			assert(c == '\r' || c == '\n');
			stream.advanceEol();
			_currentIndent = stream.advanceNextIndent();
			break;
		}
	}
	_html.line(htmlStream);
}
void Slim::rubyLine(StyleStream &stream)
{
	StyleStream rbStream;
	while (true)
	{
		auto interp = _ruby.findNextInterp(stream);
		if (interp > 0) rbStream.addSection(stream, interp);

		auto c = stream.peek();
		if (c == '#') _ruby.stringInterp(stream);
		else if (c < 0) break;
		else
		{
			assert(c == '\r' || c == '\n');
			stream.advanceEol();
			_currentIndent = stream.advanceNextIndent();
			if (rbStream.last() != ',' && rbStream.last() != '\\')
			{
				break;
			}
		}
	}
	_ruby.style(rbStream);
}
