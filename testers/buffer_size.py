import requests
import asyncio
import aiohttp

GREEN = '\033[92m'
RED = '\033[91m'
RESET = '\033[0m'

BUFFER_SIZE = 1024

headers_buffer_size = {
	"Host": "example.com",
    "X-Custom-Header": "A" * (BUFFER_SIZE + 1)
}

headers_8kb = {
	"Host": "example.com",
    "X-Custom-Header": "A" * (BUFFER_SIZE * 8)
}

headers_chunked = {
    "Host": "example.com",
    "Transfer-Encoding": "chunked"
}

url = "http://localhost:8080"

async def test_chunked_request():
	async with aiohttp.ClientSession() as session:
		async with session.post(url, headers=headers_chunked, data="A" * (BUFFER_SIZE * 8)) as response:
			status_code = response.status
			if status_code == 200:
				print(GREEN + f"{status_code} OK" + RESET)
			else:
				print(RED + f"{status_code} KO" + RESET)

async def fetch_data(url, headers):
	async with aiohttp.ClientSession() as session:
		async with session.get(url, headers=headers) as response:
			if response.status == 200:
				print(GREEN + f"{response.status} OK: " + RESET)
			else:
				print(RED + f"{response.status} KO: " + RESET)

async def main():
	await fetch_data(url, headers_buffer_size)
	await fetch_data(url, headers_8kb)
	await test_chunked_request()


if __name__ == "__main__":
    asyncio.run(main())