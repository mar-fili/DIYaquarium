import pyqtgraph as pg

class TimeAxisItem(pg.AxisItem):
    def tickStrings(self, values, scale, spacing):
        strings = []
        for value in values:
            total_minutes = int(value * 60)  # 1.5h -> 90 minut
            hours = total_minutes // 60
            minutes = total_minutes % 60
            strings.append(f"{hours:02d}:{minutes:02d}")
        return strings
