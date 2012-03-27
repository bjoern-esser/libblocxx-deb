/*******************************************************************************
* Copyright (C) 2005, Vintela, Inc. All rights reserved.
* Copyright (C) 2006, Novell, Inc. All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
*     * Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of 
*       Vintela, Inc., 
*       nor Novell, Inc., 
*       nor the names of its contributors or employees may be used to 
*       endorse or promote products derived from this software without 
*       specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/


/**
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/LogMessagePatternFormatter.hpp"
#include "blocxx/String.hpp"
#include "blocxx/LogMessage.hpp"
#include "blocxx/StringBuffer.hpp"
#include "blocxx/IntrusiveCountableBase.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/ExceptionIds.hpp"
#include "blocxx/DateTime.hpp"
#include "blocxx/ThreadImpl.hpp"
#include "blocxx/GlobalString.hpp"

#include <vector>
#include <cstdlib> // for strtol
#include <climits> // for CHAR_MAX

#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

extern "C"
{
#include <errno.h>
}

namespace BLOCXX_NAMESPACE
{

BLOCXX_DEFINE_EXCEPTION_WITH_ID(LogMessagePatternFormatter);

namespace LogMessagePatternFormatterImpl // if this is anonymous, gcc 4.1.1 spits out a warning about using a data member from an unnamed namespace.
{

enum EJustificationFlag
{
	E_RIGHT_JUSTIFY,
	E_LEFT_JUSTIFY
};

struct Formatting
{
	int minWidth;
	int maxWidth;
	EJustificationFlag justification;

	static const int NO_MIN_WIDTH = -1;
	static const int NO_MAX_WIDTH = 0x7FFFFFFF;

	Formatting()
		: minWidth(NO_MIN_WIDTH)
		, maxWidth(NO_MAX_WIDTH)
		, justification(E_RIGHT_JUSTIFY)
	{}
};

} // end namespace LogMessagePatternFormatterImpl

using namespace LogMessagePatternFormatterImpl;

/////////////////////////////////////////////////////////////////////////////
class LogMessagePatternFormatter::Converter : public IntrusiveCountableBase
{
public:
	Converter()
	{}

	Converter(const Formatting& formatting)
		: m_formatting(formatting)
	{}
	
	virtual ~Converter() {}
	
	virtual void formatMessage(const LogMessage& message, StringBuffer& output) const
	{
		if ((m_formatting.minWidth == Formatting::NO_MIN_WIDTH) && (m_formatting.maxWidth == Formatting::NO_MAX_WIDTH))
		{
			convert(message, output);
		}
		else
		{
			StringBuffer buf;
			convert(message, buf);

			if (buf.length() == 0)
			{
				if (m_formatting.minWidth > 0)
				{
					output.append(&(std::vector<char>(size_t(m_formatting.minWidth), ' ')[0]), m_formatting.minWidth);
				}
				return;
			}

			int len = buf.length();
			if (len > m_formatting.maxWidth)
			{
				if (m_formatting.justification == E_LEFT_JUSTIFY)
				{
					buf.truncate(m_formatting.maxWidth);
					output += buf;
				}
				else
				{
					output += buf.releaseString().substring(len - m_formatting.maxWidth);
				}
			}
			else if (len < m_formatting.minWidth)
			{
				if (m_formatting.justification == E_LEFT_JUSTIFY)
				{
					output += buf;
					output.append(&(std::vector<char>(size_t(m_formatting.minWidth - len), ' ')[0]), m_formatting.minWidth - len);
				}
				else
				{
					output.append(&(std::vector<char>(size_t(m_formatting.minWidth - len), ' ')[0]), m_formatting.minWidth - len);
					output += buf;
				}
			}
			else
			{
				output += buf;
			}
		}
	}

	virtual void convert(const LogMessage& message, StringBuffer& output) const = 0;

private:
	Formatting m_formatting;

};

/////////////////////////////////////////////////////////////////////////////
const GlobalString LogMessagePatternFormatter::STR_DEFAULT_MESSAGE_PATTERN = BLOCXX_GLOBAL_STRING_INIT("%r [%t] %p %c - %m");

/////////////////////////////////////////////////////////////////////////////
LogMessagePatternFormatter::~LogMessagePatternFormatter()
{
}

/////////////////////////////////////////////////////////////////////////////
void
LogMessagePatternFormatter::formatMessage(const LogMessage& message, StringBuffer& output) const
{
	typedef Array<ConverterRef>::const_iterator iter_t;
	iter_t end(m_patternConverters.end());
	for (iter_t i(m_patternConverters.begin()); i != end; ++i)
	{
		(*i)->formatMessage(message, output);
	}
}

/////////////////////////////////////////////////////////////////////////////
namespace
{

typedef LogMessagePatternFormatter::Converter Converter;
typedef LogMessagePatternFormatter::ConverterRef ConverterRef;

/////////////////////////////////////////////////////////////////////////////
class MessageConverter : public Converter
{
public:
	MessageConverter(const Formatting& formatting)
		: Converter(formatting)
	{}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
		output += message.message;
	}
};

#define CDATA_START_DEF "<![CDATA["
#define CDATA_END_DEF "]]>"
#define CDATA_PSEUDO_END_DEF "]]&gt;"

GlobalString CDATA_START = BLOCXX_GLOBAL_STRING_INIT(CDATA_START_DEF);
GlobalString CDATA_END = BLOCXX_GLOBAL_STRING_INIT(CDATA_END_DEF);
GlobalString CDATA_PSEUDO_END = BLOCXX_GLOBAL_STRING_INIT(CDATA_PSEUDO_END_DEF);
GlobalString CDATA_EMBEDDED_END = BLOCXX_GLOBAL_STRING_INIT(CDATA_END_DEF CDATA_PSEUDO_END_DEF CDATA_START_DEF);

/////////////////////////////////////////////////////////////////////////////
class XMLMessageConverter : public Converter
{
public:
	XMLMessageConverter(const Formatting& formatting)
		: Converter(formatting)
	{}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
		output += CDATA_START;
		const String& msg(message.message);
		if (!msg.empty())
		{
			size_t end = msg.indexOf(CDATA_END);
			if (end == String::npos)
			{
				output += msg;
			}

			size_t start(0);
			while (end != String::npos)
			{
				output.append(&msg[start], end - start);
				output += CDATA_EMBEDDED_END;
				start = end + static_cast<String>(CDATA_END).length();
				if (start < msg.length())
				{
					end = msg.indexOf(CDATA_END, start);
				}
				else
				{
					break;
				}
			}
		}
		output += CDATA_END;
	}
};

/////////////////////////////////////////////////////////////////////////////
class LiteralConverter : public Converter
{
public:
	LiteralConverter(const String& literal)
		: m_literal(literal)
	{}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
		output += m_literal;
	}

private:
	String m_literal;
};

/////////////////////////////////////////////////////////////////////////////
class ThreadConverter : public Converter
{
public:
	ThreadConverter(const Formatting& formatting)
		: Converter(formatting)
	{}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
		output += ThreadImpl::thread_t_ToUInt64(ThreadImpl::currentThread());
	}
};

/////////////////////////////////////////////////////////////////////////////
class PidConverter : public Converter
{
public:
	PidConverter(const Formatting& formatting)
		: Converter(formatting)
	{}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
#ifdef BLOCXX_WIN32
		output += ::GetCurrentProcessId();
#else
		output += ::getpid();
#endif
	}
};

/////////////////////////////////////////////////////////////////////////////
class ComponentConverter : public Converter
{
public:
	ComponentConverter(const Formatting& formatting, int precision)
		: Converter(formatting)
		, m_precision(precision)
	{}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
		if (m_precision <= 0)
		{
			output += message.component;
		}
		else
		{
			const String& component(message.component);
			size_t len(component.length());
			size_t end(len - 1);
			for (int i = m_precision; i > 0; --i)
			{
				end = component.lastIndexOf('.', end - 1);
				if (end == String::npos)
				{
					output += component;
					return;
				}
			}
			output += component.substring(end + 1, len - (end + 1));
		}
	}

private:
	int m_precision;
};

/////////////////////////////////////////////////////////////////////////////
class FileLocationConverter : public Converter
{
public:
	FileLocationConverter(const Formatting& formatting)
		: Converter(formatting)
	{}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
		if (message.filename != 0)
		{
			output += message.filename;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
class FullLocationConverter : public Converter
{
public:
	FullLocationConverter(const Formatting& formatting)
		: Converter(formatting)
	{}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
		if (message.filename != 0)
		{
			output += message.filename;
			output += '(';
			output += message.fileline;
			output += ')';
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
class LineLocationConverter : public Converter
{
public:
	LineLocationConverter(const Formatting& formatting)
		: Converter(formatting)
	{}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
		output += message.fileline;
	}
};

/////////////////////////////////////////////////////////////////////////////
class MethodLocationConverter : public Converter
{
public:
	MethodLocationConverter(const Formatting& formatting)
		: Converter(formatting)
	{}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
		if (message.methodname != 0)
		{
			output += message.methodname;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
class CategoryConverter : public Converter
{
public:
	CategoryConverter(const Formatting& formatting)
		: Converter(formatting)
	{}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
		output += message.category;
	}
};

/////////////////////////////////////////////////////////////////////////////
class RelativeTimeConverter : public Converter
{
public:
	RelativeTimeConverter(const Formatting& formatting)
		: Converter(formatting)
	{}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
		output += getRelativeTime();
	}

private:
	static UInt64 getRelativeTime()
	{
		return getNowMillis() - startMillis;
	}

	static UInt64 startMillis;
public:
	static UInt64 getNowMillis()
	{
		DateTime now;
		now.setToCurrent();
		return UInt64(now.get()) * 1000 + (now.getMicrosecond() / 1000);
	}
};

UInt64 RelativeTimeConverter::startMillis(RelativeTimeConverter::getNowMillis());

/////////////////////////////////////////////////////////////////////////////
enum EParserState
{
    E_LITERAL_STATE,
    E_CONVERTER_STATE,
    E_DOT_STATE,
    E_MIN_STATE,
    E_MAX_STATE
};

/////////////////////////////////////////////////////////////////////////////
class DateConverter : public Converter
{
public:
	DateConverter(const Formatting& formatting, const String& format)
		: Converter(formatting)
		, m_format(format)
	{
		size_t pos = m_format.indexOf("%Q");
		if (pos != String::npos)
		{
			// escape the %Q, since strftime doesn't know about it.
			m_format = m_format.substring(0, pos) + '%' + m_format.substring(pos);
		}
	}

	virtual void convert(const LogMessage &message, StringBuffer &output) const
	{
		char buf[255];

		DateTime now;
		now.setToCurrent();
		struct tm nowTm;
		now.toLocal(nowTm);

 		size_t len = ::strftime(buf, sizeof(buf), m_format.c_str(), &nowTm);

		buf[len] = '\0';

		// handle %Q special case
		char* p = strstr(buf, "%Q");
		if (p != NULL)
		{
			*p = '\0';
			output += buf;
			long deciMillis = now.getMicrosecond() / 1000;
			String strMillis(deciMillis);
			// output 3 chars
			switch (strMillis.length())
			{
				case 1:
					output += '0';
				case 2:
					output += '0';
			}
			output += strMillis;
			output += p+2;
		}
		else
		{
			output += buf;
		}
	}

	static const char* const ISO8601_DATE_FORMAT;
	static const char* const ISO8601_PATTERN;
	static const char* const ABSOLUTE_DATE_FORMAT;
	static const char* const ABSOLUTE_PATTERN;
	static const char* const DATE_DATE_FORMAT;
	static const char* const DATE_PATTERN;

private:
	String m_format;
};

const char* const DateConverter::ISO8601_DATE_FORMAT = "ISO8601";
const char* const DateConverter::ISO8601_PATTERN = "%Y-%m-%d %H:%M:%S,%Q";
const char* const DateConverter::ABSOLUTE_DATE_FORMAT = "ABSOLUTE";
const char* const DateConverter::ABSOLUTE_PATTERN = "%H:%M:%S,%Q";
const char* const DateConverter::DATE_DATE_FORMAT = "DATE";
const char* const DateConverter::DATE_PATTERN = "%d %b %Y %H:%M:%S,%Q";

/////////////////////////////////////////////////////////////////////////////
class Parser
{
public:
	Parser(const String& pattern_)
		: i(0)
		, state(E_LITERAL_STATE)
		, pattern(pattern_)
	{}

/////////////////////////////////////////////////////////////////////////////
	void parse(Array<ConverterRef>& converters)
	{
		char c;
		size_t patternLength(pattern.length());

		while (i < patternLength)
		{
			c = pattern[i];
			++i;
			switch (state)
			{
				case E_LITERAL_STATE:
				{
					if (i == patternLength)
					{
						literal += c;
						continue;
					}
					// handle %% -> % and %n -> \n or move to the CONVERTER_STATE
					else if (c == '%')
					{
						switch (pattern[i])
						{
							case '%':
								literal += c;
								++i;
								break;
							case 'n':
								literal += '\n';
								++i;
								break;
							default:
								if (literal.length() > 0)
								{
									converters.push_back(ConverterRef(new LiteralConverter(literal.toString())));
									literal.reset();
								}
								literal += c;
								state = E_CONVERTER_STATE;
								formatting = Formatting();
						}
					}
					// handle \n, \\, \r, \t, \x<hexDigits>
					else if (c == '\\')
					{
						switch (pattern[i])
						{
							case 'n':
								literal += '\n';
								++i;
								break;
							
							case '\\':
								literal += '\\';
								++i;
								break;
							
							case 'r':
								literal += '\r';
								++i;
								break;
							
							case 't':
								literal += '\t';
								++i;
								break;
							
							case 'x':
							{
								if (i + 1 > patternLength)
								{
									literal += "\\x";
									++i;
									break;
								}

								char* begin = &pattern[i+1];
								char* end(0);
								errno = 0;
								int hexNumber = std::strtol(begin, &end, 16);
								if (end == begin  || errno == ERANGE || hexNumber > CHAR_MAX)
								{
									literal += "\\x";
									++i;
									break;
								}
								literal += static_cast<char>(hexNumber);
								i += (end - begin) + 1;
							}
							break;

							default:
								literal += '\\';
								break;
						}
					}
					else
					{
						literal += c;
					}
				}
				break;
				// handle converter stuff after a %
				case E_CONVERTER_STATE:
				{
					literal += c;
					switch (c)
					{
						case '-':
							formatting.justification = E_LEFT_JUSTIFY;
							break;
						case '.':
							state = E_DOT_STATE;
							break;
						default:
							if (isdigit(c))
							{
								formatting.minWidth = c - '0';
								state = E_MIN_STATE;
							}
							else
							{
								converters.push_back(finalizeConverter(c));
							}
					}
				}
				break;
				case E_MIN_STATE:
				{
					literal += c;
					if (isdigit(c))
					{
						formatting.minWidth = formatting.minWidth * 10 + (c - '0');
					}
					else if (c == '.')
					{
						state = E_DOT_STATE;
					}
					else
					{
						converters.push_back(finalizeConverter(c));
					}
				}
				break;
				case E_DOT_STATE:
				{
					literal += c;
					if (isdigit(c))
					{
						formatting.maxWidth = c - '0';
						state = E_MAX_STATE;
					}
					else
					{
						BLOCXX_THROW_ERR(LogMessagePatternFormatterException,
							Format("Invalid pattern \"%1\" in position %2. Was expecting a digit, instead got char %3.",
								pattern, i, c).c_str(),
							LogMessagePatternFormatter::E_INVALID_PATTERN_NO_DIGIT_AFTER_DOT);
					}
				}
				break;
				case E_MAX_STATE:
				{
					literal += c;
					if (isdigit(c))
					{
						formatting.maxWidth = formatting.maxWidth * 10 + (c - '0');
					}
					else
					{
						converters.push_back(finalizeConverter(c));
						state = E_LITERAL_STATE;
					}
				}
				break;
			} // switch
		} // while

		// hanlde whatever is left
		if (literal.length() > 0)
		{
			converters.push_back(ConverterRef(new LiteralConverter(literal.toString())));
		}
	}

/////////////////////////////////////////////////////////////////////////////
	String getOption()
	{
		// retrieves the contents of a { }, like in a %d{ISO8601}
		if ((i < pattern.length()) && (pattern[i] == '{'))
		{
			size_t end = pattern.indexOf('}', i);
			if (end > i)
			{
				String rv = pattern.substring(i + 1, end - (i + 1));
				i = end + 1;
				return rv;
			}
		}

		return String();
	}

/////////////////////////////////////////////////////////////////////////////
	int getPrecision()
	{
		// retrieves the numeric contents of a { }, like in a %c{2}
		String opt = getOption();
		int rv = 0;
		if (!opt.empty())
		{
			try
			{
				rv = opt.toUInt32();
			}
			catch (StringConversionException& e)
			{
				BLOCXX_THROW_ERR(LogMessagePatternFormatterException,
					Format("Invalid pattern \"%1\" in position %2. A positive integer is required for precision option (%3).",
						pattern, i, opt).c_str(),
					LogMessagePatternFormatter::E_INVALID_PATTERN_PRECISION_NOT_AN_INTEGER);
			}
		}
		return rv;
	}

/////////////////////////////////////////////////////////////////////////////
	ConverterRef finalizeConverter(char c)
	{
		// handle the actual type of converter
		ConverterRef rv;
		switch (c)
		{
			case 'c':
			{
				rv = new ComponentConverter(formatting, getPrecision());
			}
			break;

			case 'd':
			{
				String dateFormat;
				String dateOpt = getOption();
				if (dateOpt.empty())
				{
					dateFormat = DateConverter::ISO8601_DATE_FORMAT;
				}
				else
				{
					dateFormat = dateOpt;
				}

				// take care of the predefined date formats
				if (dateFormat.equalsIgnoreCase(DateConverter::ISO8601_DATE_FORMAT))
				{
					dateFormat = DateConverter::ISO8601_PATTERN;
				}
				else if (dateFormat.equalsIgnoreCase(DateConverter::ABSOLUTE_DATE_FORMAT))
				{
					dateFormat = DateConverter::ABSOLUTE_PATTERN;
				}
				else if (dateFormat.equalsIgnoreCase(DateConverter::DATE_DATE_FORMAT))
				{
					dateFormat = DateConverter::DATE_PATTERN;
				}

				rv = new DateConverter(formatting, dateFormat);
			}
			break;

			case 'F':
			{
				rv = new FileLocationConverter(formatting);
			}
			break;

			case 'l':
			{
				rv = new FullLocationConverter(formatting);
			}
			break;

			case 'L':
			{
				rv = new LineLocationConverter(formatting);
			}
			break;

			case 'M':
			{
				rv = new MethodLocationConverter(formatting);
			}
			break;

			case 'm':
			{
				rv = new MessageConverter(formatting);
			}
			break;
				
			case 'e':
			{
				rv = new XMLMessageConverter(formatting);
			}
			break;
				
			case 'p':
			{
				rv = new CategoryConverter(formatting);
			}
			break;

			case 'r':
			{
				rv = new RelativeTimeConverter(formatting);
			}
			break;

			case 't':
			{
				rv = new ThreadConverter(formatting);
			}
			break;

			case 'P':
			{
				rv = new PidConverter(formatting);
			}
			break;
#if 0 // don't support these for now.
			case 'x':
			{

			}
			break;

			case 'X':
			{

			}
			break;
#endif
			default:
			{
				BLOCXX_THROW_ERR(LogMessagePatternFormatterException,
					Format("Invalid pattern \"%1\" in position %2. Unsupported conversion (%3).",
						pattern, i, c).c_str(),
					LogMessagePatternFormatter::E_INVALID_PATTERN_UNSUPPORTED_CONVERSION);
				
			}
			break;
		}

		literal.reset();
		state = E_LITERAL_STATE;
		formatting = Formatting();
		return rv;
	}

private:
	size_t i;
	EParserState state;
	StringBuffer literal;
	Formatting formatting;
	String pattern;
};


} // end unnamed namespace

/////////////////////////////////////////////////////////////////////////////
LogMessagePatternFormatter::LogMessagePatternFormatter(const String& pattern)
{
	Parser parser(pattern);
	parser.parse(m_patternConverters);
}

} // end namespace BLOCXX_NAMESPACE





