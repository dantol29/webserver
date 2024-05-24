## 400 Bad Request
### How to trigger?
Send invalid HTTP request

## 403 Forbidden
### How to trigger?
Access file without permissions (TODO: @Leo)

## 404 Not Found
### How to trigger?
Access unexisting page

## 405 Method Not Allowed
### How to trigger?
Any request method besides GET, POST, DELETE

## 408 Request Timeout
### How to trigger?
Send a request with incomplete headers

## 411 Length Required
### How to trigger?
Send POST request without `Content-Length` header

## 413 Payload Too Large
### How to trigger?
Send POST request bigger than `client_max_body_size`

## 414 URI Too Long
### How to trigger?
Send request with headers > 8KB

## 415 Unsupported Media Type
### How to trigger?
Send request to CGI that is not in `cgi_ext` directive

## 418 I'm a teapot
Would be nice to have

## 429 Too Many Requests
TODO (@Stefano maybe?)

## 431 Request Header Fields Too Large
### How to trigger?
Send request with headers > 8KB

## 500 Internal Server Error
### How to trigger?
Launch CGI with error inside

## 501 Not Implemented
### How to trigger?
Send a request with unsupported HTTP method

## 503 Service Unavailable
### How to trigger?
Set `limit_connn` directive and send more requests than written there

## 504 Gateway Timeout
### How to trigger?
Launch CGI that is executed longer than 5 seconds

## 505 HTTP Version Not Supported
### How to trigger?
Send a request with invalid HTTP version(not HTTP/1.1)

_16 supported errors_