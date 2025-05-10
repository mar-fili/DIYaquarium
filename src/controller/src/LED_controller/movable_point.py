import pyqtgraph as pg
from .constants import COLOR_MAP

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