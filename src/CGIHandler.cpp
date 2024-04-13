#include "CGIHandler.hpp"
#include <unistd.h>

#define EXIT_FAILURE 1

CGIHandler::CGIHandler() {
}

CGIHandler::~CGIHandler() {
}

CGIHandler::CGIHandler(const CGIHandler &other) {
    (void)other;
}

CGIHandler &CGIHandler::operator=(const CGIHandler &other) {
    (void)other;
    return *this;
}

HTTPResponse CGIHandler::handleRequest(const HTTPRequest &request) {
    std::string cgiOutput = handleCGIRequest(request);
    // You'll need to construct an HTTPResponse object from cgiOutput.
    // This is a simplified example. You should set the appropriate headers and status code.
    HTTPResponse response;
    response.setBody(cgiOutput);
    return response;
}

std::string CGIHandler::handleCGIRequest(const HTTPRequest &request) {
    Environment env;

    //load the meta vars from the request to env
    env.HTTPRequestToMetaVars(request, env);
    env.printMetaVars();

    std::string cgiOutput = executeCGI(env);

    return cgiOutput;
}

//NOTE FOR SELF: key1=value1&key2=value2&key3=value3 might not be directly passed as command-line arguments (i.e., in argv)
//it migth  be passed to the script as part of the environment variables
//=> what is passed as command-line arguments ? path to the script + the path to the file to process ?
char* const* CGIHandler::createArgvForExecve(const Environment& env) {
    // std::vector<const char*> argv;

    // std::string scriptName = env.getVar("SCRIPT_NAME");
    // std::string pathTranslated = env.getVar("PATH_TRANSLATED");
    // std::string pathToFile = pathTranslated + scriptName;
    // // std::string queryString = env.getVar("QUERY_STRING");

    // argv.push_back(scriptName.c_str());
    // argv.push_back(pathTranslated.c_str());
    // // argv.push_back(queryString.c_str());
    // argv.push_back(NULL);

    // char* const* argvArray = new char*[argv.size()];
    // for (size_t i = 0; i < argv.size(); ++i) {
    //     size_t len = strlen(argv[i]);
    //     argvArray[i] = new char[len + 1];
    //     ft_strcpy(argvArray[i], argv[i]);
    // }

    // UPDATE: for now, we will just hardcode the path to the script
    (void)env;
    char** argv = new char*[2];

    std::string command = "cgi-bin/hello_py.cgi";
    argv[0] = new char[command.size() + 1];
    ft_strcpy(argv[0], command.c_str());

    // Null terminate the array
    argv[1] = NULL;

    return argv;
}

std::string CGIHandler::executeCGI(const Environment &env) {

    char* const* argv = createArgvForExecve(env);
    std::cout << "argv[0]: " << argv[0] << std::endl;
    std::cout << "argv[1]: " << argv[1] << std::endl;

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
    if (execve(argv[0], argv, envp.data()) == -1) {
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
std::cout << "------------------CGI output prepared-------------------" << std::endl;
        std::cout << cgiOutput << std::endl;
        return cgiOutput;
    }

    return ""; // Should not reach here
}
