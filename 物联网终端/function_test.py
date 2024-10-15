import sys
import serial
import serial.tools.list_ports
from PyQt5 import QtWidgets, QtCore

class FingerprintWeightApp(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.serial_connection = None

    def initUI(self):
        # 串口选择框
        self.port_combo_box = QtWidgets.QComboBox(self)
        self.port_combo_box.setGeometry(20, 20, 200, 30)
        self.refresh_ports()

        # 连接按钮
        self.connect_button = QtWidgets.QPushButton('连接串口', self)
        self.connect_button.setGeometry(240, 20, 100, 30)
        self.connect_button.clicked.connect(self.connect_serial)

        # 功能按钮
        self.read_weight_button = QtWidgets.QPushButton('读取重量', self)
        self.read_weight_button.setGeometry(20, 70, 120, 30)
        self.read_weight_button.clicked.connect(lambda: self.send_command("read_weight"))

        self.enroll_button = QtWidgets.QPushButton('注册指纹', self)
        self.enroll_button.setGeometry(160, 70, 120, 30)
        self.enroll_button.clicked.connect(self.enroll_fingerprint)

        self.delete_button = QtWidgets.QPushButton('删除指纹', self)
        self.delete_button.setGeometry(20, 120, 120, 30)
        self.delete_button.clicked.connect(self.delete_fingerprint)

        self.detect_button = QtWidgets.QPushButton('检测指纹', self)
        self.detect_button.setGeometry(160, 120, 120, 30)
        self.detect_button.clicked.connect(lambda: self.send_command("fp_detect"))

        # 文本区域用于显示结果
        self.result_display = QtWidgets.QTextEdit(self)
        self.result_display.setGeometry(20, 170, 320, 100)

        # 设置窗口
        self.setWindowTitle("Fingerprint & Weight System")
        self.setGeometry(100, 100, 360, 300)

    def refresh_ports(self):
        self.port_combo_box.clear()
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.port_combo_box.addItem(port.device)

    def connect_serial(self):
        selected_port = self.port_combo_box.currentText()
        if selected_port:
            try:
                self.serial_connection = serial.Serial(selected_port, 9600, timeout=1)
                self.result_display.append(f'成功连接到 {selected_port}')
            except Exception as e:
                self.result_display.append(f'无法连接到 {selected_port}: {e}')
        else:
            self.result_display.append('请选择串口')

    def send_command(self, command):
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.write((command + "\n").encode())
            QtCore.QTimer.singleShot(500, self.read_response)  # 延时读取串口响应
        else:
            self.result_display.append('串口未连接')

    def read_response(self):
        if self.serial_connection and self.serial_connection.in_waiting:
            response = self.serial_connection.readline().decode().strip()
            self.result_display.append(f'设备响应: {response}')

    def enroll_fingerprint(self):
        user_id, ok = QtWidgets.QInputDialog.getInt(self, '输入ID', '请输入要注册的指纹ID:')
        if ok:
            self.send_command(f'fp_enroll {user_id}')

    def delete_fingerprint(self):
        user_id, ok = QtWidgets.QInputDialog.getInt(self, '输入ID', '请输入要删除的指纹ID:')
        if ok:
            self.send_command(f'fp_delete {user_id}')

    def closeEvent(self, event):
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.close()
        event.accept()

# 运行应用
if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    window = FingerprintWeightApp()
    window.show()
    sys.exit(app.exec_())
