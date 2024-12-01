from PySide6.QtCore import QObject, QThread, Signal, QUrl
from PySide6.QtWebSockets import QWebSocketServer
from PySide6.QtNetwork import QHostAddress
import json
import http.server as hpserv
import socketserver
from typing import Optional, Callable
from pathlib import Path

class HttpHandler(hpserv.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/":
            p = Path(__file__).absolute().parent / "web_files" / "test.html"
            
            if p.exists():
                # Send HTTP headers
                self.send_response(200)
                self.send_header("Content-type", "text/html")
                self.end_headers()
                
                # Serve the file content
                with open(p, "rb") as file:
                    self.wfile.write(file.read())
                return
            else:
                # File not found
                self.send_error(404, "File not found")
                return
        return hpserv.SimpleHTTPRequestHandler.do_GET(self)

    def end_headers(self):
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate')
        self.send_header('Pragma', 'no-cache')
        self.send_header('Expires', '0')
        return super().end_headers()

class WebApp(QThread):
    recv_data = Signal(dict)
    trans_data = Signal(dict)

    def __init__(self):
        super().__init__()
        self.start_callback: Optional[Callable[[], dict]] = None
        self.websocket = QWebSocketServer("Web App Server", QWebSocketServer.SslMode.NonSecureMode)
        self.websocket.listen(QHostAddress("0.0.0.0"), 5000)
        self.websocket.newConnection.connect(self.on_new_connection)
        self.trans_data.connect(self.send_json)

    def set_start_callback(self, cb: Callable[[], dict]):
        self.start_callback = cb

    def on_new_connection(self):
        self.socket = self.websocket.nextPendingConnection()
        self.socket.textMessageReceived.connect(self.recv_json)
        if self.start_callback is not None:
            self.socket.sendTextMessage(json.dumps(self.start_callback()))
    
    def recv_json(self, data):
        self.recv_data.emit(json.loads(data))

    def send_json(self, data: dict):
        self.socket.sendTextMessage(json.dumps(data))

    def run(self):
        with socketserver.TCPServer(("", 12345), HttpHandler) as httpd:
            httpd.serve_forever()
