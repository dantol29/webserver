# Server

Notes about the Server class (and the server) to unclutter the Server.cpp file.

**Socket creation**

- The server socket is blocking (by default) which should not be a problem cause we use poll.

- To evaluate all the exit(s) we have already a linear issue: https://linear.app/web-serv/issue/WEB-33/evaluate-exits

**Re-binding with SO_REUSEADDR and SO_REUSEPORT**

- We set _SO_REUSEADDR_ to allow re-binding to the same address and to the same port

- SO_REUSEADDR and SO_REUSEPORT are separate. The call could have been done together, but some systems don't support SO_REUSEPORT so this could fail more often.

**Binding**

- We are exiting but consider retrying it and consider also that the server could listen on multiple ports in which case maybe we don't want to exit, if the server successfuly bound to another port.

**Listening**

-     Here is reasonable to exit the program. We could retry the listen, and we should consider if the Server is listening to multiple ports.

**poll**

- Since poll is taking a normal array as first arguemtn with `_FDs.data()` we pass the underlying array of pollfd structs contained in the \_FDs vector.

- `-1` means wait indefinitely

- for the server socket POLLLIN means that a new connection is available

**acceptNewConnection**

- accept() is blocking but we call it when the server socket is readable, i.e. when a new connection is available.
