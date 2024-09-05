import sys
from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtCore import QTimer, QDateTime
from PyQt5.QtGui import QFont, QPalette, QBrush, QPixmap
from PyQt5.QtWidgets import QMessageBox

class Ui_mainWindow(object):
    def setupUi(self, mainWindow):
        mainWindow.setObjectName("mainWindow")
        mainWindow.resize(794, 513)

        # 设置背景图片
        self.set_background_image(mainWindow)

        self.centralwidget = QtWidgets.QWidget(mainWindow)
        self.stackedWidget = QtWidgets.QStackedWidget(self.centralwidget)
        self.stackedWidget.setGeometry(QtCore.QRect(0, 0, 794, 513))

        # 主页面
        self.main_page = QtWidgets.QWidget()
        self.setup_main_page(self.main_page)
        self.stackedWidget.addWidget(self.main_page)

        # 数据库注册页面
        self.db_page = QtWidgets.QWidget()
        self.setup_sub_page_reg(self.db_page, "数据库注册页面")
        self.stackedWidget.addWidget(self.db_page)

        # 指纹登录页面
        self.fingerprint_page = QtWidgets.QWidget()
        self.setup_sub_page(self.fingerprint_page, "指纹登录页面")
        self.stackedWidget.addWidget(self.fingerprint_page)

        # 账号密码页面
        self.password_page = QtWidgets.QWidget()
        self.setup_sub_page(self.password_page, "账号密码页面")
        self.stackedWidget.addWidget(self.password_page)

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

    def setup_main_page(self, page):
        # 设置主页面内容
        page.setObjectName("main_page")

        # LCD 显示时间
        self.lcdNumber = QtWidgets.QLCDNumber(page)
        self.lcdNumber.setGeometry(QtCore.QRect(50, 30, 191, 81))
        self.lcdNumber.setObjectName("lcdNumber")

        # 日历
        self.calendarWidget = QtWidgets.QCalendarWidget(page)
        self.calendarWidget.setGeometry(QtCore.QRect(500, 30, 248, 183))
        self.calendarWidget.setObjectName("calendarWidget")

        # 设置三个按钮
        self.pushButton = QtWidgets.QPushButton(page)
        self.pushButton.setGeometry(QtCore.QRect(50, 270, 200, 200))
        self.pushButton.setObjectName("pushButton")
        self.pushButton.clicked.connect(lambda: self.stackedWidget.setCurrentWidget(self.db_page))

        self.pushButton_2 = QtWidgets.QPushButton(page)
        self.pushButton_2.setGeometry(QtCore.QRect(300, 270, 200, 200))
        self.pushButton_2.setObjectName("pushButton_2")
        self.pushButton_2.clicked.connect(lambda: self.stackedWidget.setCurrentWidget(self.fingerprint_page))

        self.pushButton_3 = QtWidgets.QPushButton(page)
        self.pushButton_3.setGeometry(QtCore.QRect(550, 270, 200, 200))
        self.pushButton_3.setObjectName("pushButton_3")
        self.pushButton_3.clicked.connect(lambda: self.stackedWidget.setCurrentWidget(self.password_page))

    def setup_sub_page(self, page, label_text):
        # 设置子页面内容
        page.setObjectName(label_text)

        # 返回按钮
        self.back_button = QtWidgets.QPushButton(page)
        self.back_button.setGeometry(QtCore.QRect(10, 10, 80, 30))
        self.back_button.setText("返回")
        self.back_button.clicked.connect(lambda: self.stackedWidget.setCurrentWidget(self.main_page))

        # 页面标签
        self.page_label = QtWidgets.QLabel(page)
        self.page_label.setGeometry(QtCore.QRect(200, 100, 500, 100))
        self.page_label.setFont(QFont('Arial', 24))
        self.page_label.setText(label_text)
        self.page_label.setStyleSheet("background-color: white; border: 3px solid black;")
        self.page_label.setAlignment(QtCore.Qt.AlignCenter)

    def setup_sub_page_reg(self, page, label_text):
        # 设置子页面内容
        page.setObjectName(label_text)

        # 返回按钮
        self.back_button = QtWidgets.QPushButton(page)
        self.back_button.setGeometry(QtCore.QRect(10, 10, 80, 30))
        self.back_button.setText("返回")
        self.back_button.clicked.connect(lambda: self.stackedWidget.setCurrentWidget(self.main_page))

        # 页面标签
        self.page_label = QtWidgets.QLabel(page)
        self.page_label.setGeometry(QtCore.QRect(200, 100, 500, 100))
        self.page_label.setFont(QFont('Arial', 24))
        self.page_label.setText(label_text)
        self.page_label.setStyleSheet("background-color: white; border: 3px solid black;")
        self.page_label.setAlignment(QtCore.Qt.AlignCenter)

        # 添加三个按钮，布局和主页面一样
        self.clear_db_button = QtWidgets.QPushButton(page)
        self.clear_db_button.setGeometry(QtCore.QRect(50, 270, 200, 200))
        self.clear_db_button.setText("清空数据库")
        self.clear_db_button.clicked.connect(self.clear_database)

        self.register_user_button = QtWidgets.QPushButton(page)
        self.register_user_button.setGeometry(QtCore.QRect(300, 270, 200, 200))
        self.register_user_button.setText("注册用户")
        self.register_user_button.clicked.connect(self.register_user)

        self.delete_user_button = QtWidgets.QPushButton(page)
        self.delete_user_button.setGeometry(QtCore.QRect(550, 270, 200, 200))
        self.delete_user_button.setText("删除用户")
        self.delete_user_button.clicked.connect(self.delete_user)

    # 功能函数 - 弹出消息框并切换页面
    def clear_database(self):
        reply = QMessageBox.question(None, '确认', '确定要清空数据库吗？', QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
        if reply == QMessageBox.Yes:
            QMessageBox.information(None, '提示', '数据库已清空！', QMessageBox.Ok)
            print("数据库已清空！")
            # 这里可以加入清空数据库的逻辑

    def register_user(self):
        # 切换到数据库注册页面并弹出提示框
        self.stackedWidget.setCurrentWidget(self.db_page)
        QMessageBox.information(None, '提示', '未连接设备！', QMessageBox.Ok)
        # 这里可以加入注册用户的逻辑

    def delete_user(self):
        # 切换到数据库注册页面并弹出提示框
        self.stackedWidget.setCurrentWidget(self.db_page)
        QMessageBox.information(None, '提示', '数据库为空！', QMessageBox.Ok)
        # 这里可以加入删除用户的逻辑

    def update_time(self):
        # 更新当前时间
        current_time = QDateTime.currentDateTime().toString('hh:mm')  # 只显示时间
        self.lcdNumber.display(current_time)

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
