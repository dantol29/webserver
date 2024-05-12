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
	// cgiInstance.setFDsRef(_FDsRef); // here we set the FDs to close later unused ones
	MetaVariables env;
	env.HTTPRequestToMetaVars(request, env);
	// std::cout << env;
	std::string cgiOutput = executeCGI(env);
	CGIStringToResponse(cgiOutput, response);
	// std::cout << response;

	std::cout << "------------------CGIHandler::handleRequest-------------------" << std::endl;
	std::cout << "CGIHandler: path: " << request.getPath() << std::endl;

	std::cout << "CGIHandler: request target: " << request.getRequestTarget() << std::endl;

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

void CGIHandler::CGIStringToResponse(const std::string &cgiOutput, HTTPResponse &response)
{
	std::size_t headerEndPos = cgiOutput.find("\r\n\r\n");
	if (headerEndPos == std::string::npos)
	{
		headerEndPos = cgiOutput.find("\n\n");
	}

	std::string headersPart = cgiOutput.substr(0, headerEndPos);
	std::string bodyPart = cgiOutput.substr(headerEndPos);

	std::cout << "------------------CGIStringToResponse-------------------" << std::endl;

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

// returns vectors of char*, which point to the internal strings of
// std::vector<std::string>. The memory they point to is  owned by the std::vector<std::string>
std::vector<char *> CGIHandler::convertToCStringArray(const std::vector<std::string> &input)
{
	std::vector<char *> pointers;
	pointers.reserve(input.size() + 1);

	for (size_t i = 0; i < input.size(); ++i)
	{
		pointers.push_back(const_cast<char *>(input[i].c_str()));
	}

	pointers.push_back(NULL);
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
		waitpid(pid, &status, WNOHANG);
		std::cout << "------------------CGI output prepared-------------------" << std::endl;
	}

	std::cout << "\n\n\n\nCGI output: " << cgiOutput << std::endl;

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
