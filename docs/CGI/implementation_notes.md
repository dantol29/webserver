# Notes

### those are snippets from the mighty RFC 3875

### I will use them to implement further, then organize/markdown them in a better way

The remainder of the path,
if any, is a resource or sub-resource identifier to be interpreted by
the script.
============> this is PATH_info

The server MUST ensure that the script output is sent to the client
unmodified.

The server MAY implement a timeout period within which data must be
received from the script.

4.2. Request Message-Body

Request data is accessed by the script in a system-defined method;
unless defined otherwise, this will be by reading the 'standard
input' file descriptor or file handle.

7.1. AmigaDOS

Meta-Variables
Meta-variables are passed to the script in identically named
MetaVariables variables.

The current working directory
The current working directory for the script is set to the
directory containing the script.

7.2. UNIX

For UNIX compatible operating systems, the following are defined:

Meta-Variables
Meta-variables are passed to the script in identically named
MetaVariables variables. These are accessed by the C library
routine getenv() or variable environ.

1.4. Terminology

This specification uses many terms defined in the HTTP/1.1
specification [4]; however, the following terms are used here in a
sense which may not accord with their definitions in that document,
or with their common meaning.

'meta-variable'
A named parameter which carries information from the server to the
script. It is not necessarily a variable in the operating
system's MetaVariables, although that is the most common
implementation.

'script'
The software that is invoked by the server according to this
interface. It need not be a standalone program, but could be a
dynamically-loaded or shared library, or even a subroutine in the
server. It might be a set of statements interpreted at run-time,
as the term 'script' is frequently understood, but that is not a
requirement and within the context of this specification the term
has the broader definition stated.

'server'
The application program that invokes the script in order to
service requests from the client.

4.  The CGI Request

Information about a request comes from two different sources; the
request meta-variables and any associated message-body.

4.1. Request Meta-Variables

Meta-variables contain data about the request passed from the server
to the script, and are accessed by the script in a system-defined
manner. Meta-variables are identified by case-insensitive names;
there cannot be two different variables whose names differ in case
only. Here they are shown using a canonical representation of
capitals plus underscore ("\_"). A particular system can define a
different representation.

Robinson & Coar Informational [Page 10]

RFC 3875 CGI Version 1.1 October 2004

      meta-variable-name = "AUTH_TYPE" | "CONTENT_LENGTH" |
                           "CONTENT_TYPE" | "GATEWAY_INTERFACE" |
                           "PATH_INFO" | "PATH_TRANSLATED" |
                           "QUERY_STRING" | "REMOTE_ADDR" |
                           "REMOTE_HOST" | "REMOTE_IDENT" |
                           "REMOTE_USER" | "REQUEST_METHOD" |
                           "SCRIPT_NAME" | "SERVER_NAME" |
                           "SERVER_PORT" | "SERVER_PROTOCOL" |
                           "SERVER_SOFTWARE" | scheme |
                           protocol-var-name | extension-var-name
      protocol-var-name  = ( protocol | scheme ) "_" var-name
      scheme             = alpha *( alpha | digit | "+" | "-" | "." )
      var-name           = token
      extension-var-name = token

Meta-variables with the same name as a scheme, and names beginning
with the name of a protocol or scheme (e.g., HTTP_ACCEPT) are also
defined. The number and meaning of these variables may change
independently of this specification. (See also section 4.1.18.)

The server MAY set additional implementation-defined extension meta-
variables, whose names SHOULD be prefixed with "X\_".

This specification does not distinguish between zero-length (NULL)
values and missing values. For example, a script cannot distinguish
between the two requests http://host/script and http://host/script?
as in both cases the QUERY_STRING meta-variable would be NULL.

      meta-variable-value = "" | 1*<TEXT, CHAR or tokens of value>

An optional meta-variable may be omitted (left unset) if its value is
NULL. Meta-variable values MUST be considered case-sensitive except
as noted otherwise. The representation of the characters in the
meta-variables is system-defined; the server MUST convert values to
that representation.
