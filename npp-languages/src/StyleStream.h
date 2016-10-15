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
class IDocument; //Scintilla
/**Interface for IDocument to read source text and write styles.*/
class StyleStream
{
public:
	StyleStream(IDocument *doc);

	/**Sends any remainig style output to notepad++.*/
	void finish();
	/**@return True if _srcPos == _len*/
	bool eof()const
	{
		assert(_srcPos <= _len);
		return _srcPos == _len;
	}

	/**Get the previous source element.*/
	char prev()const
	{
		return _srcPos > 0 ? _src[_srcPos - 1] : '\0';
	}
	/**Get the next source element, but do not advance.*/
	char peek()const
	{
		assert(!eof());
		return _src[_srcPos];
	}
	/**Peeks further ahead. Returns '\0' if out of range.*/
	char peek(unsigned offset)const
	{
		if (_srcPos + offset >= _len) return '\0';
		else return _src[_srcPos + offset];
	}
	/**Reads a word ahead, but does not advance.
	 * A word end on any ASCII non-alpha numeric element except '_'.
	 */
	std::string peekWord()const;
	/**True if upcoming text matches.*/
	bool matches(const char *str)const;

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
	 * Ends on the first '.', '#', '{', ' ', '\t', '=' or on the line end.
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
private:
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
	std::unique_ptr<char[]> _styles;
	/**Source text to style.*/
	std::unique_ptr<char[]> _src;

	/**Fold indent of _line - 1.*/
	int _prevLineIndent;
	/**Fold indent of _line.*/
	int _curLineIndent;

	/**Advancing to next line, update state.*/
	void nextLine();
};
