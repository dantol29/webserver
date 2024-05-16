#include "CGIHandler.hpp"

CGIHandler::CGIHandler(EventManager &eventManager, Connection &connection)
	: AResponseHandler(), _connection(connection), _eventManager(eventManager)
{
}

// Copy Constructor
CGIHandler::CGIHandler(const CGIHandler &other) : AResponseHandler(other), _FDsRef(other._FDsRef), _pollFd(other._pollFd), _connection(other._connection), _eventManager(other._eventManager)
{
	// TODO: do we need deep copy here?
}

// Destructor
CGIHandler::~CGIHandler()
{
}

CGIHandler &CGIHandler::operator=(const CGIHandler &other)
{
	if (this != &other)
	{
		AResponseHandler::operator=(other);
		_eventManager = other._eventManager;
		_FDsRef = other._FDsRef;
		_pollFd = other._pollFd;
	}
	return *this;
}

void CGIHandler::handleRequest(const HTTPRequest &request, HTTPResponse &response)
{
	// CGIHandler cgiInstance(_eventManager);
	// cgiInstance.setFDsRef(_FDsRef); // here we set the FDs to close later unused ones
	MetaVariables env;
	env.HTTPRequestToMetaVars(request, env);
	// std::cout << env;
	std::string cgiOutput = executeCGI(env);
	// if cgioutput == "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
	// then we should return 500 error
	if (cgiOutput == "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n")
	{
		response.setStatusCode(500, "");
		response.setBody("500 Internal Server Error");
		return;
	}
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
	std::cout << "createArgvForExecve: scriptName: " << scriptName << std::endl;
	std::string pathTranslated = env.getVar("PATH_TRANSLATED");
	std::cout << "createArgvForExecve: pathTranslated: " << pathTranslated << std::endl;
	std::string scriptPath = pathTranslated;
	std::cout << "createArgvForExecve: scriptPath: " << scriptPath << std::endl;

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

void handleTimeout(int sig)
{
	(void)sig;
	std::cout << "CGIHandler: Timeout" << std::endl;
}

std::string CGIHandler::executeCGI(const MetaVariables &env)
{
	std::string cgiOutput;
	std::vector<std::string> argv = createArgvForExecve(env);
	std::vector<std::string> envp = env.getForExecve();

	int pipeFD[2];
	if (pipe(pipeFD) == -1)
	{
		perror("pipe failed");
		return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		perror("fork failed");
		close(pipeFD[0]);
		close(pipeFD[1]);
		return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
	}
	else if (pid == 0)
	{
		close(pipeFD[0]);
		dup2(pipeFD[1], STDOUT_FILENO);
		close(pipeFD[1]);

		closeAllSocketFDs();

		std::vector<char *> argvPointers = convertToCStringArray(argv);
		std::vector<char *> envpPointers = convertToCStringArray(envp);

		if (access(argvPointers[0], X_OK) == -1)
		{
			perror("access");
			_exit(EXIT_FAILURE);
		}

		execve(argvPointers[0], argvPointers.data(), envpPointers.data());
		perror("execve");
		_exit(EXIT_FAILURE);
	}
	// This is executed if the CGI is started successfully
	close(pipeFD[1]);
	EventData data = {1, pid}; // Assuming 1 is the event type for CGI started
	_eventManager.emit(data);  // Emit event indicating a CGI process has started
	// conn.addCGI(pid);
	_connection.addCGI(pid);
	// TODO: is this used? To which process to you want to send this signal/ @Leo
	signal(SIGALRM, handleTimeout);
	alarm(4);
	// This will be executed only if the CGI process returned (before the timeout)
	char readBuffer[256];
	ssize_t bytesRead;
	while ((bytesRead = read(pipeFD[0], readBuffer, sizeof(readBuffer) - 1)) > 0)
	{
		readBuffer[bytesRead] = '\0';
		cgiOutput += readBuffer;
	}
	close(pipeFD[0]);

	int status;
	pid_t waitedPid = waitpid(pid, &status, 0);
	alarm(0);
	//
	if (waitedPid == -1)
	{
		perror("waitpid");
		return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
	{
		return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
	}

	if (cgiOutput.empty())
	{
		return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
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
