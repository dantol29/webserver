#include <chrono>
#include <condition_variable>
#include <cstring>
#include <fcntl.h> // Add this include for fcntl function
#include <fstream>
#include <iostream>
#include <mutex>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread> // Add this include for std::thread
#include <unistd.h>
#include <vector>

#include <poll.h> // Add this include for poll function

extern std::mutex mtx;
extern std::condition_variable cv;
extern bool serverReady;

#define PREFILL 0
#define MSG_DONTWAIT_ON 0
#define NON_BLOCKING_SOCKETS 1

// This will block the main thread for files larrger than 8MB
// Which is the default buffer size for sockets
void testSocketIOBlocking(const std::string &filename) {
  std::cout << "Testing socket I/O\n";
  int sv[2]; // Socket pair
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0) {
    perror("socketpair");
    exit(EXIT_FAILURE);
  }
  std::cout << "Socket pair created\n";

  int sendBufSize, recvBufSize;
  socklen_t optlen = sizeof(sendBufSize);

  // Get the send buffer size
  if (getsockopt(sv[0], SOL_SOCKET, SO_SNDBUF, &sendBufSize, &optlen) < 0) {
    perror("getsockopt");
    close(sv[0]);
    close(sv[1]);
    exit(EXIT_FAILURE);
  } else {
    std::cout << "Send buffer size: " << sendBufSize << " bytes\n";
  }

  // Get the receive buffer size
  if (getsockopt(sv[0], SOL_SOCKET, SO_RCVBUF, &recvBufSize, &optlen) < 0) {
    perror("getsockopt");
    close(sv[0]);
    close(sv[1]);
    exit(EXIT_FAILURE);
  } else {
    std::cout << "Receive buffer size: " << recvBufSize << " bytes\n";
  }

  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file) {
    std::cerr << "Failed to open file.\n";
    close(sv[0]);
    close(sv[1]);
    exit(EXIT_FAILURE);
  }
  std::cout << "File opened\n";

  auto fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  std::cout << "File size: " << fileSize << " bytes\n";
  std::cout << "Reading file...\n";
  std::vector<char> buffer(fileSize);
  if (!file.read(buffer.data(), fileSize)) {
    std::cerr << "Error reading the file.\n";
    file.close();
    close(sv[0]);
    close(sv[1]);
    exit(EXIT_FAILURE);
  }
  std::cout << "File read\n";
  file.close();
  std::cout << "File closed\n";

  // Write data to socketA
  std::cout << "Writing to socket...\n";
  auto startWrite = std::chrono::high_resolution_clock::now();
  std::cout << "Just before write\n";
  if (write(sv[0], buffer.data(), fileSize) < 0) {
    perror("write");
    close(sv[0]);
    close(sv[1]);
    return;
  }
  auto endWrite = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsedWrite = endWrite - startWrite;
  std::cout << "Time taken to write to socket: " << elapsedWrite.count()
            << " seconds\n";

  // Read data from socket
  std::vector<char> readBuffer(fileSize);
  auto startRead = std::chrono::high_resolution_clock::now();
  if (read(sv[1], readBuffer.data(), fileSize) < 0) {
    perror("read");
    close(sv[0]);
    close(sv[1]);
    return;
  }
  auto endRead = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsedRead = endRead - startRead;
  std::cout << "Time taken to read from socket: " << elapsedRead.count()
            << " seconds\n";

  close(sv[0]);
  close(sv[1]);
}
// With blcoking sockets, but with non bocking send and receive
bool testSocketIO(const std::string &filename) {
  int sv[2]; // Socket pair
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0) {
    perror("socketpair");
    exit(EXIT_FAILURE);
  }
  std::cout << "Socket pair created\n";
  int flags = fcntl(sv[0], F_GETFL, 0);
  if (flags & O_NONBLOCK) {
    std::cout << "Socket sv[0] is non-blocking.\n";
  } else {
    std::cout << "Socket sv[0] is blocking!\n";
  }
  int flags2 = fcntl(sv[1], F_GETFL, 0);
  if (flags2 & O_NONBLOCK) {
    std::cout << "Socket sv[1] is non-blocking.\n";
  } else {
    std::cout << "Socket sv[1] is blocking!\n";
  }
  // Make the sockets non-blocking
  if (NON_BLOCKING_SOCKETS) {
    if (fcntl(sv[0], F_SETFL, flags | O_NONBLOCK) < 0) {
      perror("fcntl");
      close(sv[0]);
      close(sv[1]);
      return false;
    }
    std::cout << "Socket sv[0] is now non-blocking.\n";
    if (fcntl(sv[1], F_SETFL, flags2 | O_NONBLOCK) < 0) {
      perror("fcntl");
      close(sv[0]);
      close(sv[1]);
      return false;
    }
    std::cout << "Socket sv[1] is now non-blocking.\n";
  }
  // Get the buffer size
  int sendBufSize, recvBufSize;
  socklen_t optlen = sizeof(sendBufSize);
  if (getsockopt(sv[0], SOL_SOCKET, SO_SNDBUF, &sendBufSize, &optlen) < 0) {
    perror("getsockopt");
    close(sv[0]);
    close(sv[1]);
    return false;
  } else {
    std::cout << "Send buffer size: " << sendBufSize << " bytes\n";
  }
  if (getsockopt(sv[0], SOL_SOCKET, SO_RCVBUF, &recvBufSize, &optlen) < 0) {
    perror("getsockopt");
    close(sv[0]);
    close(sv[1]);
    return false;
  } else {
    std::cout << "Receive buffer size: " << recvBufSize << " bytes\n";
  }
  ssize_t written;
  if (PREFILL) {
    // Prepare a buffer exactly the size of the send buffer
    std::vector<char> data(sendBufSize, '0');
    // Wiree exactly the size of the send buffer
    written = send(sv[0], data.data(), data.size(), 0);
    if (written < 0) {
      perror("send");
      close(sv[0]);
      close(sv[1]);
      return false;
    }
  }

  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file) {
    std::cerr << "Failed to open file.\n";
    close(sv[0]);
    close(sv[1]);
    return false;
  }
  std::cout << "First write, bytes written: " << written << std::endl;
  auto fileSize = file.tellg();
  file.seekg(0, std::ios::beg);
  size_t actualFileSize = static_cast<size_t>(fileSize);
  std::vector<char> buffer(fileSize);
  if (!file.read(buffer.data(), actualFileSize)) {
    std::cerr << "Error reading the file.\n";
    file.close();
    close(sv[0]);
    close(sv[1]);
    return false;
  }
  file.close();
  std::cout << "File copied to buffer and closed\n";

  size_t totalBytesWritten = 0;
  size_t totalBytesRead = 0;
  std::vector<char> readBuffer(actualFileSize, 0);

  std::cout << "Entering while loop\n" << std::endl;
  while (totalBytesWritten < actualFileSize ||
         totalBytesRead < actualFileSize) {
    if (totalBytesWritten < actualFileSize) {
      std::cout << "Writing to socket\n";
      std::cout << "Data length to write: "
                << actualFileSize - totalBytesWritten << std::endl;
      if (MSG_DONTWAIT_ON) {

        written = send(sv[0], buffer.data() + totalBytesWritten,
                       actualFileSize - totalBytesWritten, MSG_DONTWAIT_ON);
      } else {
        written = send(sv[0], buffer.data() + totalBytesWritten,
                       actualFileSize - totalBytesWritten, 0);
      }
      std::cout << "Written: " << written << std::endl;
      if (written < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("send");
        close(sv[0]);
        close(sv[1]);
        return false;
      } else
        totalBytesWritten += written;
    }
    if (totalBytesRead < actualFileSize) {
      std::cout << "Reading from socket\n";
      size_t read;
      if (MSG_DONTWAIT_ON) {

        read = recv(sv[1], readBuffer.data() + totalBytesRead,
                    actualFileSize - totalBytesRead, MSG_DONTWAIT_ON);
      } else {
        read = recv(sv[1], readBuffer.data() + totalBytesRead,
                    actualFileSize - totalBytesRead, 0);
      }
      std::cout << "Read: " << read << std::endl;
      if (read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("recv");
        close(sv[0]);
        close(sv[1]);
        return false;
      } else if (read == 0) {
        std::cout << "Connection closed\n";
        close(sv[0]);
        close(sv[1]);
        return false;
      } else
        totalBytesRead += read;
    }
  }
  close(sv[0]);
  close(sv[1]);
  return true;
}

int createUnixSocketServer(const std::string &socketPath) {
  int sockfd;

  // Create a Unix domain socket
  if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return -1;
  }

  struct sockaddr_un addr;
  // Set socket address structure
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

  // Remove the socket file if it already exists
  unlink(socketPath.c_str());

  // Bind the socket to the specified path
  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return -1;
  }

  // Listen for incoming connections
  if (listen(sockfd, 5) == -1) {
    perror("listen");
    return -1;
  }
  return sockfd;
}

void writeToUnixSocket(const std::string &socketPath,
                       const std::string &filename) {
  int sockfd;

  // Create a Unix domain socket
  if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_un addr;
  // Set socket address structure
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

  // Connect to the socket
  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("connect");
    close(sockfd);
    return;
  }

  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Failed to open file.\n";
    close(sockfd);
    return;

    std::vector<char> buffer(1024 * 1024, '0'); // 1MB buffer
    ssize_t totalBytesWritten = 0;

    auto start = std::chrono::high_resolution_clock::now();

    while (file.eof()) {
      file.read(buffer.data(), buffer.size());
      ssize_t bytesRead = file.gcount();
      if (send(sockfd, buffer.data(), bytesRead, 0) == -1) {
        perror("send");
        break;
      }
      totalBytesWritten += bytesRead;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedWrite = end - start;
    std::cout << "Time taken to write to socket: " << elapsedWrite.count()
              << " seconds\n";
  }
  // Close the socket
  close(sockfd);
}

void readFromUnixSocket(const std::string &socketPath) {
  int sockfd;

  // Create a Unix domain socket
  if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }
  struct sockaddr_un addr;

  // Set socket address structure
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

  // Connect to the socket
  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("connect");
    exit(EXIT_FAILURE);
  }

  std::vector<char> buffer(1024); // 1MB buffer
  ssize_t totalBytesRead = 0;
  bool isError = false;

  auto start = std::chrono::high_resolution_clock::now();
  while (true) {
    ssize_t bytesRead = recv(sockfd, buffer.data(), buffer.size(), 0);
    if (bytesRead == -1) {
      perror("recv");
      isError = true;
      exit(EXIT_FAILURE);
    }
    if (bytesRead == 0) {
      break;
    }
    totalBytesRead += bytesRead;
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsedRead = end - start;
  std::cout << "Time taken to read from socket: " << elapsedRead.count()
            << " seconds\n";

  // Close the socket
  close(sockfd);
}

void createLargeFile(const std::string &filename, std::size_t sizeInMB) {
  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Failed to create file.\n";
    return;
  }

  std::size_t bufferSize = 1024 * 1024; // 1MB buffer
  std::vector<char> buffer(bufferSize,
                           '0');    // Fill the buffer with '0' characters
  std::size_t numWrites = sizeInMB; // Number of writes of 1MB needed

  for (std::size_t i = 0; i < numWrites; ++i) {
    file.write(buffer.data(), buffer.size());
    if (!file) {
      std::cerr << "Failed to write to file.\n";
      break;
    }
  }

  file.close();
  std::cout << "File created successfully: " << filename << std::endl;
  // Reopen the file to check its size
  std::ifstream inFile(filename, std::ios::binary | std::ios::ate);
  if (!inFile) {
    std::cerr << "Failed to reopen file to check size.\n";
  } else {
    auto fileSizeBytes = inFile.tellg(); // Get the size of the file
    inFile.close();
    std::cout << "Size of the file: " << fileSizeBytes << " bytes\n";
    double fileSizeMB =
        static_cast<double>(fileSizeBytes) / (1024 * 1024); // Convert to MB
    std::cout << "Size of the file: " << fileSizeMB << " MB\n";
  }
}

int main() {
  std::string filename = "largefile.dat";
  std::string smallFilename = "smallfile.txt";
  std::size_t sizeInMB = 100;
  createLargeFile(filename, sizeInMB);
  std::ifstream file(filename,
                     std::ios::binary |
                         std::ios::ate); // Open at end to get size

  if (!file) {
    std::cerr << "File could not be opened.\n";
    return 1;
  }

  auto fileSize = file.tellg();
  file.seekg(0, std::ios::beg); // Move to the beginning of the file

  std::vector<char> buffer(fileSize);

  auto start = std::chrono::high_resolution_clock::now();
  if (file.read(buffer.data(), fileSize)) {
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken to read file: " << elapsed.count()
              << " seconds.\n";
  } else {
    std::cerr << "Error reading the file.\n";
  }

  file.close();
  std::string socketPath = "/tmp/my_unix_socket";

  //   testSocketIOBlocking(filename);
  testSocketIO(filename);
  //   testSocketIOBlocking(smallFilename);
  return 0;
}
