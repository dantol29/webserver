# poll

Commented and a little bit variated man page for poll

## Name

poll – synchronous I/O multiplexing

## SYNOPSIS

```c
#include <poll.h>

int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```

## DESCRIPTION

poll() examines a set of file descriptors to see if some of them are ready for I/O or if certain events have occurred on them.

- The **fds** argument is a pointer to an array of pollfd structures, as defined in ⟨poll.h⟩ (shown below).
- The **nfds** argument specifies the size of the fds array.
- The **timeout** parameter specifies the duration (in milliseconds) that poll() should wait for any of the specified file descriptors to become ready for I/O operations. A timeout value of -1 indicates that poll() should wait indefinitely, a value of 0 instructs poll() to return immediately even if no file descriptors are ready (non-blocking mode), and a positive value specifies the maximum wait time.

```c
struct pollfd {
	int	fd;	  /* file descriptor */
	short	events;   /* events to look for */
	short	revents;  /* events returned */
};
```

The event bitmasks in events and revents have the following bits:

<details>
<summary>
*Note on events as bitmask*
</summary>
In a bitmask, each bit represents a different flag or setting, and these bits correspond to individual values. When a bit is set to 1, it indicates that the associated setting or flag is "on" or active. Conversely, a bit set to 0 means the setting is "off" or inactive. This allows multiple flags to be stored in a single integer variable, making it an efficient way to track a combination of on/off settings. POLLIN is represented by 1 (0000 0001), and POLLPRI by 2 (0000 0010), so that basically you could listen for both events (0000 0011). You would set this by writing `int event = POLLIN | POLLPRI`]
</details>

[*Note on events vs revents*: events are the event we are listening for for a certain file descriptor, revents return the 'state' of the file descriptor at the moment the event we are listening for occured. For example we could listen only for POLLIN (data available to read), and when POLLIN happens, when data are available to read, the returned revents could containe not only POLLIN, but also POLLHUP (the other side hung up), and POLLER (an error occured on the connection)]

- **POLLERR** An exceptional condition has occurred on the device or socket. This flag is `output only`, and ignored if present in the input events bitmask.

[Not so easy to reproduce this error in a program]

- **POLLHUP** The device or socket has been disconnected. This flag is `output only`, and ignored if present in the input events bitmask. Note that POLLHUP and POLLOUT are mutually exclusive and should never be present in the revents bitmask at the same time.

[Note on POLLHUP and POLLOUT being mutalliy exclusive:]
[Note on the name POLLHUP. The 'HUP' in POLLHUP means 'Hung UP']
[A POLLHUP event let poll returns even if we don't listen explicitely for it: see the pollhup.c file. It is relatively easy to reproduce]

- **POLLIN** Data other than high priority data may be read without blocking. This is equivalent to ( POLLRDNORM | POLLRDBAND).

[POLLIN is triggered when a file descriptor, usually a socket, has data available for reading without blocking. This condition indicates that the system can read data other than high-priority data, similar to how one would read from standard input (STDIN). POLLIN is equivalent to combining POLLRDNORM and POLLRDBAND, which means both normal and out-of-band data are ready to be read. POLLIN listens for both of them. This flag is essential in network programming and IPC (Interprocess Communication) for efficiently managing data transmission by signaling when it's appropriate to read from a file descriptor.]

[Note on POLLRDNNORM and POLLRDBAND:

- for the meaning see the section of these errors
- POLLRDNORM and POLLRDBAND, means respectively 'Normal Data is available for reading and Out-Of-Band (Priority) Data is available for reading. The distinction between normal data and priority data pertains not the http layer but the TCP/IP layer. It seems that a web server should not handle at all the POLLRDBAND event. A short check in different codebase gave as result that no one is explicitely handling POLLRDBAND events]

- **POLLNVAL** The file descriptor is not open. This flag is `output only`, and ignored if present in the input events bitmask.

It means: POLL INVALID (REQUEST)

- **POLLOUT** Normal data may be written without blocking. This is equivalent to POLLWRNORM.

- **POLLPRI** High priority data may be read without blocking.

- **POLLRDBAND** Priority data may be read without blocking.

`POLL`, `RD` (related to read operations), `BAND` ('out of BAND', which is a term used in networking to describe data that is sent outside of the regular data stream, often with higher priority. )

- **POLLRDNORM** Normal data may be read without blocking.

`POLL`, `RD` (related to read operations), `NORM` (the file descriptor have 'normal' data available for reading)

- **POLLWRBAND** Priority data may be written without blocking.

- **POLLWRNORM** Normal data may be written without blocking.

  The distinction between normal, priority, and high-priority data is
  specific to particular file types or devices.

  If timeout is greater than zero, it specifies a maximum interval (in
  milliseconds) to wait for any file descriptor to become ready. If
  timeout is zero, then poll() will return without blocking. If the value
  of timeout is -1, the poll blocks indefinitely.

## List of events as in poll.h

# poll.h Compatibility and Events

# Ubuntu Implementation of poll.h

**Note:** Never use `<bits/poll.h>` directly; include `<sys/poll.h>` instead.

## Event Types for Polling

Event types that can be polled for. These bits may be set in `events` to indicate the interesting event types; they will appear in `revents` to indicate the status of the file descriptor.

- `POLLIN`: 0x001 - There is data to read.
- `POLLPRI`: 0x002 - There is urgent data to read.
- `POLLOUT`: 0x004 - Writing now will not block.

### X/Open System Interfaces Extension

These values are defined in XPG4.2.

- `POLLRDNORM`: 0x040 - Normal data may be read.
- `POLLRDBAND`: 0x080 - Priority data may be read.
- `POLLWRNORM`: 0x100 - Writing now will not block.
- `POLLWRBAND`: 0x200 - Priority data may be written.

### GNU Extensions (Linux-specific)

These are extensions for Linux.

- `POLLMSG`: 0x400
- `POLLREMOVE`: 0x1000
- `POLLRDHUP`: 0x2000

## Implicitly Polled Event Types

Event types always implicitly polled for. These bits need not be set in `events`, but they will appear in `revents` to indicate the status of the file descriptor.

- `POLLERR`: 0x008 - Error condition.
- `POLLHUP`: 0x010 - Hung up.
- `POLLNVAL`: 0x020 - Invalid polling request.

- `POLLSTANDARD`: `(POLLIN|POLLPRI|POLLOUT|POLLRDNORM|POLLRDBAND|POLLWRBAND|POLLERR|POLLHUP|POLLNVAL)`

### events vs revents

Even if we are listeing only for POLLIN, when poll returns, cause POLLIN happened, all three if could be positively triggered.

```c
int ret = poll(fds, POLLIN, timeout_milliseconds);
if (ret > 0) {
    if (fds[0].revents & POLLIN) {
        // Data available to read.
    }
    if (fds[0].revents & POLLHUP) {
        // Connection closed by the other side.
    }
    if (fds[0].revents & POLLERR) {
        // An error occurred.
    }
}

```
