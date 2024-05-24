## 400 Bad Request
### How to trigger?
Send invalid HTTP request

## 401 Unauthorized
_not supported_ 

## 402 Payment Required
_not supported_

## 403 Forbidden
### How to trigger?
Access file without permissions (TODO: @Leo)

## 404 Not Found
### How to trigger?
Access unexisting page

## 405 Method Not Allowed
### How to trigger?
Any request method besides GET, POST, DELETE

## 406 Not Acceptable
_not supported_

## 407 Proxy Authentication Required
_not supported_

## 408 Request Timeout
### How to trigger?
Send a request with incomplete headers

## 409 Conflict
_not supported_

## 410 Gone
_not supported_

## 411 Length Required
### How to trigger?
Send POST request without Content-Length header

## 412 Precondition Failed
_not supported_

## 413 Payload Too Large
### How to trigger?
Send POST request bigger than client_max_body_size

## 414 URI Too Long
### How to trigger?
Send request with headers > 8KB

## 415 Unsupported Media Type
Send request to CGI that is not in cgi_ext directive

## 416 Range Not Satisfiable
_not supported_

## 417 Expectation Failed
_not supported_

## 418 I'm a teapot
_not supported_

## 421 Misdirected Request
_not supported_

## 422 Unprocessable Content (WebDAV)
_not supported_

## 423 Locked (WebDAV)
_not supported_

## 424 Failed Dependency (WebDAV)
_not supported_

## 425 Too Early Experimental
_not supported_

## 426 Upgrade Required
TODO (@Someone)

## 428 Precondition Required
_not supported_

## 429 Too Many Requests
TODO (@Stefano maybe?)

## 431 Request Header Fields Too Large
### How to trigger?
Send request with headers > 8KB

## 451 Unavailable For Legal Reasons
_not supported_

## 503 Service Unavailable
### How to trigger?
Set `limit_connn` directive and send more requests than written there