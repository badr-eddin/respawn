from http.server import BaseHTTPRequestHandler, HTTPServer
import json

class MyRequestHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        # Get content length to read the body of the request
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length).decode('utf-8')

        # Try to parse JSON data
        try:
            data = json.loads(post_data)
            for key, value in data.items():
                print(f"{key}: {value}")
        except json.JSONDecodeError:
            print("Received form data:")

            form_data = post_data.split('&')
            for pair in form_data:
                key, value = pair.split('=')
                print(f"{key}: {value}")

        # Send response
        self.send_response(200)
        self.end_headers()

# Set up the server
def run(server_class=HTTPServer, handler_class=MyRequestHandler, port=8080):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    print(f"Starting server on port {port}...")
    httpd.serve_forever()

if __name__ == '__main__':
    run()
