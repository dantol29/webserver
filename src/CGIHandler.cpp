#include "CGIHandler.hpp"

#define EXIT_FAILURE 1

std::string CGIHandler::handleRequest(const HTTPRequest &request) {
    Environment env;

	//load the meta vars from the request to env
    env.HTTPRequestToMetaVars(request, env);
	env.printMetaVars();

    std::string cgiOutput = executeCGI(env);

    return cgiOutput;
}

//key1=value1&key2=value2&key3=value3 might not be directly passed as command-line arguments (i.e., in argv)
//it migth  be passed to the script as part of the environment variables
//=> what is passed as command-line arguments ? path to the script + the path to the file to process ?
char** CGIHandler::createArgvForExecve(const Environment& env) {
    std::vector<const char*> argv;

    std::string scriptName = env.getVar("SCRIPT_NAME");
    std::string pathTranslated = env.getVar("PATH_TRANSLATED");
	std::string pathToFile = pathTranslated + scriptName;
    // std::string queryString = env.getVar("QUERY_STRING");

    argv.push_back(scriptName.c_str());
    argv.push_back(pathTranslated.c_str());
    // argv.push_back(queryString.c_str());
    argv.push_back(NULL);

    char** argvArray = new char*[argv.size()];
    for (size_t i = 0; i < argv.size(); ++i) {
        size_t len = strlen(argv[i]);
        argvArray[i] = new char[len + 1];
        strcpy(argvArray[i], argv[i]);
    }

    return argvArray;
}

std::string CGIHandler::executeCGI(const Environment &env) {
	std::cout<<"------------------inside CGIHandler::executeCGI-------------------" << std::endl;
  	
    std::string cgiScriptPath = env.getVar("SCRIPT_NAME");
    // const char** argv = NULL;
	const char* argv[0] = {cgiScriptPath.c_str()};
	

	
	int pipeFD[2];
    if (pipe(pipeFD) == -1) {
        perror("pipe failed");
        _exit(EXIT_FAILURE);
    }

     pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        _exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(pipeFD[0]);
        dup2(pipeFD[1], STDOUT_FILENO);
        close(pipeFD[1]);

        std::vector<char *> envp = env.getForExecve();
        if (execve(argv[0], const_cast<char* const*>(argv), envp.data()) == -1) {
            perror("execve");
            exit(EXIT_FAILURE);
        }
    } else {
        close(pipeFD[1]);

        std::string cgiOutput;
        char readBuffer[256];
        ssize_t bytesRead;
        while ((bytesRead = read(pipeFD[0], readBuffer, sizeof(readBuffer) - 1)) > 0) {
            readBuffer[bytesRead] = '\0';
            cgiOutput += readBuffer;
        }
        close(pipeFD[0]);

        int status;
        waitpid(pid, &status, 0);
        return cgiOutput;
    }

    return ""; // Should not reach here
}
