import asyncio
import aiohttp

GREEN = "\033[92m"
RED = "\033[91m"
RESET = "\033[0m"

BUFFER_SIZE = 1024

headers_buffer_size = {
    "Host": "www.example.com",
    "X-Custom-Header": "A" * (BUFFER_SIZE + 1),
}

headers_8kb = {"Host": "www.example.com", "X-Custom-Header": "A" * (BUFFER_SIZE * 8)}

headers_chunked = {"Host": "www.example.com", "Transfer-Encoding": "chunked"}


url = "http://localhost:8080"


async def print_message(status, expected_status, message):
    if status == expected_status:
        print(GREEN + f"{status} OK " + message + RESET)
    else:
        print(RED + f"{status} KO " + message + RESET)


async def upload_multiple_file():
    async with aiohttp.ClientSession() as session:
        data = aiohttp.FormData()
        file_paths = ["a.txt", "b.txt", "c.txt"]
        for file_path in file_paths:
            data.add_field(
                "file",
                open(file_path, "rb"),
                filename=file_path,
                content_type="text/tab-separated-values",
            )
        async with session.post(url, data=data) as response:
            await print_message(response.status, 200, "multiple file upload")


async def upload_large_file(file_name):
    async with aiohttp.ClientSession() as session:
        data = aiohttp.FormData()
        data.add_field(
            "file",
            open(file_name, "rb"),
            filename=file_name,
            content_type="text/tab-separated-values",
        )
        async with session.post(url, data=data) as response:
            await print_message(response.status, 200, "file upload")


async def upload_file(file_name):
    async with aiohttp.ClientSession() as session:
        data = aiohttp.FormData()
        data.add_field(
            "file",
            open(file_name, "rb"),
            filename=file_name,
            content_type="text/tab-separated-values",
        )
        async with session.post(url, data=data) as response:
            await print_message(response.status, 200, "file upload")


async def chunked_request():
    async with aiohttp.ClientSession() as session:
        async with session.post(
            url, headers=headers_chunked, data="A" * (BUFFER_SIZE * 8)
        ) as response:
            await print_message(response.status, 200, "chunked request")


async def func_headers_8kb():
    async with aiohttp.ClientSession() as session:
        async with session.get(url, headers=headers_8kb) as response:
            await print_message(response.status, 431, "headers > 8KB")


async def func_headers_buffer_size():
    async with aiohttp.ClientSession() as session:
        async with session.get(url, headers=headers_buffer_size) as response:
            await print_message(response.status, 200, "headers > buffer_size")


# send requests async
async def main():
    await upload_large_file("5mb.jpg")  # large file to test non-blocking
    await func_headers_buffer_size()  # header bigger than 1024 bytes(server should read in multiple reads)
    await func_headers_8kb()  # header bigger than 8KB (server should send 431)
    await chunked_request()  # chunked request
    await upload_file("a.txt")  # upload singe small file
    await upload_multiple_file()  # upload 3 files at the same time (in one POST request)


if __name__ == "__main__":
    asyncio.run(main())
