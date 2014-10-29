/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "TeamcityOutput.h"
#include "Utils/Utils.h"

#include <iostream>


#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)


namespace DAVA
{
    
const char* const TeamcityOutput::START_TEST = "start test ";
const char* const TeamcityOutput::FINISH_TEST = "finish test ";
const char* const TeamcityOutput::TEST_ERROR = "test error ";

void TeamcityOutput::Output(Logger::eLogLevel ll, const char8 *text) const
{
	switch(outputMode)
	{
	case DEFAULT_MODE:
		DefaultOutput(text, ll);
		break;
	case TESTS_MODE:
		TestsOutput(text, ll);
		break;
	}
}

void TeamcityOutput::Output(Logger::eLogLevel ll, const char16 *text) const
{
    WideString wstr = text;
    Output(ll, WStringToString(wstr).c_str());
}

String TeamcityOutput::NormalizeString(const char8 *text) const
{
    String str = text;
    
    StringReplace(str, "|", "||");

    StringReplace(str, "'", "|'");
    StringReplace(str, "\n", "|n");
    StringReplace(str, "\r", "|r");

    StringReplace(str, "[", "|[");
    StringReplace(str, "]", "|]");
    
    return str;
}

void TeamcityOutput::PlatformOutput(const String &text) const
{
	std::cout << text << std::flush; // not std::endl couse \n already added by Logger
}

void TeamcityOutput::DefaultOutput(const char8 * text, Logger::eLogLevel ll) const
{
	String outStr = NormalizeString(text);
	String output;
	String status;

	switch (ll)
	{
	case Logger::LEVEL_ERROR:
		status = "ERROR";
		output = "##teamcity[buildProblem description=\'ERROR: " + outStr + "\']";
		PlatformOutput(output);
		break;

	case Logger::LEVEL_WARNING:
		status = "WARNING";
		break;

	default:
		status = "NORMAL";
		break;
	}

	output = "##teamcity[message text=\'" + outStr + "\' errorDetails=\'\' status=\'" + status + "\']\n";
	PlatformOutput(output);
}

void TeamcityOutput::TestsOutput(const char8 * text, Logger::eLogLevel ll) const
{
	String outStr = NormalizeString(text);
	String output;

	String testName;
	if (0 == outStr.find(START_TEST)) // start with
	{
		testName = outStr.substr(std::strlen(START_TEST));
		testName = testName.substr(0, testName.size() - 2); // remove last two chars "|n"
		output = "##teamcity[testStarted name=\'" + testName + "\']\n";
	} else if (0 == outStr.find(FINISH_TEST))
	{
		testName = outStr.substr(std::strlen(FINISH_TEST));
		testName = testName.substr(0, testName.size() - 2); // remove last two chars "|n"
		output = "##teamcity[testFinished name=\'" + testName + "\']\n";
	} else if (0 == outStr.find(TEST_ERROR))
	{
		// format for test error: "TEST_ERROR test_name error_info"
		outStr = outStr.substr(std::strlen(TEST_ERROR));
		testName = outStr.substr(0, outStr.find(' '));
		output = "##teamcity[testFailed name=\'" + testName + "\' message=\'" + outStr + "\' details=\'\']\n";
	} else
	{
		DefaultOutput(text, ll);
		return;
	}

	PlatformOutput(output);
}


}; // end of namespace DAVA

#endif //#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__)

