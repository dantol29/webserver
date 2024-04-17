### From URI to MetaVariables variables AKA meta-variable

## From [RFC 3875](https://datatracker.ietf.org/doc/html/rfc3875)

> ### 1.4. Terminology
>
> `meta-variable`  
> A named parameter which carries information from the server to the script. It is not necessarily a variable in the operating system's MetaVariables, although that is the most common implementation.
> 7.3. [...]
> Meta-Variables
> Meta-variables are passed to the script in identically named
> MetaVariables variables. These are accessed by the C library
> routine getenv().

### meta-variables VS MetaVariables variables:

- Meta-variables are named parameters sent from the server to the script.
- Some of them are system MetaVariables variables, but not all, or not necessarily
- Even if they are not env vars, they are accessed as such (as instance via getenv())

> ### 4.1. Request Meta-Variables
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

- **URI (Uniform Resource Identifier):** Identifies resources either by location, name, or both, encompassing URLs and URNs (Uniform Resource Names) A URI provides a way to **identify a resource without** necessarily providing **information on how to locate it**.
  `scheme:[//authority]path[?query][#fragment]`

- **URL (Uniform Resource Locator):** a specific type of URI that **provides the means to locate a resource** on the internet. It includes the protocol used to access the resource (such as HTTP or FTP), the domain name or IP address of the server, and the path to the resource on that server.
  `https://example.com:80/path/to/resource?query=123#section`

**Example of a URI that is not a URL:**
URN (Uniform Resource Name): URNs are another type of URI that is used to uniquely identify a resource without specifying its location. Unlike URLs, URNs do not provide information on how to locate the resource. An example of a URN could be an ISBN for a book or a DOI for a scientific paper.

_So, while all URLs are URIs, not all URIs are URLs. URNs are one example of URIs that are not URLs._

## How does a URI translates to a meta-variable:

example for the URI `http://localhost:8080/myScript.cgi/view?id=123`

| scheme | SERVER_NAME | SERVER_PORT | SCRIPT_NAME   | PATH_INFO | QUERY_STRING |
| ------ | ----------- | ----------- | ------------- | --------- | ------------ |
| http   | localhost   | 8080        | /myScript.cgi | /view     | id=123       |

In this constructed Script-URI, /myScript.cgi is the script you're calling, /view represents additional path info that could affect how the script processes the request, and ?id=123 is a query string providing extra parameters to the script. The construction aligns with the format described, ensuring that accessing this URI would execute the script with the specified path info and query string.

## Request target to meta-variable

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
