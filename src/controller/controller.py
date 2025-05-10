import sys
import json
from PyQt5 import QtWidgets
import pyqtgraph as pg
import socket


COLOR_MAP = {
    'r': 'red',
    'b': 'blue',
    'wWarm': (255, 165, 0),
    'wCold': (173, 216, 230)
}

class MovablePoint(pg.ScatterPlotItem):
    def __init__(self, x, y, plot, color, schedule_editor):
        super().__init__(pos=[[x, y]], size=10, brush=pg.mkBrush(COLOR_MAP[color]), pen=pg.mkPen('k'), symbol='o')
        self.setZValue(10)
        self.plot = plot
        self.schedule_editor = schedule_editor
        self.setParentItem(plot.getPlotItem())
        self.moving = False
        self.x = x
        self.y = y
        self.color = color
        self.setBrush(pg.mkBrush(COLOR_MAP[color]))

    def mouseDragEvent(self, ev):
        if ev.button() == pg.QtCore.Qt.LeftButton:
            if ev.isStart():
                self.moving = True
                ev.accept()
            elif ev.isFinish():
                self.moving = False
                ev.accept()
            if self.moving:
                newPos = self.plot.plotItem.vb.mapSceneToView(ev.scenePos())
                x = max(0, min(24, newPos.x()))
                y = max(0, min(100, newPos.y()))
                self.setData(pos=[[x, y]])
                self.x, self.y = x, y
                self.schedule_editor.update_lines(self.color)

class ScheduleEditor(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("LED Scheduler")
        self.resize(800, 600)

        self.central_widget = QtWidgets.QWidget(self)
        self.setCentralWidget(self.central_widget)
        self.layout = QtWidgets.QVBoxLayout(self.central_widget)
        self.cmd = "Set 100 000 000 000"

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
        parts = []
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

class TimeAxisItem(pg.AxisItem):
    def tickStrings(self, values, scale, spacing):
        strings = []
        for value in values:
            total_minutes = int(value * 60)  # 1.5h -> 90 minut
            hours = total_minutes // 60
            minutes = total_minutes % 60
            strings.append(f"{hours:02d}:{minutes:02d}")
        return strings

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    editor = ScheduleEditor()
    editor.show()
    sys.exit(app.exec_())