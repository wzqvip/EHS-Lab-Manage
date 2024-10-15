import sqlite3
import serial
import serial.tools.list_ports
from PyQt5 import QtCore, QtWidgets

class FingerprintApp(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.initDatabase()
        self.serial_connection = None

    def initUI(self):
        # 串口选择框
        self.port_combo_box = QtWidgets.QComboBox(self)
        self.refresh_ports()  # 刷新可用的串口列表
        self.port_combo_box.move(20, 20)

        # 连接按钮
        self.connect_button = QtWidgets.QPushButton('连接串口', self)
        self.connect_button.move(200, 20)
        self.connect_button.clicked.connect(self.connect_serial)

        # 指纹功能按钮
        self.enroll_button = QtWidgets.QPushButton('注册指纹', self)
        self.enroll_button.move(20, 70)
        self.enroll_button.clicked.connect(self.enroll_fingerprint)

        self.delete_button = QtWidgets.QPushButton('删除指纹', self)
        self.delete_button.move(20, 120)
        self.delete_button.clicked.connect(self.delete_fingerprint)

        self.detect_button = QtWidgets.QPushButton('检测指纹', self)
        self.detect_button.move(20, 170)
        self.detect_button.clicked.connect(self.detect_fingerprint)

        # 设置窗口
        self.setWindowTitle("Fingerprint Database Management")
        self.resize(400, 300)

    def initDatabase(self):
        # 初始化SQLite数据库
        self.conn = sqlite3.connect('fingerprint_db.sqlite')
        self.cursor = self.conn.cursor()
        self.cursor.execute('''CREATE TABLE IF NOT EXISTS fingerprints (id INTEGER PRIMARY KEY, name TEXT)''')
        self.conn.commit()

    def refresh_ports(self):
        # 列出可用串口
        self.port_combo_box.clear()
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.port_combo_box.addItem(port.device)

    def connect_serial(self):
        # 连接选定的串口
        selected_port = self.port_combo_box.currentText()
        if selected_port:
            try:
                self.serial_connection = serial.Serial(selected_port, 9600, timeout=1)
                QtWidgets.QMessageBox.information(self, '串口', f'成功连接到 {selected_port}')
            except Exception as e:
                QtWidgets.QMessageBox.critical(self, '错误', f'无法连接到 {selected_port}\n{str(e)}')
        else:
            QtWidgets.QMessageBox.warning(self, '警告', '没有选择串口')

    def enroll_fingerprint(self):
        # 发送注册指纹命令
        if self.serial_connection:
            user_id = self.get_user_id()
            if user_id:
                name = self.get_user_name()
                command = f'fp_enroll {user_id}\n'
                self.send_serial_command(command)
                self.add_to_database(user_id, name)

    def delete_fingerprint(self):
        # 发送删除指纹命令
        if self.serial_connection:
            user_id = self.get_user_id()
            if user_id:
                command = f'fp_delete {user_id}\n'
                self.send_serial_command(command)
                self.remove_from_database(user_id)

    def detect_fingerprint(self):
        # 发送检测指纹命令
        if self.serial_connection:
            command = 'fp_detect\n'
            self.send_serial_command(command)
            response = self.serial_connection.readline().decode().strip()
            if response.startswith("ID:"):
                finger_id = int(response.split(":")[1])
                self.lookup_fingerprint(finger_id)

    def send_serial_command(self, command):
        if self.serial_connection:
            self.serial_connection.write(command.encode())
        else:
            QtWidgets.QMessageBox.warning(self, '错误', '串口未连接')

    def add_to_database(self, user_id, name):
        # 将指纹ID和用户名称存入数据库
        self.cursor.execute('INSERT INTO fingerprints (id, name) VALUES (?, ?)', (user_id, name))
        self.conn.commit()

    def remove_from_database(self, user_id):
        # 从数据库中删除指纹ID
        self.cursor.execute('DELETE FROM fingerprints WHERE id = ?', (user_id,))
        self.conn.commit()

    def lookup_fingerprint(self, finger_id):
        # 从数据库中查找指纹ID对应的用户
        self.cursor.execute('SELECT name FROM fingerprints WHERE id = ?', (finger_id,))
        result = self.cursor.fetchone()
        if result:
            QtWidgets.QMessageBox.information(self, "匹配成功", f"用户: {result[0]}")
        else:
            QtWidgets.QMessageBox.warning(self, "未找到", "指纹未在数据库中找到")

    def get_user_id(self):
        # 获取用户ID
        user_id, ok = QtWidgets.QInputDialog.getInt(self, '输入用户ID', '请输入用户ID:')
        if ok:
            return user_id
        return None

    def get_user_name(self):
        # 获取用户名称
        name, ok = QtWidgets.QInputDialog.getText(self, '输入用户名', '请输入用户名:')
        if ok:
            return name
        return None

    def closeEvent(self, event):
        # 关闭数据库和串口连接
        self.conn.close()
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.close()
        event.accept()

# 运行应用程序
if __name__ == '__main__':
    import sys
    app = QtWidgets.QApplication(sys.argv)
    window = FingerprintApp()
    window.show()
    sys.exit(app.exec_())
