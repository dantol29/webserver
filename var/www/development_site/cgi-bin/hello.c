#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv, char **envp)
{
    //  header
    printf("HTTP/1.1 200 OK\n");
    printf("Content-type: text/html\n\n");
	//
    printf("<html>\n");
    printf("<head>\n");
    printf("<title>Simple C CGI Script</title>\n");
    printf("</head>\n");
    printf("<body>\n");
    printf("<h1>Hello from C CGI!</h1>\n");

    printf("<h2>Environment Variables</h2>\n");
    for (char **env = envp; *env != NULL; ++env) {
        printf("<p>%s</p>\n", *env);
    }

    printf("</body>\n");
    printf("</html>\n");

    return 0;
}