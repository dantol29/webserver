import requests


def main():
    server_url = "http://server:8000"  # Assuming 'server' is the name of your web server service in Docker Compose and listens on port 8000
    response = requests.get(server_url)
    print(f"Response from server: {response.text}")


if __name__ == "__main__":
    main()
