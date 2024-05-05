import requests
import asyncio
import aiohttp

GREEN = '\033[92m'
RED = '\033[91m'
RESET = '\033[0m'

BUFFER_SIZE = 1024

headers_buffer_size = {
	"Host": "www.example.com",
    "X-Custom-Header": "A" * (BUFFER_SIZE + 1)
}

headers_8kb = {
	"Host": "www.example.com",
    "X-Custom-Header": "A" * (BUFFER_SIZE * 8)
}

headers_chunked = {
    "Host": "www.example.com",
    "Transfer-Encoding": "chunked"
}


url = "http://localhost:8080"

async def print_message(status, message):
	if status == 200:
		print(GREEN + f"{status} OK " + message + RESET)
	else:
		print(RED + f"{status} KO " + message + RESET)

async def upload_multiple_file():
	async with aiohttp.ClientSession() as session:
		data = aiohttp.FormData()
		file_paths = ["a.txt", "b.txt", "c.txt"]
		for file_path in file_paths:
			data.add_field('file',open(file_path, 'rb'), filename=file_path, content_type='text/tab-separated-values')
		async with session.post(url, data=data) as response:
			await print_message(response.status, "multiple file upload")

async def upload_large_file(file_name):
	async with aiohttp.ClientSession() as session:
		data = aiohttp.FormData()
		data.add_field('file',open(file_name, 'rb'), filename=file_name, content_type='text/tab-separated-values')
		async with session.post(url, data=data) as response:
			await print_message(response.status, "file upload")

async def upload_file(file_name):
	async with aiohttp.ClientSession() as session:
		data = aiohttp.FormData()
		data.add_field('file',open(file_name, 'rb'), filename=file_name, content_type='text/tab-separated-values')
		async with session.post(url, data=data) as response:
			await print_message(response.status, "file upload")

async def chunked_request():
	async with aiohttp.ClientSession() as session:
		async with session.post(url, headers=headers_chunked, data="A" * (BUFFER_SIZE * 8)) as response:
			await print_message(response.status, "chunked request")

async def fetch_data(url, headers, message):
	async with aiohttp.ClientSession() as session:
		async with session.get(url, headers=headers) as response:
			await print_message(response.status, message)

async def main():
	await upload_large_file("5mb.jpg")
	await fetch_data(url, headers_buffer_size, "headers > buffer_size")
	await fetch_data(url, headers_8kb, "headers > 8KB")
	await chunked_request()
	await upload_file("a.txt")
	await upload_multiple_file()


if __name__ == "__main__":
    asyncio.run(main())