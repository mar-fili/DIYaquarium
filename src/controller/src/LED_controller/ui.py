import json
from PyQt5 import QtWidgets
from .plot import TimeAxisItem
from .constants import COLOR_MAP
from .movable_point import MovablePoint
import pyqtgraph as pg
import socket
import time

class ScheduleEditor(QtWidgets.QMainWindow):
    def __init__(self, stacked_widget=None):
        super().__init__()
        self.setWindowTitle("LED Scheduler")
        self.resize(800, 600)
        self.stacked_widget = stacked_widget

        self.central_widget = QtWidgets.QWidget(self)
        self.setCentralWidget(self.central_widget)
        self.layout = QtWidgets.QVBoxLayout(self.central_widget)

        self.plot = pg.PlotWidget(axisItems={'bottom': TimeAxisItem(orientation='bottom')})
        self.plot.setLimits(xMin=0, xMax=24)
        self.plot.setLimits(yMin=0, yMax=100)
        self.plot.setRange(xRange=(0, 24), yRange=(0, 100))

        self.plot.showGrid(x=True, y=True)
        self.layout.addWidget(self.plot)

        self.points = {'r': [], 'wWarm': [], 'b': [], 'wCold': []}
        self.lines = {
            color: pg.PlotDataItem(pen=pg.mkPen(COLOR_MAP[color], width=2))
            for color in COLOR_MAP
        }
        for line in self.lines.values():
            self.plot.plotItem.addItem(line)

        self.current_color = 'r'

        self.back_button = QtWidgets.QPushButton("← Powrót do menu")
        self.back_button.setFixedWidth(150)
        self.back_button.clicked.connect(lambda: self.stacked_widget.setCurrentIndex(0))
        self.layout.addWidget(self.back_button)

        self.send_button = QtWidgets.QPushButton("Wyslij")
        self.send_button.clicked.connect(self.handle_send)
        self.layout.addWidget(self.send_button)

        self.red_button = QtWidgets.QPushButton("Red", self)
        self.red_button.clicked.connect(lambda: self.set_color('r'))
        self.layout.addWidget(self.red_button)

        self.green_button = QtWidgets.QPushButton("wWarm", self)
        self.green_button.clicked.connect(lambda: self.set_color('wWarm'))
        self.layout.addWidget(self.green_button)

        self.blue_button = QtWidgets.QPushButton("Blue", self)
        self.blue_button.clicked.connect(lambda: self.set_color('b'))
        self.layout.addWidget(self.blue_button)

        self.yellow_button = QtWidgets.QPushButton("wCold", self)
        self.yellow_button.clicked.connect(lambda: self.set_color('wCold'))
        self.layout.addWidget(self.yellow_button)

        self.export_button = QtWidgets.QPushButton("Export to JSON", self)
        self.export_button.clicked.connect(self.export_to_json)
        self.layout.addWidget(self.export_button)

        self.plot.scene().sigMouseClicked.connect(self.on_click)

    def handle_send(self):
        for color, points in self.points.items():
            color_points = []
            for point in points:
                raw_x = round(point.x, 2)
                hour = int(raw_x)
                minute = int((raw_x - hour) * 60)
                y = int(point.y)
                color_points.append(f"{hour}:{minute},{y}")
            
            color_str = f"{color}=" + ';'.join(color_points)
            if len(color_points) > 0:
                self.cmd = "SCHED?" + "&" + color_str + "&"
            else:
                continue
            try:
                conn = socket.create_connection(('192.168.1.65', 80), timeout=5)
            except socket.error as e:
                raise ConnectionError(f"Connection failed: {e}")
            
            try:
                request = ("GET /" + self.cmd).encode('utf-8')
                conn.sendall(request)
                response = b""
                while True:
                    chunk = conn.recv(4096)
                    if not chunk:
                        break
                    response += chunk
                response = response.decode('utf-8')
                print("Odpowiedź z urządzenia:", response)
            except socket.error as e:
                raise IOError(f"Send or read failed: {e}")
            finally:
                conn.close()
                time.sleep(5)

    def set_color(self, color):
        """Zmiana bieżącego koloru"""
        self.current_color = color
        self.update_lines(self.current_color)

    def on_click(self, event):
        if event.button() == pg.QtCore.Qt.LeftButton:
            mousePoint = event.scenePos()
            mousePoint = self.plot.plotItem.vb.mapSceneToView(event.scenePos())
            x = max(0, min(24, mousePoint.x()))
            y = max(0, min(100, mousePoint.y()))
            self.add_point(x, y)

    def add_point(self, x, y):
        point = MovablePoint(x, y, self.plot, self.current_color, self)
        self.plot.addItem(point)
        self.points[self.current_color].append(point)
        self.update_lines(self.current_color)

    def update_lines(self, color):
        """Aktualizowanie linii tylko dla wybranego koloru"""
        if not self.points[color]:
            return

        positions = [(p.x, p.y) for p in self.points[color]]
        positions.sort(key=lambda pos: pos[0])
        xs, ys = zip(*positions)
        self.lines[color].setData(xs, ys)

    def export_to_json(self):
        data = {}
        for color, points in self.points.items():
            data[color] = [{'x': p.x, 'y': p.y} for p in points]
        
        with open('points_schedule.json', 'w') as json_file:
            json.dump(data, json_file, indent=4)
        print("Dane zapisane do points_schedule.json")