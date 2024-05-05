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

void CGIHandler::createArgvForExecve(const MetaVariables &env,
									 std::vector<std::string> &argvStorage,
									 std::vector<char *> &argv)
{
	std::string scriptName = env.getVar("SCRIPT_NAME");
	std::string pathTranslated = env.getVar("PATH_TRANSLATED");
	std::string scriptPath = pathTranslated + scriptName;

	// Store the full script path in argvStorage
	argvStorage.push_back(scriptPath);

	// Add a pointer to the stored string's C-string representation
	argv.push_back(const_cast<char *>(argvStorage.back().c_str()));

	// Null-terminate the argv vector as required by execve
	argv.push_back(NULL);
}

// void CGIHandler::createArgvForExecve(const MetaVariables &env, std::vector<char *> &argv)
// {

// 	std::string scriptName = env.getVar("SCRIPT_NAME");
// 	std::string pathTranslated = env.getVar("PATH_TRANSLATED");
// 	std::string scriptPath = pathTranslated + scriptName;
// 	argv.push_back(const_cast<char *>(scriptPath.c_str()));
// 	argv.push_back(NULL);

// 	// argv.push_back(const_cast<char *>(env.getVar("PATH_TRANSLATED" + env.getVar("SCRIPT_NAME")).c_str()));
// 	// argv.push_back(NULL);

// 	// std::cout << env;
// 	// std::string scriptName = env.getVar("SCRIPT_NAME");
// 	// std::string pathTranslated = env.getVar("PATH_TRANSLATED");
// 	// std::string scriptPath = pathTranslated + scriptName;

// 	// std::cout << "scriptPath: " << scriptPath << std::endl;

// 	// if (env.getVar("X_INTERPRETER_PATH") != "")
// 	// {
// 	// 	std::string interpreterVar = env.getVar("X_INTERPRETER_PATH");
// 	// 	argv.push_back(const_cast<char *>(interpreterVar.c_str()));
// 	// 	argv.push_back(const_cast<char *>(scriptPath.c_str()));
// 	// }
// 	// else
// 	// {
// 	// argv.push_back(const_cast<char *>(scriptPath.c_str()));
// 	// argv.push_back(NULL);
// 	// }

// 	// std::cout << "argv[0] in createArgvForExecve : " << argv[0] << std::endl;
// 	// std::cout << &argv[0] << std::endl;

// 	// return;
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

std::string CGIHandler::executeCGI(const MetaVariables &env)
{
	std::string cgiOutput = "";
	std::vector<char *> argv;
	std::vector<std::string> argvStorage;

	// std::string scriptName = env.getVar("SCRIPT_NAME");
	// std::string pathTranslated = env.getVar("PATH_TRANSLATED");
	// std::string scriptPath = pathTranslated + scriptName;
	// argv.push_back(const_cast<char *>(scriptPath.c_str()));
	// argv.push_back(NULL);

	// // argv.push_back(const_cast<char *>("/var/www/development_site/cgi-bin/hello_py.cgi"));
	// argv.push_back(const_cast<char *>("hello_py.cgi"));
	// argv.push_back(NULL);

	// createArgvForExecve(env, argv);
	// std::cout << &argv[0] << std::endl;

	// argv.push_back(const_cast<char *>(env.getVar("PATH_TRANSLATED" + env.getVar("SCRIPT_NAME")).c_str()));
	// argv.push_back(NULL);

	CGIHandler::createArgvForExecve(env, argvStorage, argv);

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

		// std::vector<char *> envp = env.getForExecve();
		// std::cout << "argv[0]: " << argv[0] << std::endl;
		// execve(argv[0], argv.data(), envp.data());

		// argv[0] = const_cast<char *>("hello_py.cgi");
		// argv[1] = NULL;

		// std::vector<char *> envp = env.getForExecve();
		// std::cout << "Executing: " << argv[0] << std::endl;
		// execve(argv[0], argv.data(), envp.data());

		//------------------hardcoded values for testing-------------------
		(void)env;
		const char *envp[] = {"CONTENT_LENGTH=0",
							  "CONTENT_TYPE=text/html",
							  "GATEWAY_INTERFACE=CGI/1.1",
							  "PATH_INFO=/hello_py.cgi",
							  "PATH_TRANSLATED=/var/www/development_site/cgi-bin/hello_py.cgi",
							  NULL};

		// std::vector<char *> argv;
		// argv.push_back(const_cast<char *>("hello_py.cgi"));
		// argv.push_back(NULL);
		std::cout << "argv[0]: " << argv[0] << std::endl;
		execve(argv[0], argv.data(), const_cast<char *const *>(envp));
		//-------------------------------------------------------------------

		// execve(argv[0], &argv[0], envp);

		perror("execve");

		// exit(EXIT_FAILURE);
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
