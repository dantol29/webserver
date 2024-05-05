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
	{
		AResponseHandler::operator=(other);
	}
	return *this;
}

void CGIHandler::handleRequest(const HTTPRequest &request, HTTPResponse &response)
{
	CGIHandler cgiInstance;
	cgiInstance.setFDsRef(_FDsRef); // here we set the FDs to close later unused ones
	MetaVariables env;
	env.HTTPRequestToMetaVars(request, env);
	// std::cout << env;
	std::string cgiOutput = executeCGI(env);
	response.setIsCGI(true);
	CGIStringToResponse(cgiOutput, response);
	// std::cout << response;
	return;
}

std::vector<std::string> CGIHandler::createArgvForExecve(const MetaVariables &env)
{
	std::vector<std::string> argv;
	std::string scriptName = env.getVar("SCRIPT_NAME");
	std::string pathTranslated = env.getVar("PATH_TRANSLATED");
	std::string scriptPath = pathTranslated + scriptName;

	if (env.getVar("X_INTERPRETER_PATH") != "")
	{
		std::string interpreterVar = env.getVar("X_INTERPRETER_PATH");
		argv.push_back(interpreterVar);
		argv.push_back(scriptPath);
	}
	else
	{
		argv.push_back(scriptPath);
	}

	return argv;
}

// void CGIHandler::createArgvForExecve(const MetaVariables &env, std::vector<std::string> &argv)
// {
// 	std::string scriptName = env.getVar("SCRIPT_NAME");
// 	std::string pathTranslated = env.getVar("PATH_TRANSLATED");
// 	std::string scriptPath = pathTranslated + scriptName;

// 	if (env.getVar("X_INTERPRETER_PATH") != "")
// 	{
// 		std::string interpreterVar = env.getVar("X_INTERPRETER_PATH");
// 		argv.push_back(interpreterVar);
// 		argv.push_back(scriptPath);
// 	}
// 	else
// 	{
// 		argv.push_back(scriptPath);
// 	}

// 	return;
// }

// void CGIHandler::createArgvForExecve(const MetaVariables &env, std::vector<char *> &argv)
// {

// 	std::string scriptName = env.getVar("SCRIPT_NAME");
// 	std::string pathTranslated = env.getVar("PATH_TRANSLATED");
// 	std::string scriptPath = pathTranslated + scriptName;
// 	argv.push_back(const_cast<char *>(scriptPath.c_str()));
// 	argv.push_back(NULL);

// argv.push_back(const_cast<char *>(env.getVar("PATH_TRANSLATED" + env.getVar("SCRIPT_NAME")).c_str()));
// argv.push_back(NULL);

// std::cout << env;
// std::string scriptName = env.getVar("SCRIPT_NAME");
// std::string pathTranslated = env.getVar("PATH_TRANSLATED");
// std::string scriptPath = pathTranslated + scriptName;

// std::cout << "scriptPath: " << scriptPath << std::endl;

// if (env.getVar("X_INTERPRETER_PATH") != "")
// {
// 	std::string interpreterVar = env.getVar("X_INTERPRETER_PATH");
// 	argv.push_back(const_cast<char *>(interpreterVar.c_str()));
// 	argv.push_back(const_cast<char *>(scriptPath.c_str()));
// }
// else
// {
// argv.push_back(const_cast<char *>(scriptPath.c_str()));
// argv.push_back(NULL);
// }

// std::cout << "argv[0] in createArgvForExecve : " << argv[0] << std::endl;
// std::cout << &argv[0] << std::endl;

// return;
// }

void CGIHandler::CGIStringToResponse(const std::string &cgiOutput, HTTPResponse &response)
{
	std::size_t headerEndPos = cgiOutput.find("\r\n\r\n");
	if (headerEndPos == std::string::npos)
	{
		headerEndPos = cgiOutput.find("\n\n");
	}

	std::string headersPart = cgiOutput.substr(0, headerEndPos);
	std::string bodyPart = cgiOutput.substr(headerEndPos);
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
	response.setStatusCode(200, "");
	return;
}

void CGIHandler::closeAllSocketFDs()
{
	for (std::vector<struct pollfd>::iterator it = _FDsRef->begin(); it != _FDsRef->end(); ++it)
	{
		close(it->fd);
	}
}

std::vector<char *> convertToCStringArray(const std::vector<std::string> &input)
{
	std::vector<char *> pointers;
	pointers.reserve(input.size() + 1); // Reserve space for all entries and the terminating NULL

	// Replace range-based loop with index-based loop for C++98 compatibility
	for (size_t i = 0; i < input.size(); ++i)
	{
		pointers.push_back(const_cast<char *>(input[i].c_str()));
	}

	pointers.push_back(NULL); // Null-terminate the array
	return pointers;
}

std::string CGIHandler::executeCGI(const MetaVariables &env)
{
	std::string cgiOutput = "";
	std::vector<std::string> argv = createArgvForExecve(env);
	std::vector<std::string> envp = env.getForExecve();

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

		closeAllSocketFDs();

		std::vector<char *> argvPointers = convertToCStringArray(argv);
		std::vector<char *> envpPointers = convertToCStringArray(envp);

		execve(argvPointers[0], &argvPointers[0], &envpPointers[0]);

		perror("execve");

		exit(EXIT_FAILURE);
		// TODO: check if _exit isn't better
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
		std::cout << "------------------CGI output prepared-------------------" << std::endl;

		std::cout << "cgiOutput" << std::endl;
		std::cout << cgiOutput << std::endl;

		return cgiOutput;
	}
	return cgiOutput;
}

void CGIHandler::setFDsRef(std::vector<struct pollfd> *FDsRef)
{
	_FDsRef = FDsRef;
}

void CGIHandler::setPollFd(struct pollfd *pollFd)
{
	_pollFd = pollFd;
}
