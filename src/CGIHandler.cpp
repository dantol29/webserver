#include "CGIHandler.hpp"

CGIHandler::CGIHandler()
{
}

CGIHandler::~CGIHandler()
{
}

CGIHandler &CGIHandler::operator=(const CGIHandler &other)
{
	if (this != &other)
	{										// Protect against self-assignment
		AResponseHandler::operator=(other); // Call the base class assignment operator
											// Copy or assign other members of CGIHandler if necessary
	}
	return *this;
}

HTTPResponse CGIHandler::handleRequest(const HTTPRequest &request)
{
	CGIHandler cgiInstance;
	MetaVariables env;
	env.HTTPRequestToMetaVars(request, env);
	std::cout << env;
	std::string cgiOutput = executeCGI(env);

	// HTTPResponse response;
	// response.setBody(cgiOutput);
	// response.setIsCGI(true);
	// std::cout << response;
	return CGIStringToResponse(cgiOutput);
}

char *const *CGIHandler::createArgvForExecve(const MetaVariables &env)
{
	std::cout << env;
	char **argv = new char *[2];

	std::string scriptName = env.getVar("SCRIPT_NAME");
	std::string pathTranslated = env.getVar("PATH_TRANSLATED");
	std::string scriptPath = pathTranslated + scriptName;

	if (env.getVar("X_INTERPRETER_PATH") != "")
	{
		argv[0] = new char[env.getVar("X_INTERPRETER_PATH").length() + 1];
		std::strcpy(argv[0], env.getVar("X_INTERPRETER_PATH").c_str());
		argv[1] = new char[scriptPath.length() + 1];
		std::strcpy(argv[1], scriptPath.c_str());
		argv[2] = NULL;
	}
	else
	{
		argv[0] = new char[scriptPath.length() + 1];
		std::strcpy(argv[0], scriptPath.c_str());
		argv[1] = NULL;
	}

	return argv;
}

HTTPResponse CGIHandler::CGIStringToResponse(const std::string &cgiOutput)
{
	HTTPResponse response;

	std::size_t headerEndPos = cgiOutput.find("\r\n\r\n");
	if (headerEndPos == std::string::npos)
	{
		headerEndPos = cgiOutput.find("\n\n");
	}

	std::string headersPart = cgiOutput.substr(0, headerEndPos);
	std::string bodyPart = cgiOutput.substr(headerEndPos); // separator

	std::istringstream headerStream(headersPart);
	std::string headerLine;
	while (std::getline(headerStream, headerLine))
	{
		if (!headerLine.empty() && headerLine[headerLine.size() - 1] == '\r')
		{
			headerLine.erase(headerLine.size() - 1); // carriage return
		}

		std::size_t separatorPos = headerLine.find(": ");
		if (separatorPos != std::string::npos)
		{
			std::string headerName = headerLine.substr(0, separatorPos);
			std::string headerValue = headerLine.substr(separatorPos + 2);
			response.setHeader(headerName, headerValue);
		}
	}

	response.setBody(bodyPart);
	response.setIsCGI(true);
	response.setStatusCode(200);
	return response;
}

std::string CGIHandler::executeCGI(const MetaVariables &env)
{
	std::string cgiOutput = "";
	char *const *argv = createArgvForExecve(env);

	int pipeFD[2];
	if (pipe(pipeFD) == -1)
	{
		perror("pipe failed");
		_exit(EXIT_FAILURE);
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		perror("fork failed");
		_exit(EXIT_FAILURE);
	}
	else if (pid == 0)
	{
		close(pipeFD[0]);
		dup2(pipeFD[1], STDOUT_FILENO);
		close(pipeFD[1]);

		std::vector<char *> envp = env.getForExecve();
		execve(argv[0], argv, envp.data());

		perror("execve");
		for (int i = 0; argv[i] != NULL; i++)
		{
			delete[] argv[i];
		}
		delete[] argv;
		exit(EXIT_FAILURE); // TODO: check if _exit isn't better
	}
	else
	{
		close(pipeFD[1]);

		char readBuffer[256];
		ssize_t bytesRead;
		while ((bytesRead = read(pipeFD[0], readBuffer, sizeof(readBuffer) - 1)) > 0)
		{
			readBuffer[bytesRead] = '\0';
			cgiOutput += readBuffer;
		}
		close(pipeFD[0]);

		int status;
		waitpid(pid, &status, 0);

		for (int i = 0; argv[i] != NULL; i++)
		{
			delete[] argv[i];
		}
		delete[] argv;

		std::cout << "------------------CGI output prepared-------------------" << std::endl;
		return cgiOutput;
	}

	for (int i = 0; argv[i] != NULL; i++)
	{
		delete[] argv[i];
	}
	delete[] argv;

	return cgiOutput;
}
