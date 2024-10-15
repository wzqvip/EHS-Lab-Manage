import sys, os
from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtCore import QTimer, QDateTime
from PyQt5.QtGui import QFont, QPalette, QBrush, QPixmap
from PyQt5.QtWidgets import QMessageBox

import serial
import serial.tools.list_ports

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
        self.setup_sub_page(self.fingerprint_page, "未连接设备！")
        self.stackedWidget.addWidget(self.fingerprint_page)

        # 账号密码页面
        self.password_page = QtWidgets.QWidget()
        self.setup_sub_page(self.password_page, "数据库为空！")
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

                 # 添加串口选择菜单
        self.serial_menu = QtWidgets.QMenu(self.menubar)
        self.serial_menu.setObjectName("serial_menu")
        self.serial_menu.setTitle("串口")
        self.menubar.addMenu(self.serial_menu)

        # 添加串口选择下拉菜单
        self.port_combo_box = QtWidgets.QComboBox()
        self.refresh_ports()
        port_action = QtWidgets.QWidgetAction(self.serial_menu)
        port_action.setDefaultWidget(self.port_combo_box)
        self.serial_menu.addAction(port_action)

        # 添加串口连接按钮
        self.connect_button = QtWidgets.QPushButton("连接串口")
        connect_action = QtWidgets.QWidgetAction(self.serial_menu)
        connect_action.setDefaultWidget(self.connect_button)
        self.serial_menu.addAction(connect_action)
        
        self.connect_button.clicked.connect(self.open_serial_connection)

        # 添加断开连接按钮
        self.disconnect_button = QtWidgets.QPushButton("断开连接")
        disconnect_action = QtWidgets.QWidgetAction(self.serial_menu)
        disconnect_action.setDefaultWidget(self.disconnect_button)
        self.serial_menu.addAction(disconnect_action)

        self.disconnect_button.clicked.connect(self.close_serial_connection)


        # 定时刷新串口列表
        self.refresh_timer = QTimer(mainWindow)
        self.refresh_timer.timeout.connect(self.refresh_ports)
        self.refresh_timer.start(5000)  # 每5秒刷新一次串口列表


        self.retranslateUi(mainWindow)
        QtCore.QMetaObject.connectSlotsByName(mainWindow)

        # 定时更新时间
        timer = QTimer(mainWindow)
        timer.timeout.connect(self.update_time)
        timer.start(1000)  # 每秒更新一次

        

    def set_background_image(self, mainWindow):
        # 获取当前运行文件的目录
        basedir = getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__)))
        pixmap_path = os.path.join(basedir, 'background_image.png')  # 使用相对路径

        # 加载图片并设置背景
        pixmap = QPixmap(pixmap_path)
        if not pixmap.isNull():  # 确保图片加载成功
            palette = QPalette()
            scaled_pixmap = pixmap.scaled(mainWindow.size(), QtCore.Qt.IgnoreAspectRatio, QtCore.Qt.SmoothTransformation)
            palette.setBrush(QPalette.Background, QBrush(scaled_pixmap))
            mainWindow.setPalette(palette)
        else:
            print("Failed to load background image!")
            
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
        self.clear_db_button.clicked.connect(self.clear_database)  # 将来定义函数

        self.register_user_button = QtWidgets.QPushButton(page)
        self.register_user_button.setGeometry(QtCore.QRect(300, 270, 200, 200))
        self.register_user_button.setText("注册用户")
        self.register_user_button.clicked.connect(self.register_user)  # 将来定义函数

        self.delete_user_button = QtWidgets.QPushButton(page)
        self.delete_user_button.setGeometry(QtCore.QRect(550, 270, 200, 200))
        self.delete_user_button.setText("删除用户")
        self.delete_user_button.clicked.connect(self.delete_user)  # 将来定义函数

    # 功能函数 - 弹出消息框
    def clear_database(self):
        reply = QMessageBox.question(None, '确认', '确定要清空数据库吗？', QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
        if reply == QMessageBox.Yes:
            QMessageBox.information(None, '提示', '数据库已清空！', QMessageBox.Ok)
            print("数据库已清空！")
            # 这里可以加入清空数据库的逻辑

    def register_user(self):
        # 这里弹出一个提示框，仅带确认按钮
        QMessageBox.information(None, '提示', '未连接设备！', QMessageBox.Ok)
        # 这里可以加入注册用户的逻辑

    def delete_user(self):
        # 这里弹出一个提示框，仅带确认按钮
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

    def refresh_ports(self):
            """刷新串口列表"""
            self.port_combo_box.clear()
            ports = serial.tools.list_ports.comports()
            for port in ports:
                self.port_combo_box.addItem(port.device)

    def open_serial_connection(self):
        """打开选择的串口"""
        selected_port = self.port_combo_box.currentText()
        if selected_port:
            try:
                self.serial_connection = serial.Serial(selected_port, 9600, timeout=1)
                QMessageBox.information(None, '成功', f'成功连接到 {selected_port}', QMessageBox.Ok)
                self.connect_button.setText("已连接")
                self.port_combo_box.setEnabled(False)  # 禁用串口选择下拉框
                self.refresh_timer.stop()  # 停止刷新串口列表
            except serial.SerialException as e:
                QMessageBox.critical(None, '错误', f'无法连接到 {selected_port}\n{str(e)}', QMessageBox.Ok)
        else:
            QMessageBox.warning(None, '警告', '没有选择串口', QMessageBox.Ok)

    def close_serial_connection(self):
        """断开串口连接"""
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.close()
            QMessageBox.information(None, '断开', '串口已断开', QMessageBox.Ok)
            self.connect_button.setText("连接串口")
            self.port_combo_box.setEnabled(True)  # 重新启用串口选择
            self.refresh_timer.start(5000)  # 恢复刷新串口列表
        else:
            QMessageBox.warning(None, '警告', '没有连接的串口', QMessageBox.Ok)


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    mainWindow = QtWidgets.QMainWindow()
    ui = Ui_mainWindow()
    ui.setupUi(mainWindow)
    mainWindow.show()
    sys.exit(app.exec_())
