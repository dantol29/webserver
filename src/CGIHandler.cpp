#include "CGIHandler.hpp"

CGIHandler::CGIHandler(EventManager &eventManager, Connection &connection)
	: AResponseHandler(), _connection(connection), _eventManager(eventManager)
{
}

// Copy Constructor
CGIHandler::CGIHandler(const CGIHandler &other)
	: AResponseHandler(other)
	, _FDsRef(other._FDsRef)
	, _pollFd(other._pollFd)
	, _connection(other._connection)
	, _eventManager(other._eventManager)
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

void CGIHandler::handleRequest(HTTPRequest &request, HTTPResponse &response)
{

	std::cout << RED << "Entering CGIHandler::handleRequest" << RESET << std::endl;
	MetaVariables env;
	env.HTTPRequestToMetaVars(request, env);
	if (!executeCGI(env, response))
	{
		response.setStatusCode(500, "");
		response.setBody("500 Internal Server Error");
	}
	std::cout << RED << "Exiting CGIHandler::handleRequest" << RESET << std::endl;
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

bool CGIHandler::executeCGI(const MetaVariables &env, HTTPResponse &response)
{
	std::cout << RED << "Entering CGIHandler::executeCGI" << RESET << std::endl;
	std::string cgiOutput;
	std::vector<std::string> argv = createArgvForExecve(env);
	std::vector<std::string> envp = env.getForExecve();

	int pipeFD[2];
	if (pipe(pipeFD) == -1)
	{
		perror("pipe failed");
		return false;
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		perror("fork failed");
		close(pipeFD[0]);
		close(pipeFD[1]);
		return false;
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
			return false;
			_exit(EXIT_FAILURE);
			// TODO: @leo I don't think we should exit here. We don't want to kill the whole server cause of a CGI
			// error. No?
		}

		// execve(argvPointers[0], argvPointers.data(), envpPointers.data());
		if (execve(argvPointers[0], argvPointers.data(), envpPointers.data()) == -1)
		{
			perror("execve");
			return false;
			// TODO: @leo We should check if execve failed and return an error response and not exti
			_exit(EXIT_FAILURE);
		}
	}
	// This is executed if the CGI is started successfully
	response.setIsCGI(true);
	response.setCGIpipeFD(pipeFD);

	close(pipeFD[1]);
	EventData data = {1, pid}; // Assuming 1 is the event type for CGI started
	std::cout << "CGIHandler: Emitting event indicating a CGI process has started" << std::endl;
	_eventManager.emit(data); // Emit event indicating a CGI process has started
	// conn.addCGI(pid);
	_connection.addCGI(pid);
	// TODO: is this used? To which process to you want to send this signal/ @Leo
	// signal(SIGALRM, handleTimeout);
	// alarm(4);
	std::cout << RED << "Exiting CGIHandler::executeCGI with true" << RESET << std::endl;
	return true;
}

void CGIHandler::setFDsRef(std::vector<struct pollfd> *FDsRef)
{
	_FDsRef = FDsRef;
}

void CGIHandler::setPollFd(struct pollfd *pollFd)
{
	_pollFd = pollFd;
}
