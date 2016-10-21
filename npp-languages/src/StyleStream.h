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

#pragma once
#include <memory>
#include <cassert>
#include <string>
#include <functional>
#include <vector>
class IDocument; //Scintilla

inline bool isAlphaNumeric(int c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/**Interface for lexer to read source text and write styles.
 * Able to store a number of sections that are transparent to the streams operation.
 */
class StyleStream
{
public:
	enum SingleLineTag { singleLineTag };

	StyleStream() : _sections(), _section(0), _pos(0), _line(0) {}
	StyleStream(StyleStream &stream, SingleLineTag)
		: StyleStream()
	{
		addLine(stream);
	}
	StyleStream(char *src, char *styles, unsigned len)
		: StyleStream()
	{
		Section sec = {src, styles, len};
		_sections.push_back(sec);
	}

	void addLine(StyleStream &stream)
	{
		addSection(stream, stream.lineLen());
		stream.advanceLine(0);
	}
	void addLineWithEol(StyleStream &stream)
	{
		addSection(stream, stream.fullLineLen());
	}
	void addSection(StyleStream &stream, unsigned len)
	{
		while (len > 0)
		{
			assert(!stream.eof());
			const auto &sec = stream._sections[stream._section];
			Section newSec = {sec._src + stream._pos, sec._styles + stream._pos, stream._line};
			unsigned remaining = sec._len - stream._pos;
			if (len <= remaining) newSec._len = len;
			else newSec._len = remaining;

			assert(newSec._len > 0 && newSec._len <= len);
			_sections.push_back(newSec);
			stream.skip(newSec._len);
			len -= newSec._len;
		}
	}

	bool eof()const
	{
		return _section >= _sections.size();
	}
	int peek(unsigned p = 0)const
	{
		if (eof()) return -1;

		int section = _section;
		int pos = _pos;
		unsigned i = 0;
		while (true)
		{
			if (pos == _sections[section]._len)
			{
				++section;
				pos = 0;
			}
			if (section >= _sections.size()) return -1;
			if (i == p) break;
			++i;
			++pos;
		}
		return _sections[section]._src[pos];
	}
	std::string peekStr(unsigned len)const
	{
		std::string ret;
		ret.reserve(len);
		for (unsigned i = 0; i < len; ++i)
		{
			auto c = peek(i);
			assert(c >= 0);
			ret.push_back((char)c);
		}
		return ret;
	}
	template<typename F>
	std::string peekStr(F f, unsigned start = 0)const
	{
		std::string ret;
		unsigned p = 0;
		while (true)
		{
			auto c = peek(p + start);
			if (c < 0 || !f((char)c)) return ret;
			ret.push_back((char)c);
			++p;
		}
	}
	int peekAfterSpTab(unsigned start)
	{
		while (true)
		{
			auto c = peek(start);
			if (c == ' ' || c == '\t') ++start;
			else return c;
		}
	}
	int last()const
	{
		if (_sections.empty() || _sections.back()._len == 0) return -1;
		auto &sec = _sections.back();
		return sec._src[sec._len - 1];
	}

	//src lookaheads
	bool matches(char c, unsigned n)const
	{
		for (unsigned p = 0; p < n; ++p)
		{
			if (peek(p) != c) return false;
		}
		return true;
	}
	bool matches(const char *str, unsigned start = 0)const
	{
		for (int i = 0; *str; ++str, ++i)
		{
			if (peek(i + start) != *str) return false;
		}
		return true;
	}
	unsigned countSp(unsigned start = 0)const
	{
		return countChr(' ', start);
	}
	unsigned countChr(char c, unsigned start = 0)const
	{
		unsigned p = 0;
		while (peek(p + start) == c) ++p;
		return p;
	}
	template<typename F>
	unsigned countMatches(F f, unsigned start = 0)
	{
		unsigned p = 0;
		while (true)
		{
			auto c = peek(p + start);
			if (c < 0 || !f((char)c)) return p;
			else ++p;
		}
	}
	bool isWsAt(unsigned p)const
	{
		auto c = peek(p);
		return c == ' ' || c == '\t';
	}
	bool lineContains(char c, unsigned p = 0)const
	{
		while (true)
		{
			auto c2 = peek(p++);
			if (c2 < 0 || c2 == '\r' || c2 == '\n') return false;
			if (c2 == c) return true;
		}
	}
	bool isBlankLine(unsigned start = 0)const
	{
		while (true)
		{
			auto c = peek(start++);
			if (c < 0 || c == '\r' || c == '\n') return true;
			if (c != ' ' && c != '\t') return false;
		}
	}

	unsigned lineLen(unsigned start = 0)const
	{
		unsigned p = start;
		while (true)
		{
			auto c2 = peek(p);
			if (c2 < 0 || c2 == '\r' || c2 == '\n') break;
			++p;
		}
		return p - start;
	}
	unsigned eolLen(unsigned start = 0)const
	{
		auto c = peek(start);
		if (c < 0) return 0; //end of document section
		if (c == '\n') return 1; //\n
		assert(c == '\r');
		if (peek(start + 1) == '\n') return 2; //\r\n
		return 1; //\r
	}
	unsigned fullLineLen(unsigned start = 0)const
	{
		unsigned n = lineLen(start);
		return n + eolLen(start + n);
	}
	//style
	/**Style next n elements.*/
	void advance(char style, size_t n = 1)
	{
		for (size_t i = 0; i < n; ++i)
		{
			assert(!eof());
			assert(peek() != '\r' && peek() != '\n');
			_sections[_section]._styles[_pos] = (char)style;
			++_pos;
			nextSection();
		}
	}
	/**Style rest of line.*/
	void advanceLine(char style, char eolStyle = 0)
	{
		while (true)
		{
			auto c = peek();
			if (c < 0) return;
			else if (c == '\r' || c == '\n') return advanceEol(style);
			else advance(style);
		}
	}
	void advanceWithEol(char style, unsigned n = 1)
	{
		for (unsigned i = 0; i < n;)
		{
			assert(!eof());
			if (peek() == '\r' || peek() == '\n')
			{
				i += eolLen();
				assert(i <= n);
				advanceEol(style);
			}
			else
			{
				advance(style);
				++i;
			}
		}
	}
	/**Style EOL and update line number.*/
	void advanceEol(char style = 0)
	{
		assert(!eof());
		auto c = peek();
		if (c == '\n')
		{
			_sections[_section]._styles[_pos] = style;
			++_line;
			nextSection();
		}
		else
		{
			assert(c == '\r');
			++_line;
			_sections[_section]._styles[_pos] = style;
			++_pos;
			nextSection();
			if (peek() == '\n')
			{
				_sections[_section]._styles[_pos] = style;
				++_pos;
				nextSection();
			}
			return;
		}
	}
	/**Style ' ' and '\t'*/
	void advanceSpTab(char style = 0)
	{
		while (true)
		{
			switch (peek())
			{
			case ' ':
			case '\t':
				advance(style);
				break;
			default:
				return;
			}
		}
	}
	/**Style ' ' and '\t' and return the count.*/
	unsigned advanceIndent(char style = 0)
	{
		unsigned n = 0;
		while (true)
		{
			auto c = peek();
			if (c == ' ' || c == '\t')
			{
				++n;
				advance(style);
			}
			else break;
		}
		return n;
	}
	/**Style ' ', '\t' and blank lines, and return the next indent count.*/
	unsigned advanceNextIndent(char style = 0)
	{
		if (eof()) return 0;

		//assert(_srcPos == 0 || _src[_srcPos - 1] == '\r' || _src[_srcPos - 1] == '\n');
		//skip blank lines using default style, they are not significant
		//return indent of first non blank line
		unsigned indent = 0;
		while (true)
		{
			char c = peek();
			if (c == '\n')
			{
				advanceEol();
				indent = 0;
			}
			else if (c == '\r')
			{
				advanceEol();
				indent = 0;
			}
			else if (c == ' ' || c == '\t')
			{
				advance(0);
				++indent;
			}
			else return indent;
		}
	}
	template<typename F>
	void advanceMatches(F f, char style)
	{
		while (true)
		{
			auto c = peek();
			if (c < 0 || !f((char)c)) return;
			else advance(style);
		}
	}
	//fold current line
	void foldLevel(int level) {}
protected:
	struct Section
	{
		char *_src;
		char *_styles;
		unsigned _len;
		/**First line number.*/
		unsigned _line;
	};
	std::vector<Section> _sections;
	/**Current position in _sections.*/
	unsigned _section;
	/**Current position in _sections[_section] _src and _styles.*/
	unsigned _pos;
	/**Current document line number.*/
	unsigned _line;
	IDocument *_doc;

	/**Moves to next _section if _pos reached the end.*/
	void nextSection()
	{
		assert(!eof());
		if (_pos == _sections[_section]._len)
		{
			++_section;
			_pos = 0;
			if (!eof()) _line = _sections[_section]._line;
		}
	}
	/**Advance without styling. Used when creating sub streams for other languages.*/
	void skip(unsigned n = 1)
	{
		for (unsigned i = 0; i < n;)
		{
			assert(!eof());
			char c = peek();
			if (c == '\r' || c == '\n')
			{
				i += eolLen();
				assert(i <= n);
				advanceEol(); //TODO: this will set styles, which is wasted effort
			}
			else
			{
				++_pos;
				++i;
				nextSection();
			}
		}
	}
};

/**Interface for IDocument to read source text and write styles.*/
class DocumentStyleStream : public StyleStream
{
public:
	DocumentStyleStream(IDocument *doc);
	~DocumentStyleStream();
private:
	IDocument *_doc;
};
