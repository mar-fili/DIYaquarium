from PyQt5 import QtWidgets
import socket

class TemperatureWindow(QtWidgets.QMainWindow):
    def __init__(self, stacked_widget=None):
        super().__init__()
        self.setWindowTitle("DIYaquarium - Kontroler")
        self.resize(800, 600)
        self.stacked_widget = stacked_widget

        self.central_widget = QtWidgets.QWidget(self)
        self.setCentralWidget(self.central_widget)
        self.layout = QtWidgets.QVBoxLayout(self.central_widget)

        self.back_button = QtWidgets.QPushButton("← Powrót do menu")
        self.back_button.setFixedWidth(150)
        self.back_button.clicked.connect(lambda: self.stacked_widget.setCurrentIndex(0))
        self.layout.addWidget(self.back_button)

        self.read_button = QtWidgets.QPushButton("Zmierz temperaturę")
        self.read_button.clicked.connect(self.handle_read)
        self.layout.addWidget(self.read_button)

        self.temp_label = QtWidgets.QLabel("Odczytana temperatura:")
        self.layout.addWidget(self.temp_label)

    def handle_read(self):
        self.cmd = "Temp"

        try:
            conn = socket.create_connection(('192.168.1.65', 80), timeout=5)
            request = ("GET /" + self.cmd).encode('utf-8')
            conn.sendall(request)
            response = b""
            while True:
                chunk = conn.recv(4096)
                if not chunk:
                    break
                response += chunk
            conn.close()
            self.temp_label.setText(f"Odpowiedź: {response.decode('utf-8')}")
        except socket.error as e:
            self.temp_label.setText(f"Błąd połączenia: {e}")
        finally:
            if conn:
                conn.close()
