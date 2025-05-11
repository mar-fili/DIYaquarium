from PyQt5 import QtWidgets

class MainMenu(QtWidgets.QWidget):
    def __init__(self, stacked_widget):
        super().__init__()
        self.stacked_widget = stacked_widget
        layout = QtWidgets.QVBoxLayout(self)

        label = QtWidgets.QLabel("Wybierz akcję:")
        layout.addWidget(label)

        btn_schedule = QtWidgets.QPushButton("Edytor Harmonogramu")
        btn_schedule.clicked.connect(self.open_schedule_editor)
        layout.addWidget(btn_schedule)

        btn_temperature = QtWidgets.QPushButton("Pomiar temperatury")
        btn_temperature.clicked.connect(self.open_temperature)
        layout.addWidget(btn_temperature)

        btn_exit = QtWidgets.QPushButton("Wyjście")
        btn_exit.clicked.connect(QtWidgets.qApp.quit)
        layout.addWidget(btn_exit)

    def open_schedule_editor(self):
        self.stacked_widget.setCurrentIndex(1)
    
    def open_temperature(self):
        self.stacked_widget.setCurrentIndex(2)
