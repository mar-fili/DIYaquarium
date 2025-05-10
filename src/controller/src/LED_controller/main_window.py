from PyQt5 import QtWidgets
from LED_controller.main_menu import MainMenu
from LED_controller.ui import ScheduleEditor

class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("DIYaquarium - Kontroler")
        self.resize(800, 600)

        self.stacked_widget = QtWidgets.QStackedWidget()
        self.setCentralWidget(self.stacked_widget)

        self.main_menu = MainMenu(self.stacked_widget)
        self.schedule_editor = ScheduleEditor(self.stacked_widget)

        self.stacked_widget.addWidget(self.main_menu)         # index 0
        self.stacked_widget.addWidget(self.schedule_editor)   # index 1

        self.stacked_widget.setCurrentIndex(0)
