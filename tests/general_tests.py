import requests
import asyncio
import aiofiles
import aiohttp
import sys

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

async def print_message(status, expected_status, message):
	global is_error
	if status == expected_status:
		print(GREEN + f"{status} OK " + message + RESET)
	else:
		print(RED + f"{status} KO " + message + RESET)
		is_error = True

async def upload_multiple_file():
	async with aiohttp.ClientSession() as session:
		data = aiohttp.FormData()
		file_paths = ["a.txt", "b.txt", "c.txt"]
		for file_path in file_paths:
			data.add_field('file',open(file_path, 'rb'), filename=file_path, content_type='text/tab-separated-values')
		async with session.post(url, data=data) as response:
			await print_message(response.status, 200, "multiple file upload")

async def upload_large_file(file_name):
	async with aiohttp.ClientSession() as session:
		data = aiohttp.FormData()
		data.add_field('file',open(file_name, 'rb'), filename=file_name, content_type='text/tab-separated-values')
		async with session.post(url, data=data) as response:
			await print_message(response.status, 200, "file upload")

async def upload_file(file_name):
	async with aiohttp.ClientSession() as session:
		data = aiohttp.FormData()
		data.add_field('file',open(file_name, 'rb'), filename=file_name, content_type='text/tab-separated-values')
		async with session.post(url, data=data) as response:
			await print_message(response.status, 200, "file upload")

async def chunked_request():
	async with aiohttp.ClientSession() as session:
		async with session.post(url, headers=headers_chunked, data="A" * (BUFFER_SIZE * 8)) as response:
			await print_message(response.status, 200, "chunked request")

async def func_headers_8kb():
	async with aiohttp.ClientSession() as session:
		async with session.get(url, headers=headers_8kb) as response:
			await print_message(response.status, 431, "headers > 8KB")

async def func_headers_buffer_size():
	async with aiohttp.ClientSession() as session:
		async with session.get(url, headers=headers_buffer_size) as response:
			await print_message(response.status, 200, "headers > buffer_size")

async def file_sender(file_name):
	try:
		async with aiofiles.open(file_name, 'rb') as f:
			chunk = await f.read(900)  # 900 bytes chunks
			while chunk:
				print(f'Sending chunk of size: {len(chunk)} bytes')  # Print chunk size
				yield chunk
				chunk = await f.read(1024)
		print("Finished reading file.")
	except Exception as e:
		print(f"Error during file reading: {e}")

async def send_chunked_request(file_name):
	async with aiohttp.ClientSession() as session:
		async with session.post(url, data=file_sender(file_name)) as response:
			print(await response.text())

# send requests async
async def main():
	global is_error

	is_error = False
	await upload_large_file("5mb.jpg") # large file to test non-blocking
	await func_headers_buffer_size() # header bigger than 1024 bytes(server should read in multiple reads)
	await func_headers_8kb() # header bigger than 8KB (server should send 431)
	# await chunked_request() # chunked request
	await upload_file("a.txt") # upload singe small file
	await upload_multiple_file() # upload 3 files at the same time (in one POST request)
	await send_chunked_request("5mb.jpg") # chunked request with file
	if (is_error == True):
		sys.exit(1)
	sys.exit(0)

if __name__ == "__main__":
    asyncio.run(main())