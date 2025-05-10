from PyQt5 import QtWidgets
from LED_controller.main_window import MainWindow
import sys

def main():
    app = QtWidgets.QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
