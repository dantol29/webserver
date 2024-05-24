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
		_connection = other._connection;
		_eventManager = other._eventManager;
		_FDsRef = other._FDsRef;
		_pollFd = other._pollFd;
	}
	return *this;
}

void CGIHandler::handleRequest(HTTPRequest &request, HTTPResponse &response)
{
	Debug::log("CGIHandler::handleRequest", Debug::CGI);
	MetaVariables env;
	env.HTTPRequestToMetaVars(request, env);

	if (!executeCGI(env, request.getBody(), response))
	{
		response.setStatusCode(500, "Internal Server Error");
		// TODO: it should be hardcoded
		response.setBody("500 Internal Server Error");
	}
	Debug::log("Connection PID" +  toString(_connection.getCGIPid()), Debug::CGI);
	return;
}

std::vector<std::string> CGIHandler::createArgvForExecve(const MetaVariables &env)
{
	std::vector<std::string> argv;
	std::string scriptName = env.getVar("SCRIPT_NAME");
	Debug::log("createArgvForExecve: scriptName: " + scriptName, Debug::CGI);
	std::string pathTranslated = env.getVar("PATH_TRANSLATED");
	Debug::log("createArgvForExecve: pathTranslated: " + pathTranslated, Debug::CGI);
	std::string scriptPath = pathTranslated;
	Debug::log("createArgvForExecve: scriptPath: " + scriptPath, Debug::CGI);

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
	Debug::log("CGIHandler: Timeout", Debug::CGI);
}

bool CGIHandler::executeCGI(const MetaVariables &env, std::string body, HTTPResponse &response)
{
	Debug::log("CGIHandler::executeCGI", Debug::CGI);
	std::string cgiOutput;
	std::vector<std::string> argv = createArgvForExecve(env);
	std::vector<std::string> envp = env.getForExecve();

	Debug::log("CGIHandler: executeCGI: body: " + body, Debug::NORMAL);

	int pipeFD[2];
	int bodyPipeFD[2];
	if (pipe(pipeFD) == -1)
	{
		perror("pipe failed");
		return false;
	}

	if (pipe(bodyPipeFD) == -1)
	{
		perror("body pipe failed");
		close(pipeFD[0]);
		close(pipeFD[1]);
		return false;
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		perror("fork failed");
		close(pipeFD[0]);
		close(pipeFD[1]);
		close(bodyPipeFD[0]);
		close(bodyPipeFD[1]);
		return false;
	}
	else if (pid == 0)
	{
		// clang-format off
		// std::vector<std::pair<int, int> > pipes = _eventManager.getPipeFDs();
		// std::cerr << "CGIHandler: pipes: " << pipes.size() << std::endl;
		// for (std::vector<std::pair<int, int> >::const_iterator it = pipes.begin(); it != pipes.end(); ++it)
		// {
		// 	std::cerr << GREEN << "CLOSING: " << (*it).first << ", " << (*it).second <<  RESET << std::endl;
		// 	close((*it).first);
		// 	close((*it).second);
		// }
		// clang-format on
		close(pipeFD[0]);
		dup2(pipeFD[1], STDOUT_FILENO);
		close(pipeFD[1]);

		close(bodyPipeFD[1]);
		dup2(bodyPipeFD[0], STDIN_FILENO);
		close(bodyPipeFD[0]);

		closeAllSocketFDs();

		std::vector<char *> argvPointers = convertToCStringArray(argv);
		std::vector<char *> envpPointers = convertToCStringArray(envp);

		if (access(argvPointers[0], X_OK) == -1)
		{
			Debug::log("CGIHandler: access failed", Debug::CGI);
			return false;
			_exit(EXIT_FAILURE);
			// TODO: @leo I don't think we should exit here. We don't want to kill the whole server cause of a CGI
			// error. No?
		}

		if (execve(argvPointers[0], argvPointers.data(), envpPointers.data()) == -1)
		{
			Debug::log("CGIHandler: execve failed", Debug::CGI);
			return false;
			// TODO: @leo We should check if execve failed and return an error response and not exti

			_exit(EXIT_FAILURE);
		}
	}
	else
	{
		close(pipeFD[1]);
		close(bodyPipeFD[0]);

		write(bodyPipeFD[1], body.c_str(), body.size());
		close(bodyPipeFD[1]);

		response.setIsCGI(true);
		response.setCGIpipeFD(pipeFD);

		close(pipeFD[1]);
		EventData data = {1, pid, pipeFD[0], pipeFD[1]}; // Assuming 1 is the event type for CGI started

		_eventManager.emit(data); // Emit event indicating a CGI process has started

		_connection.addCGI(pid);
		// std::cout << GREEN << _connection.getCGIPid() << RESET << std::endl;

		// clang-format off
		// std::vector<std::pair<int, int> > pipes = _eventManager.getPipeFDs();
		// for (std::vector<std::pair<int, int> >::const_iterator it = pipes.begin(); it != pipes.end(); ++it)
		// {
		// 	std::cout << GREEN << "CGIHandler: pipeFDs: " << (*it).first << RESET << std::endl;
		// }
		// clang-format on
		// std::cout << RED << "Exiting CGIHandler::executeCGI with true" << RESET << std::endl;
		return true;
	}
	return false;
}

void CGIHandler::setFDsRef(std::vector<struct pollfd> *FDsRef)
{
	_FDsRef = FDsRef;
}

void CGIHandler::setPollFd(struct pollfd *pollFd)
{
	_pollFd = pollFd;
}
