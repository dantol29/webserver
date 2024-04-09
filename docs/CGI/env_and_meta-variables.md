
### From URI to environment variables AKA meta-variable

## From [RFC 3875](https://datatracker.ietf.org/doc/html/rfc3875)
> ### 1.4.  Terminology
> 
> `meta-variable`  
> A named parameter which carries information from the server to the script. It is not necessarily a variable in the operating system's environment, although that is the most common implementation.

> ### 4.1.  Request Meta-Variables
> 
> ```plaintext
> meta-variable-name = "AUTH_TYPE" | "CONTENT_LENGTH" |
>                      "CONTENT_TYPE" | "GATEWAY_INTERFACE" |
>                      "PATH_INFO" | "PATH_TRANSLATED" |
>                      "QUERY_STRING" | "REMOTE_ADDR" |
>                      "REMOTE_HOST" | "REMOTE_IDENT" |
>                      "REMOTE_USER" | "REQUEST_METHOD" |
>                      "SCRIPT_NAME" | "SERVER_NAME" |
>                      "SERVER_PORT" | "SERVER_PROTOCOL" |
>                      "SERVER_SOFTWARE" | scheme |
>                      protocol-var-name | extension-var-name
> protocol-var-name  = ( protocol | scheme ) "_" var-name
> scheme             = alpha *( alpha | digit | "+" | "-" | "." )
> var-name           = token
> extension-var-name = token
> ```

## URI and URL

- **URI (Uniform Resource Identifier):** Identifies resources either by location, name, or both, encompassing URLs and URNs
`scheme:[//authority]path[?query][#fragment]`

- **URL (Uniform Resource Locator):** A specific type of URI that locates a resource on the internet by specifying its access mechanism.
`https://example.com:80/path/to/resource?query=123#section`


## How does a URI translates to a meta-variable:
example for the URI `http://localhost:8080/myScript.cgi/view?id=123`

| scheme | SERVER_NAME | SERVER_PORT | SCRIPT_NAME    | PATH_INFO | QUERY_STRING |
|--------|-------------|-------------|----------------|-----------|--------------|
| http   | localhost   | 8080        | /myScript.cgi  | /view     | id=123       |

In this constructed Script-URI, /myScript.cgi is the script you're calling, /view represents additional path info that could affect how the script processes the request, and ?id=123 is a query string providing extra parameters to the script. The construction aligns with the format described, ensuring that accessing this URI would execute the script with the specified path info and query string.

## Request target to meta-variable
we use a `std::string _requestTarget;` to map the request to CGI meta-variables:

### Origin-Form
- **Approach**: For requests **directly to our server**, we extract the path and query from `_requestTarget`.
  - `SCRIPT_NAME`: Assign path component of `_requestTarget` for endpoint identification.
  - `QUERY_STRING`: Extract and assign query component from `_requestTarget` for additional data processing.

### Absolute-Form
- **Approach**: Handle proxy requests where `_requestTarget` includes the entire URI.
  - Since `REQUEST_URI` is not a standard CGI 1.1 meta-variable, no direct CGI variable assignment is needed.

### Authority-Form
- **Approach**: Manage CONNECT methods for tunnels (e.g., HTTPS).
  - Direct setting of CGI variables for CONNECT methods is typically not applicable. Handle these methods at the web server or proxy level.

### Asterisk-Form
- **Approach**: Address OPTIONS methods targeting the server itself.
  - `REQUEST_METHOD`: Set to "OPTIONS" for asterisk-form requests. Specific handling is server configuration dependent.

By mapping `_requestTarget` and other request details to these CGI meta-variables, we facilitate a smooth communication between the web server and our C++ application, ensuring that requests are handled appropriately based on their nature and our application's architecture.

