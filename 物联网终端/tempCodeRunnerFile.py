import sys
from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtCore import QTimer, QDateTime
from PyQt5.QtGui import QFont, QPalette, QBrush, QPixmap

class Ui_mainWindow(object):
    def setupUi(self, mainWindow):
        mainWindow.setObjectName("mainWindow")
        mainWindow.resize(794, 513)

        # 设置背景图片
        self.set_background_image(mainWindow)

        self.centralwidget = QtWidgets.QWidget(mainWindow)
        self.centralwidget.setObjectName("centralwidget")

        # 添加时间、温度、湿度显示
        self.info_label = QtWidgets.QLabel(self.centralwidget)
        self.info_label.setGeometry(QtCore.QRect(10, 10, 600, 30))
        self.info_label.setFont(QFont('Arial', 12))
        self.info_label.setObjectName("info_label")

        # 设置按钮
        self.pushButton = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton.setGeometry(QtCore.QRect(50, 270, 200, 200))
        self.pushButton.setObjectName("pushButton")

        self.pushButton_2 = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_2.setGeometry(QtCore.QRect(300, 270, 200, 200))
        self.pushButton_2.setObjectName("pushButton_2")

        self.pushButton_3 = QtWidgets.QPushButton(self.centralwidget)
        self.pushButton_3.setGeometry(QtCore.QRect(550, 270, 200, 200))
        self.pushButton_3.setObjectName("pushButton_3")

        # 日历
        self.calendarWidget = QtWidgets.QCalendarWidget(self.centralwidget)
        self.calendarWidget.setGeometry(QtCore.QRect(500, 30, 248, 183))
        self.calendarWidget.setObjectName("calendarWidget")

        # LCD 显示
        self.lcdNumber = QtWidgets.QLCDNumber(self.centralwidget)
        self.lcdNumber.setGeometry(QtCore.QRect(290, 30, 191, 81))
        self.lcdNumber.setObjectName("lcdNumber")

        mainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(mainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 794, 22))
        self.menubar.setObjectName("menubar")
        self.menu = QtWidgets.QMenu(self.menubar)
        self.menu.setObjectName("menu")
        mainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(mainWindow)
        self.statusbar.setObjectName("statusbar")
        mainWindow.setStatusBar(self.statusbar)
        self.menubar.addAction(self.menu.menuAction())

        self.retranslateUi(mainWindow)
        QtCore.QMetaObject.connectSlotsByName(mainWindow)

        # 定时更新时间
        timer = QTimer(mainWindow)
        timer.timeout.connect(self.update_time)
        timer.start(1000)  # 每秒更新一次

    def set_background_image(self, mainWindow):
        # 设置背景图片并拉伸填满窗口
        palette = QPalette()
        pixmap = QPixmap("background_image.png")  # 替换成你的背景图片路径
        scaled_pixmap = pixmap.scaled(mainWindow.size(), QtCore.Qt.IgnoreAspectRatio, QtCore.Qt.SmoothTransformation)
        palette.setBrush(QPalette.Background, QBrush(scaled_pixmap))
        mainWindow.setPalette(palette)

    def update_time(self):
        # 更新当前时间和固定的温度、湿度信息
        current_time = QDateTime.currentDateTime().toString('yyyy-MM-dd hh:mm:ss')
        self.info_label.setText(f"实验室仓库智能管理系统")

    def retranslateUi(self, mainWindow):
        _translate = QtCore.QCoreApplication.translate
        mainWindow.setWindowTitle(_translate("mainWindow", "主页面"))
        self.pushButton.setText(_translate("mainWindow", "数据库注册"))
        self.pushButton_2.setText(_translate("mainWindow", "指纹登录"))
        self.pushButton_3.setText(_translate("mainWindow", "账号密码"))
        self.menu.setTitle(_translate("mainWindow", "实验室仓库智能管理系统"))

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    mainWindow = QtWidgets.QMainWindow()
    ui = Ui_mainWindow()
    ui.setupUi(mainWindow)
    mainWindow.show()
    sys.exit(app.exec_())
