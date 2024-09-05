import sys
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QHBoxLayout, QLabel, QVBoxLayout
from PyQt5.QtGui import QFont, QPalette, QBrush, QPixmap
from PyQt5.QtCore import Qt, QTimer, QDateTime

class MainWindow(QWidget):
    def __init__(self):
        super().__init__()

        # 设置窗口标题和大小
        self.setWindowTitle('实验室仓库智能管理系统')
        self.setGeometry(100, 100, 1366, 768)

        # 设置背景图片
        self.set_background_image()

        # 创建垂直布局，包含顶部信息和按钮区域
        layout = QVBoxLayout()

        # 顶部信息区域,设置在最顶部

        self.info_label = QLabel('实验室仓库智能管理系统   温度: 26.8°C   湿度: 74.8%   时间: ')
        self.info_label.setFont(QFont('Arial', 16))
        self.info_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.info_label)

        # 定时更新时间
        timer = QTimer(self)
        timer.timeout.connect(self.update_time)
        timer.start(1000)

        # 创建水平布局用于放置三个按钮
        button_layout = QHBoxLayout()

        # 添加三个按钮（账号密码、指纹识别、人脸识别）
        button1 = QPushButton('账号密码')
        button1.setFont(QFont('Arial', 24))
        button1.setFixedSize(300, 200)  # 设置按钮大小
        button_layout.addWidget(button1)
        
        button2 = QPushButton('指纹识别')
        button2.setFont(QFont('Arial', 24))
        button2.setFixedSize(300, 200)  # 设置按钮大小
        button_layout.addWidget(button2)

        button3 = QPushButton('人脸识别')
        button3.setFont(QFont('Arial', 24))
        button3.setFixedSize(300, 200)  # 设置按钮大小
        button_layout.addWidget(button3)

        # 将水平布局添加到主布局
        layout.addLayout(button_layout)

        # 设置布局
        self.setLayout(layout)

    def set_background_image(self):
        # 设置窗口背景图片，拉伸填满
        palette = QPalette()
        pixmap = QPixmap("background_image.png")  # 替换成你的背景图片路径
        palette.setBrush(self.backgroundRole(), QBrush(pixmap.scaled(self.size(), Qt.IgnoreAspectRatio, Qt.SmoothTransformation)))
        self.setPalette(palette)


    def update_time(self):
        # 获取当前时间，并更新到顶部信息区域
        current_time = QDateTime.currentDateTime().toString('yyyy-MM-dd hh:mm:ss')
        self.info_label.setText(f'实验室仓库智能管理系统   温度: 26.8°C   湿度: 74.8%   时间: {current_time}')


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
