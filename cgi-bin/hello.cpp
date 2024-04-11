#include <iostream>
#include <cstdlib> // For getenv() and system()

using namespace std;

int main() {
    //header
    cout << "HTTP/1.1 200 OK\n";
    cout << "Content-type: text/html\n\n";    
    // Start of the HTML content
    cout << "<html>\n";
    cout << "<head>\n";
    cout << "<title>Simple C++ CGI Script</title>\n";
    cout << "</head>\n";
    cout << "<body>\n";
    cout << "<h1>Hello from C++ CGI!</h1>\n";

    // Getting the QUERY_STRING environment variable
    char* query_string = getenv("QUERY_STRING");
    if (query_string) {
        cout << "<p>Query String: " << query_string << "</p>\n";
    } else {
        cout << "<p>No query string provided.</p>\n";
    }

    // End of the HTML content
    cout << "</body>\n";
    cout << "</html>\n";

    return 0;
}


//    $> c++ hello.cpp -o hello.cgi
//    $> chmod +x hello.cgi
