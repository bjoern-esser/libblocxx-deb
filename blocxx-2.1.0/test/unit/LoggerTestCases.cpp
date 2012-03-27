/*******************************************************************************
* Copyright (C) 2005, Quest Software, Inc. All rights reserved.
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
*       Quest Software, Inc., 
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
#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "LoggerTestCases.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/LogConfig.hpp"
#include "blocxx/LogAppender.hpp"
#include "blocxx/AppenderLogger.hpp"
#include "blocxx/StringBuffer.hpp"
#include "blocxx/Process.hpp"
#include "blocxx/UnnamedPipe.hpp"
#include "blocxx/Exec.hpp"

#include <cstdio> // for remove
#include <algorithm>
#include <vector>

using namespace blocxx;

void LoggerTestCases::setUp()
{
}

void LoggerTestCases::tearDown()
{
}

namespace
{
LoggerRef
createLogger(const String& type_)
{
	String type(type_);

	StringArray components;
	components.push_back("*");

	StringArray categories;
	categories.push_back("*");

	LoggerConfigMap configItems;

	String filename = type;
	if (type != "syslog")
	{
		type = "file";
		configItems["log.test.location"] = filename;
	}
	LogAppenderRef logAppender = LogAppender::createLogAppender("test", components, categories,
		LogMessagePatternFormatter::STR_DEFAULT_MESSAGE_PATTERN, type, configItems);

	LoggerRef l(new AppenderLogger("logger test cases", E_ERROR_LEVEL, logAppender));
	return l;
}

class StringLogAppender : public LogAppender
{
public:
	StringLogAppender(const String& components, const String& categories, StringArray& outputMessages, const char* pattern = "%m")
		: LogAppender(String(components).tokenize(), String(categories).tokenize(), pattern)
		, m_outputMessages(outputMessages)
	{
	}

	virtual void doProcessLogMessage(const String& formattedMessage, const LogMessage& message) const
	{
		m_outputMessages.push_back(formattedMessage);
	}
private:
	StringArray& m_outputMessages;
};

LoggerRef
createStringLogger(const String& components, const String& categories, const String& defaultComponent, StringArray& outputMessages, const char* pattern = "%m")
{
	LogAppenderRef stringAppender(new StringLogAppender(components, categories, outputMessages, pattern));
	return LoggerRef(new AppenderLogger(defaultComponent, stringAppender->getLogLevel(), stringAppender));
}

}

#ifdef BLOCXX_WIN32
void LoggerTestCases::testCreateFileLogger()
{
	String logname = "testCreateFileLogger.log";
	{
		LoggerRef pLogger = createLogger(logname);
		unitAssert( pLogger );
	}

	unitAssert( DeleteFile(logname.c_str()) );
	String badfilename = "some\\dir\\that\\doesn't\\exist";
	unitAssertThrows(createLogger(badfilename));
}
#else
void LoggerTestCases::testCreateFileLogger()
{
	String logname = "/tmp/testlog";
	LoggerRef pLogger = createLogger(logname);
	unitAssert( pLogger );
	unitAssert( remove(logname.c_str()) != -1 );
	String badfilename = "some/dir/that/doesn't/exist";
	unitAssertThrows(createLogger(badfilename));
}
#endif

void LoggerTestCases::testCreateSyslogLogger()
{
	LoggerRef pLogger = createLogger(String("syslog"));
	unitAssert( pLogger );
}

void LoggerTestCases::verifyFileLog( const char* file, int line, const char* filename, const char* test )
{
	StringArray expectedLines = String(test).tokenize("\n");
	StringArray actualLines = FileSystem::getFileLines(filename);
	
	this->assertImplementation( expectedLines.size() == actualLines.size(),
		"verifyFileLog: expectedLines.size() == actualLines.size()", line, file );
	for (size_t i = 0; i < expectedLines.size(); ++i)
	{
		this->assertImplementation( actualLines[i].indexOf(expectedLines[i]) != String::npos,
			"verifyFileLog: actualLines[i].find(expectedLines[i]) != String::npos", line, file );
	}
}

void LoggerTestCases::testFileLogging()
{
	String filename = "testFileLogging.log";
	FileSystem::removeFile(filename);
	
	LoggerRef pLogger = createLogger(filename);
	BLOCXX_LOG_FATAL_ERROR(pLogger, "fatalerror1");
	BLOCXX_LOG_ERROR(pLogger, "error1" );
	BLOCXX_LOG_INFO(pLogger, "custinfo1" );
	BLOCXX_LOG_DEBUG(pLogger, "debug1" );
	// we recreate the logger before every verify so that the previous one gets deleted and flushes the logs to the file.
	pLogger = createLogger(filename);
	verifyFileLog( __FILE__, __LINE__, filename.c_str(),
				   "fatalerror1\n"
				   "error1\n" );

	pLogger->setLogLevel( E_INFO_LEVEL );
	BLOCXX_LOG_FATAL_ERROR(pLogger, "fatalerror2");
	BLOCXX_LOG_ERROR(pLogger, "error2" );
	BLOCXX_LOG_INFO(pLogger, "custinfo2" );
	BLOCXX_LOG_DEBUG(pLogger, "debug2" );
	pLogger = createLogger(filename);
	verifyFileLog( __FILE__, __LINE__, filename.c_str(),
				    "fatalerror1\n"
				    "error1\n"
					"fatalerror2\n"
					"error2\n"
					"custinfo2\n"
					 );

	pLogger->setLogLevel( E_DEBUG_LEVEL );
	BLOCXX_LOG_FATAL_ERROR(pLogger, "fatalerror3");
	BLOCXX_LOG_ERROR(pLogger, "error3" );
	BLOCXX_LOG_INFO(pLogger, "custinfo3" );
	BLOCXX_LOG_DEBUG(pLogger, "debug3" );
	pLogger = createLogger(filename);
	verifyFileLog( __FILE__, __LINE__, filename.c_str(),
					"fatalerror1\n"
					"error1\n"
					"fatalerror2\n"
					"error2\n"
					"custinfo2\n"
					"fatalerror3\n"
					"error3\n"
					"custinfo3\n"
					"debug3\n"
					 );
	
	pLogger->setLogLevel( E_FATAL_ERROR_LEVEL );
	BLOCXX_LOG_FATAL_ERROR(pLogger, "fatalerror4");
	BLOCXX_LOG_ERROR(pLogger, "error4" );
	BLOCXX_LOG_INFO(pLogger, "custinfo4" );
	BLOCXX_LOG_DEBUG(pLogger, "debug4" );
	pLogger = createLogger(filename);
	verifyFileLog( __FILE__, __LINE__, filename.c_str(),
					"fatalerror1\n"
					"error1\n"
					"fatalerror2\n"
					"error2\n"
					"custinfo2\n"
					"fatalerror3\n"
					"error3\n"
					"custinfo3\n"
					"debug3\n"
				    "fatalerror4\n"
					 );

	FileSystem::removeFile(filename);
}

namespace
{
	char const mp_mid_line[] =
		": xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx line ";
	size_t const mp_line_len = (sizeof(mp_mid_line) - 1) + 2 + 3;

	String mp_expected_output(unsigned nprocs, unsigned nreps)
	{
		StringBuffer buf;
		for (unsigned i = 0; i < nprocs; ++i)
		{
			String s;
			s.format("%02x", i);
			for (unsigned j = 0; j < nreps; ++j)
			{
				String nstr;
				nstr.format("%02x\n", j);
				buf += s;
				buf += mp_mid_line;
				buf += nstr;
			}
		}
		return buf.releaseString();
	}

	bool mp_less(String const & s1, String const & s2)
	{
		return s1.substring(0, 2) < s2.substring(0, 2);
	}

	template <typename InputIter>
	String cat_strings(InputIter first, InputIter last)
	{
		StringBuffer buf;
		for ( ; first != last; ++first)
		{
			buf += *first;
		}
		return buf.releaseString();
	}

	void remove_log_files(String const & fname, unsigned n)
	{
		FileSystem::removeFile(fname);
		for (unsigned i = 0; i <= n; ++i)
		{
			FileSystem::removeFile(fname + "." + String(i));
		}
		FileSystem::removeFile(fname + ".lock");
	}

	// Split contents on newline characters and append the resulting
	// sequence of strings to lines.  The terminating newlines are retained.
	//
	void append_lines(std::vector<String> & lines, String const & contents)
	{
		size_t pos = 0;
		while (pos < contents.length())
		{
			size_t end = contents.indexOf('\n', pos);
			if (end == String::npos)
			{
				lines.push_back(contents.substring(pos));
				return;
			}
			else
			{
				++end;
				lines.push_back(contents.substring(pos, end - pos));
				pos = end;
			}
		}
	}

	String backup_fname(String const & log_fname, unsigned i)
	{
		String fname;
		fname.format("%s.%u", log_fname.c_str(), i);
		return fname;
	}
}

void LoggerTestCases::testMultiProcessLogger()
{
	char const *   log_file_name       = "test.mp.log";
	char const *   lock_file_name       = "test.mp.log.lock";
	unsigned const nreps               = 23;
	char const *   nreps_str           = "23";
	unsigned const max_file_kbytes     = 1;
	char const *   max_file_kbytes_str = "1";
	unsigned const max_backups         = 50;
	char const *   max_backups_str     = "50";
	unsigned const nprocs = 10;
#ifdef BLOCXX_WIN32
	char const *   run_file_name       = "./runMultiProcessLogger.exe";
#else
	char const *   run_file_name       = "./runMultiProcessLogger";
#endif

	remove_log_files(log_file_name, max_backups);
	std::vector<ProcessRef> process;
	char const * argv[] = {
		run_file_name, max_file_kbytes_str, max_backups_str, 0,
		nreps_str, log_file_name, 0
	};
	unsigned i;
	for (i = 0; i < nprocs; ++i)
	{
		String s;
		s.format("%02x", i);
		argv[3] = s.c_str();
		process.push_back(Exec::spawn(argv));
	}
	for (i = 0; i < nprocs; ++i)
	{
		String errmsg = process[i]->err()->readAll();
		String outmsg = process[i]->out()->readAll();
		unitAssert(errmsg == "");
		unitAssert(outmsg == "");
		process[i]->waitCloseTerm(Timeout::relative(5.0), Timeout::relative(0.0), Timeout::relative(8.0));
		unitAssert(process[i]->processStatus().terminatedSuccessfully());
	}

	size_t lower_limit = max_file_kbytes * 1024;
	size_t upper_limit = lower_limit + mp_line_len;

	for (i = 0; FileSystem::exists(backup_fname(log_file_name, i + 1)); ++i)
	{
	}
	unsigned nbackup = i;

	std::vector<String> lines;
	String contents;
	for (i = nbackup; i > 0; --i)
	{
		contents = FileSystem::getFileContents(backup_fname(log_file_name, i));
		append_lines(lines, contents);
		unitAssert(contents.length() < upper_limit);
		unitAssert(contents.length() >= lower_limit);
	}
	contents = FileSystem::getFileContents(log_file_name);
	append_lines(lines, contents);
	unitAssert(contents.length() < upper_limit);

	std::stable_sort(lines.begin(), lines.end(), &mp_less);
	String output = cat_strings(lines.begin(), lines.end());
	String expected = mp_expected_output(nprocs, nreps);
	unitAssert(output == expected);

	for (i = nbackup; i > 0; --i)
	{
		FileSystem::removeFile(backup_fname(log_file_name, i));
	}
	FileSystem::removeFile(log_file_name);
	FileSystem::removeFile(lock_file_name);
}

void LoggerTestCases::testSyslogLogging()
{
	String filename = "syslog";
	LoggerRef pLogger = createLogger(filename);
	BLOCXX_LOG_FATAL_ERROR(pLogger, "fatalerror1");
	BLOCXX_LOG_ERROR(pLogger,  "error1" );
	BLOCXX_LOG_INFO(pLogger,  "custinfo1" );
	BLOCXX_LOG_DEBUG(pLogger,  "debug1" );
}

void LoggerTestCases::testComponentLog()
{
	StringArray outputMessages;
	LoggerRef lgr = createStringLogger("testcomponent1", "*", "testcomponent1", outputMessages);
	// assume default - should get logged.
	BLOCXX_LOG_ERROR(lgr, "error1");
	unitAssert(outputMessages.size() == 1);
	unitAssert(outputMessages[0] == "error1");

	// try category and message - component should be the default
	lgr->logMessage(Logger::STR_ERROR_CATEGORY, "error2");
	unitAssert(outputMessages.size() == 2);
	unitAssert(outputMessages[1] == "error2");

	// now a explicit component that should get logged
	lgr->logMessage("testcomponent1", Logger::STR_ERROR_CATEGORY, "error3");
	unitAssert(outputMessages.size() == 3);
	unitAssert(outputMessages[2] == "error3");

	// now for a non-default component that *shouldn't* get logged
	lgr->logMessage("nonloggedcomponent", Logger::STR_ERROR_CATEGORY, "error4");
	unitAssert(outputMessages.size() == 3);

}

void LoggerTestCases::testCategoryLog()
{
	StringArray outputMessages;
	LoggerRef lgr = createStringLogger("*", Logger::STR_INFO_CATEGORY, "testcomponent1", outputMessages);

	// assume default - should get logged.
	BLOCXX_LOG_INFO(lgr, "info1");
	unitAssert(outputMessages.size() == 1);
	unitAssert(outputMessages[0] == "info1");

	// try category and message - component should be the default
	lgr->logMessage(Logger::STR_INFO_CATEGORY, "info2");
	unitAssert(outputMessages.size() == 2);
	unitAssert(outputMessages[1] == "info2");

	// now a explicit component that should get logged
	lgr->logMessage("testcomponent1", Logger::STR_INFO_CATEGORY, "info3");
	unitAssert(outputMessages.size() == 3);
	unitAssert(outputMessages[2] == "info3");

	// now for a non-default component that should get logged
	lgr->logMessage("anothercomponent", Logger::STR_INFO_CATEGORY, "info4");
	unitAssert(outputMessages.size() == 4);
	unitAssert(outputMessages[3] == "info4");

	// now for another category that shouldn't get logged
	lgr->logMessage("anothercomponent", Logger::STR_ERROR_CATEGORY, "info4");
	unitAssert(outputMessages.size() == 4);

}

void LoggerTestCases::testComponentCategoryLog()
{
	// first try with only 1 component and 1 category
	{
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("cp", "ct", "main", outputMessages);

		// default - no log
		BLOCXX_LOG_INFO(lgr, "info1");
		unitAssert(outputMessages.size() == 0);

		// try category and message - component should be the default
		lgr->logMessage(Logger::STR_INFO_CATEGORY, "info2");
		unitAssert(outputMessages.size() == 0);

		// now a explicit component that shouldn't get logged
		lgr->logMessage("testcomponent1", Logger::STR_INFO_CATEGORY, "info3");
		unitAssert(outputMessages.size() == 0);

		lgr->logMessage("cp", Logger::STR_INFO_CATEGORY, "info4");
		unitAssert(outputMessages.size() == 0);

		lgr->logMessage("xx", "ct", "info4");
		unitAssert(outputMessages.size() == 0);

		lgr->logMessage("cp", "ct", "info5");
		unitAssert(outputMessages.size() == 1);
		unitAssert(outputMessages[0] == "info5");
	}

	// second try with multiple components & multiple categories
	{
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("cp1 cp2 cp3", "ct1 ct2", "main", outputMessages);

		// default - no log
		BLOCXX_LOG_INFO(lgr, "info1");
		unitAssert(outputMessages.size() == 0);

		// try category and message - component should be the default
		lgr->logMessage(Logger::STR_INFO_CATEGORY, "info2");
		unitAssert(outputMessages.size() == 0);

		// now a explicit component that shouldn't get logged
		lgr->logMessage("testcomponent1", Logger::STR_INFO_CATEGORY, "info3");
		unitAssert(outputMessages.size() == 0);

		lgr->logMessage("cp1", Logger::STR_INFO_CATEGORY, "info4");
		unitAssert(outputMessages.size() == 0);

		lgr->logMessage("cp2", Logger::STR_INFO_CATEGORY, "info4");
		unitAssert(outputMessages.size() == 0);

		lgr->logMessage("cp3", Logger::STR_INFO_CATEGORY, "info4");
		unitAssert(outputMessages.size() == 0);

		lgr->logMessage("xx", "ct1", "info4");
		unitAssert(outputMessages.size() == 0);

		lgr->logMessage("xx", "ct2", "info4");
		unitAssert(outputMessages.size() == 0);

		lgr->logMessage("xx", "ct3", "info4");
		unitAssert(outputMessages.size() == 0);

		lgr->logMessage("cp1", "ct1", "info5");
		unitAssert(outputMessages.size() == 1);
		unitAssert(outputMessages[0] == "info5");

		lgr->logMessage("cp1", "ct2", "info6");
		unitAssert(outputMessages.size() == 2);
		unitAssert(outputMessages[1] == "info6");

		lgr->logMessage("cp2", "ct1", "info7");
		unitAssert(outputMessages.size() == 3);
		unitAssert(outputMessages[2] == "info7");

		lgr->logMessage("cp2", "ct2", "info8");
		unitAssert(outputMessages.size() == 4);
		unitAssert(outputMessages[3] == "info8");

		lgr->logMessage("cp3", "ct1", "info9");
		unitAssert(outputMessages.size() == 5);
		unitAssert(outputMessages[4] == "info9");

		lgr->logMessage("cp3", "ct2", "info10");
		unitAssert(outputMessages.size() == 6);
		unitAssert(outputMessages[5] == "info10");
	}

}

void LoggerTestCases::testMultipleComponentLogs()
{
	StringArray outputMessages1;
	StringArray outputMessages2;
	StringArray outputMessages3;
	StringArray outputMessages4;

	Array<LogAppenderRef> appenders;
	appenders.push_back(LogAppenderRef(new StringLogAppender("testcomponent1", "*", outputMessages1)));
	appenders.push_back(LogAppenderRef(new StringLogAppender("testcomponent1 testcomponent2", "FATAL ERROR INFO", outputMessages2)));
	appenders.push_back(LogAppenderRef(new StringLogAppender("testcomponent3", Logger::STR_DEBUG_CATEGORY, outputMessages3)));
	appenders.push_back(LogAppenderRef(new StringLogAppender("*", "*", outputMessages4)));

	LoggerRef lgr(new AppenderLogger("defaultComponent", appenders));


    // should get logged to 4
	BLOCXX_LOG_ERROR(lgr, "error1");
    // should get logged to 1, 4
	lgr->logMessage("testcomponent1", "foo", "foo1");
    // should get logged to 1, 2, 4
	lgr->logMessage("testcomponent1", Logger::STR_INFO_CATEGORY, "foo2");
    // should get logged to 2, 4
	lgr->logMessage("testcomponent2", Logger::STR_ERROR_CATEGORY, "error2");
    // should get logged to 4
	lgr->logMessage("testcomponent2", "foo", "foo3");
    // should get logged to 3, 4
	lgr->logMessage("testcomponent3", Logger::STR_DEBUG_CATEGORY, "debug1");
    // should get logged to 4
	lgr->logMessage("testcomponent3", "foo", "foo4");

	// outputMessages1 == { "foo1", "foo2" }
	// outputMessages2 == { "foo2", "error2" }
	// outputMessages3 == { "debug1" }
	// outputMessages4 == { "error1", "foo1", "foo2", "error2", "foo3", "debug1", "foo4" };

	unitAssert(outputMessages1.size() == 2);
	unitAssert(outputMessages1[0] == "foo1");
	unitAssert(outputMessages1[1] == "foo2");

	unitAssert(outputMessages2.size() == 2);
	unitAssert(outputMessages2[0] == "foo2");
	unitAssert(outputMessages2[1] == "error2");
	
	unitAssert(outputMessages3.size() == 1);
	unitAssert(outputMessages3[0] == "debug1");

	unitAssert(outputMessages4.size() == 7);
	unitAssert(outputMessages4[0] == "error1");
	unitAssert(outputMessages4[1] == "foo1");
	unitAssert(outputMessages4[2] == "foo2");
	unitAssert(outputMessages4[3] == "error2");
	unitAssert(outputMessages4[4] == "foo3");
	unitAssert(outputMessages4[5] == "debug1");
	unitAssert(outputMessages4[6] == "foo4");


}

void LoggerTestCases::testLogMessageFormat()
{
	{
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%m");
		BLOCXX_LOG_ERROR(lgr, "error1");
		//std::cout << "\n01234567890123456789\n" << outputMessages[0] << std::endl;
		unitAssert(outputMessages.size() == 1);
		unitAssert(outputMessages[0] == "error1");
	}
	{	// %d outputs the date in ISO8601 format: "YYYY-mm-dd HH:mm:ss,SSS"
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%d");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);
		// too hard to check the actual date, we'll just check the delimiters.
		unitAssert(outputMessages[0][4] == '-' && outputMessages[0][7] == '-' &&
			outputMessages[0][10] == ' ' && outputMessages[0][13] == ':' &&
			outputMessages[0][16] == ':' && outputMessages[0][19] == ',');
	}
	{	// %d{ISO8601} outputs the date in ISO8601 format: "YYYY-mm-dd HH:mm:ss,SSS"
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%d{ISO8601}");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);
		// too hard to check the actual date, we'll just check the delimiters.
		unitAssert(outputMessages[0][4] == '-' && outputMessages[0][7] == '-' &&
			outputMessages[0][10] == ' ' && outputMessages[0][13] == ':' &&
			outputMessages[0][16] == ':' && outputMessages[0][19] == ',');
	}
	{	// %d{ABSOLUTE} outputs the date in format: %H:%M:%S,%Q for example, "15:49:37,459"
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%d{ABSOLUTE}");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);
		// too hard to check the actual date, we'll just check the delimiters.
		unitAssert(outputMessages[0][2] == ':' && outputMessages[0][5] == ':' &&
			outputMessages[0][8] == ',');
	}
	{	// %d{DATE} outputs the date in format: "%d %b %Y %H:%M:%S,%Q" for example, "06 Nov 1994 15:49:37,459"
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%d{DATE}");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);
		// too hard to check the actual date, we'll just check the delimiters.
		unitAssert(outputMessages[0][2] == ' ' && outputMessages[0][6] == ' ' &&
			outputMessages[0][11] == ' ' && outputMessages[0][14] == ':' && outputMessages[0][17] == ':' &&
			outputMessages[0][20] == ',');
	}
	{	// date in format: "%d %b %Y %H:%M:%S,%Q" for example, "06 Nov 1994 15:49:37,459"
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%d{%d %b %Y %H:%M:%S,%Q}");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);
		// too hard to check the actual date, we'll just check the delimiters.
		unitAssert(outputMessages[0][2] == ' ' && outputMessages[0][6] == ' ' &&
			outputMessages[0][11] == ' ' && outputMessages[0][14] == ':' && outputMessages[0][17] == ':' &&
			outputMessages[0][20] == ',');
	}
	{ // test some literal text together with %m
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "This is a test message: \"%m\" and here's a %%%n \\x41 \\x \\n\\r\\\\ \\t end");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);

#define NEWLINE "\n"

		//std::cout << "\n01234567890123456789\n" << outputMessages[0] << std::endl;
		unitAssert(outputMessages[0] == "This is a test message: \"error1\" and here's a %" NEWLINE " A \\x \n\r\\ \t end");
#undef NEWLINE
	}
	{ // test all the variations on width formatting and justification.
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, ",%m,%.4m,%2m,%2.4m,%-.4m,%-2m,%-2.4m,");
		BLOCXX_LOG_ERROR(lgr, "1");
		BLOCXX_LOG_ERROR(lgr, "22");
		BLOCXX_LOG_ERROR(lgr, "333");
		BLOCXX_LOG_ERROR(lgr, "4444");
		BLOCXX_LOG_ERROR(lgr, "12345");
		unitAssert(outputMessages.size() == 5);
		unitAssert(outputMessages[0] == ",1,1, 1, 1,1,1 ,1 ,");
		unitAssert(outputMessages[1] == ",22,22,22,22,22,22,22,");
		unitAssert(outputMessages[2] == ",333,333,333,333,333,333,333,");
		unitAssert(outputMessages[3] == ",4444,4444,4444,4444,4444,4444,4444,");
		unitAssert(outputMessages[4] == ",12345,2345,12345,2345,1234,12345,1234,");
	}
	{ // test thread id %t
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%t");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);
		unitAssertNoThrow(outputMessages[0].toUInt64());
	}
	{ // test process id %P
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%P");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);
		unitAssertNoThrow(outputMessages[0].toUInt64());
	}
	{ // bad format strings
		StringArray outputMessages;
		unitAssertThrows(LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%.m"));
		unitAssertThrows(LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%A"));
	}
	{ // test component %c
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x.y.z", outputMessages, "%c");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);
		unitAssert(outputMessages[0] == "x.y.z");
	}
	{ // test component %c{2}
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x.y.z", outputMessages, "%c{2}");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);
		unitAssert(outputMessages[0] == "y.z");
	}
	{ // test filename %F
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x.y.z", outputMessages, "%F");
		lgr->logError("error1", "testfile", 123);
		unitAssert(outputMessages.size() == 1);
		unitAssert(outputMessages[0] == "testfile");
	}
	{ // test full location %l
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x.y.z", outputMessages, "%l");
		lgr->logError("error1", "testfile", 123);
		unitAssert(outputMessages.size() == 1);
		unitAssert(outputMessages[0] == "testfile(123)");
	}
	{ // test line number %L
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x.y.z", outputMessages, "%L");
		lgr->logError("error1", "testfile", 123);
		unitAssert(outputMessages.size() == 1);
		unitAssert(outputMessages[0] == "123");
	}
	{ // test priority (category) %p
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x.y.z", outputMessages, "%p");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);
		unitAssert(outputMessages[0] == Logger::STR_ERROR_CATEGORY);
	}
	{ // test relative time %r
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%r");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);
		unitAssertNoThrow(outputMessages[0].toUInt64());
	}
	{ // XML escaped message %e
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%e");
		BLOCXX_LOG_ERROR(lgr, "error1");
		BLOCXX_LOG_ERROR(lgr, "]]>err]]>or1]]>");
		unitAssert(outputMessages.size() == 2);
		unitAssert(outputMessages[0] == "<![CDATA[error1]]>");
		//std::cout << "\n01234567890123456789\n" << outputMessages[1] << std::endl;
		unitAssertEquals(outputMessages[1], "<![CDATA[]]>]]&gt;<![CDATA[err]]>]]&gt;<![CDATA[or1]]>]]&gt;<![CDATA[]]>");
	}
	{ // method name %M
		StringArray outputMessages;
		LoggerRef lgr = createStringLogger("*", "*", "x", outputMessages, "%M");
		BLOCXX_LOG_ERROR(lgr, "error1");
		unitAssert(outputMessages.size() == 1);
		//std::cout << "\n01234567890123456789\n" << outputMessages[0] << std::endl;
#if defined (BLOCXX_HAVE_UUPRETTY_FUNCTIONUU)
		unitAssert(outputMessages[0] == __PRETTY_FUNCTION__);
#elif defined (BLOCXX_HAVE_C99_UUFUNCUU)
		unitAssert(outputMessages[0] == __func__);
#else
		unitAssert(outputMessages[0] == "");
#endif
	}

}

void LoggerTestCases::testLogLevels()
{
	StringArray outputMessages;
	LoggerRef lgr = createStringLogger("testcomponent1", "*", "testcomponent1", outputMessages);
	// assume default - should get logged.
	BLOCXX_LOG_FATAL_ERROR(lgr, "fatalerror1");
	BLOCXX_LOG_ERROR(lgr, "error1");
	BLOCXX_LOG_WARNING(lgr, "warning1");
	BLOCXX_LOG_INFO(lgr, "info1");
	BLOCXX_LOG_DEBUG(lgr, "debug1");
	BLOCXX_LOG_DEBUG2(lgr, "debug2");
	BLOCXX_LOG_DEBUG3(lgr, "debug3");
	unitAssertEquals(outputMessages.size(), 7U);
}


Test* LoggerTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("Logger");

	ADD_TEST_TO_SUITE(LoggerTestCases, testCreateFileLogger);
	ADD_TEST_TO_SUITE(LoggerTestCases, testFileLogging);
#ifndef BLOCXX_WIN32
	ADD_TEST_TO_SUITE(LoggerTestCases, testCreateSyslogLogger);
	ADD_TEST_TO_SUITE(LoggerTestCases, testSyslogLogging);
#endif
	ADD_TEST_TO_SUITE(LoggerTestCases, testComponentLog);
	ADD_TEST_TO_SUITE(LoggerTestCases, testCategoryLog);
	ADD_TEST_TO_SUITE(LoggerTestCases, testComponentCategoryLog);
	ADD_TEST_TO_SUITE(LoggerTestCases, testMultipleComponentLogs);
	ADD_TEST_TO_SUITE(LoggerTestCases, testLogMessageFormat);
	ADD_TEST_TO_SUITE(LoggerTestCases, testMultiProcessLogger);
	ADD_TEST_TO_SUITE(LoggerTestCases, testLogLevels);

	return testSuite;
}

