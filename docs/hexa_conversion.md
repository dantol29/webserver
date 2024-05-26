# About the hexa conversion

We perform this hexa conversion

```c++
bool Connection::readChunkedBody(Parser &parser)
{
	// TODO: check if this is blocking; I mean the two recvs in readChunkSize and readChunk
	if (!parser.getBodyComplete())
	{
		std::string chunkSizeLine;
		// readChiunkSize cuould be a method of Connection, now it's a free function.
		if (!readChunkSize(chunkSizeLine))
			return false;

		std::istringstream iss(chunkSizeLine);
		size_t chunkSize;
		if (!(iss >> std::hex >> chunkSize))
			return false;

		if (chunkSize == 0)
		{
			parser.setBodyComplete(true);
			return true;
		}
		else
		{
			std::string chunkData;
			if (!readChunk(chunkSize, chunkData, _response))
				return false;
			parser.setBuffer(parser.getBuffer() + chunkData);
			return true;
		}
	}
	return false;
}
```

Convert the hexadecimal string from `chunkSizeLine` to a size_t value.
Here, `std::istringstream` is used to create a stream from the string, which then allows for input operations similar to cin. The `std::hex` manipulator is used to interpret the input as a hexadecimal value.
We attempt to stream the input into the `chunkSize` variable. If this operation fails (e.g., because of invalid input characters that can't be interpreted as hex), the stream's failbit is set, and the conditional check fails. In this case, we return false indicating an error in parsing the chunk size.
