#include <iostream>
#include <cstdlib>	// For getenv()
#include <unistd.h> // For environ

using namespace std;

extern char **environ; // Declare the external environ variable

int main()
{
	// header
	cout << "HTTP/1.1 200 OK\n";
	cout << "Content-type: text/html\n\n";
	// Start of the HTML content
	cout << "<html>\n";
	cout << "<head>\n";
	cout << "<title>Simple C++ CGI Script</title>\n";
	cout << "</head>\n";
	cout << "<body>\n";
	cout << "<h1>Hello from C++ CGI!</h1>\n";

	// Print all MetaVariables variables
	cout << "<h2>MetaVariables Variables</h2>\n";
	for (char **env = environ; *env != nullptr; ++env)
	{
		cout << "<p>" << *env << "</p>\n";
	}

	// End of the HTML content
	cout << "</body>\n";
	cout << "</html>\n";

	return 0;
}
