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
class IDocument; //Scintilla
class SubStyleStream;
/**Interface for IDocument to read source text and write styles.*/
class StyleStream
{
public:
	/**Create a StyleStream for an entire document.*/
	StyleStream() {}

	StyleStream(const StyleStream&) = delete;
	StyleStream(StyleStream&&) = default;
	StyleStream& operator = (const StyleStream&) = delete;
	StyleStream& operator = (StyleStream&&) = default;

	/**@return True if _srcPos == _len*/
	bool eof()
	{
		assert(_srcPos <= _len);
		if (_srcPos == _len)
		{
			return onEof();
		}
		else return false;
	}

	/**Get the previous source element.*/
	char prev()
	{
		return _srcPos > 0 ? _src[_srcPos - 1] : '\0';
	}
	/**Get the next source element, but do not advance.*/
	char peek()
	{
		assert(!eof());
		return _src[_srcPos];
	}
	/**Peeks further ahead. Returns '\0' if out of range.*/
	char peek(unsigned offset)
	{
		if (_srcPos + offset >= _len) return '\0';
		else return _src[_srcPos + offset];
	}
	/**Reads a word ahead, but does not advance.
	 * A word end on any ASCII non-alpha numeric element except '_'.
	 */
	std::string peekWord();
	/**True if upcoming text matches.*/
	bool matches(const char *str);

	/**Advance a byte (e.g. after peek).
	 * _stylePos must equal _srcPos.
	 */
	void advance(char style)
	{
		assert(!eof());
		assert(_stylePos == _srcPos);
		_styles[_stylePos] = style;
		++_stylePos;
		++_srcPos;
	}
	/**Advances len bytes (e.g. after peek workd).*/
	void advance(char style, size_t len)
	{
		for (size_t i = 0; i < len; ++i) advance(style);
	}

	/**Start a line and return its indent level.
	 * The current _srcPos should be a line start, or the results will be incorrect.
	 * _stylePos must equal _srcPos, and style will be advanced by setting the default style (0).
	 * Tabs and spaces are used for indent, tabs counting as one space.
	 * @return The number of indents processed.
	 */
	unsigned nextIndent();
	/**Style the results from peekWord.*/
	void nextWord(char style)
	{
		auto w = peekWord();
		advance(style, w.size());
	}
	/**Style past any spaces and tabs.
	 * _stylePos must equal _srcPos.
	 */
	void nextSpaces()
	{
		assert(_stylePos == _srcPos);
		for (; _srcPos < _len && (_src[_srcPos] == ' ' || _src[_srcPos] == '\t'); ++_srcPos, ++_stylePos)
		{
			_styles[_stylePos] = 0;
		}
	}

	/**Reads and highlights a XML, HTML or CSS name.
	 *
	 * _stylePos must equal _srcPos.
	 * Ends on the first '.', '#', '{', '(', '[', ' ', '\t', '=', ''', '"', '>' or on the line end.
	 * Note that is is far less strict than the actual languages themselves.
	 *
	 * For "<htmltag>", "tag#id.class attr" and other such syntax.
	 */
	void nextXmlName(char style);

	/**Advance to the next line, setting its style.
	 * _stylePos must equal _srcPos.
	 */
	void readRestOfLine(char style);

	/**Set the current lines fold level by its indent.*/
	void foldLevel(int indent);

	/**Delegate a number of bytes to a substream.*/
	void subStream(SubStyleStream &sub, unsigned n);
protected:
	IDocument *_doc;
	//TODO: Investigate different strategies here
	//The basic assumption with this implementation is that reading and writing single bytes to
	//Notepad++ will be slow, and that documents will never be large enough that making complete
	//copies will be an issue.
	/**Position within the document where _styles and _str start.*/
	unsigned _docPos;
	/**Line number in document.*/
	unsigned _line;
	/**Length of _styles and _str.*/
	unsigned _len;
	/**Current style position in _styles.*/
	unsigned _stylePos;
	/**Current read position in _str.*/
	unsigned _srcPos;
	/**Styles for each source text byte.*/
	char *_styles;
	/**Source text to style.*/
	char *_src;

	/**Fold indent of _line - 1.*/
	int _prevLineIndent;
	/**Fold indent of _line.*/
	int _curLineIndent;

	/**Advancing to next line, update state.*/
	void nextLine();

	/**Called when the stream reaches the end, to provide new content.*/
	virtual bool onEof() { return true; }
};

/**Interface for IDocument to read source text and write styles.*/
class DocumentStyleStream : public StyleStream
{
public:
	DocumentStyleStream(IDocument *doc);
	~DocumentStyleStream();
	/**Sends any remainig style output to notepad++.*/
	void finish();
};

class SubStyleStream : public StyleStream
{
public:
	typedef std::function<void(SubStyleStream&)> Next;
	SubStyleStream(Next next) : StyleStream(), _next(next) {}
private:
	Next _next;

	virtual bool onEof()override;
};
