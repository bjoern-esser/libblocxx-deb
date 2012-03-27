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
* @author Kevin S. Van Horn
*/

#include "blocxx/BLOCXX_config.h"
#include "blocxx/AppenderLogger.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/LogMessagePatternFormatter.hpp"
#include "blocxx/LogAppender.hpp"
#include "blocxx/SortedVectorMap.hpp"
#include "blocxx/Thread.hpp"
#include <iostream>
#include <cstdlib>
using namespace std;
using namespace blocxx;

LoggerRef create_logger(
	char const * max_file_size, char const * max_backup_index,
	char const * log_file_name
)
{
	StringArray components;
	components.push_back("*");

	StringArray categories;
	categories.push_back("*");

	LoggerConfigMap config_items;
	config_items["log.test.location"] = log_file_name;
	config_items["log.test.max_file_size"] = max_file_size;
	config_items["log.test.max_backup_index"] = max_backup_index;

	LogAppenderRef log_appender = LogAppender::createLogAppender(
		"test", components, categories, "%m", LogAppender::TYPE_MPFILE,
		config_items
	);
	return new AppenderLogger("logger test cases", E_ERROR_LEVEL, log_appender);
}

int main_aux(int argc, char * * argv)
{
	char const * max_file_size = argv[1];
	char const * max_backup_index = argv[2];
	String message_prefix(argv[3]);
	unsigned nreps = atoi(argv[4]);
	char const * log_file_name = argv[5];

	try
	{
		create_logger(
			max_file_size, max_backup_index, "./some/dir/that/doesn't/exist");
		cerr << "Failed to throw expected exception for bad log file name" << endl;
		return 3;
	}
	catch (LoggerException & e)
	{
	}

	LoggerRef logger =
		create_logger(max_file_size, max_backup_index, log_file_name);
	for (unsigned i = 0; i < nreps; ++i)
	{
		Thread::yield(); // encourage interleaved process execution
		String s;
		s.format("%02x", i);
		String msg = message_prefix + ": xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx line " + s;
		BLOCXX_LOG_ERROR(logger, msg);
	}

	return 0;
}

int main(int argc, char * * argv)
{
	if (argc != 6)
	{
		cerr << "Usage: " << argv[0] << " <max file size (Kbytes)>"
			<< " <max backup index> <message prefix> <nreps> <log file name>"
			<< endl;
		return 1;
	}
	try
	{
		return main_aux(argc, argv);
	}
	catch (std::exception & e)
	{
		cerr << "Exception thrown: " << e.what() << endl;
	}
	catch (...)
	{
		cerr << "Unknown exception thrown" << endl;
	}
	return 2;
}
