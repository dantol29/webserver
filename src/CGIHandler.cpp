#include "CGIHandler.hpp"

#define EXIT_FAILURE 1

CGIHandler::CGIHandler()
{
}

CGIHandler::~CGIHandler()
{
}

CGIHandler &CGIHandler::operator=(const CGIHandler &other)
{
	if (this != &other)
	{									   // Protect against self-assignment
		ARequestHandler::operator=(other); // Call the base class assignment operator
										   // Copy or assign other members of CGIHandler if necessary
	}
	return *this;
}

HTTPResponse CGIHandler::handleRequest(const HTTPRequest &request)
{
	CGIHandler cgiInstance;
	Environment env;
	env.HTTPRequestToMetaVars(request, env);
	std::string cgiOutput = executeCGI(env);

	HTTPResponse response;
	response.setBody(cgiOutput);
	response.setIsCGI(true);
	std::cout << response;
	return response;
}

// NOTE FOR SELF: key1=value1&key2=value2&key3=value3 might not be directly passed as command-line arguments (i.e., in
// argv) it migth  be passed to the script as part of the environment variables
//=> what is passed as command-line arguments ? path to the script + the path to the file to process ?
char *const *CGIHandler::createArgvForExecve(const Environment &env)
{
	char **argv = new char *[2];

	std::string scriptName = env.getVar("SCRIPT_NAME");
	std::string pathTranslated = env.getVar("PATH_TRANSLATED");
	std::string scriptPath = pathTranslated + scriptName;

	argv[0] = new char[scriptPath.length() + 1];
	ft_strcpy(argv[0], scriptPath.c_str());

	argv[1] = NULL;

	return argv;
}

std::string CGIHandler::executeCGI(const Environment &env)
{

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
		if (execve(argv[0], argv, envp.data()) == -1)
		{
			perror("execve");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		close(pipeFD[1]);

		std::string cgiOutput;
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
		// std::cout << cgiOutput << std::endl;
		return cgiOutput;
	}

	return ""; // Should not reach here
}
