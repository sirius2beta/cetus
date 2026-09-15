import os
import glob 
import json
import re
import sqlite3
import threading  # 💡 引入執行緒模組
from flask import Flask, jsonify, send_file, send_from_directory, abort, request
from flask_cors import CORS 

# 💡 引入 ROS 2 核心庫
import rclpy
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, QoSProfile, ReliabilityPolicy
from std_msgs.msg import String
import logging


# ==========================================
# 1. 保持你原本的 Flask 設定與路徑
# ==========================================
app = Flask(__name__)
CORS(app)

BASE_DIR = "/home/sirius2beta/GPlayerLogNew/snapshot/seagrass"
LOG_DIR = "/home/sirius2beta/GPlayerLogNew"
REPLAY_ROW_LOCK = threading.Lock()


@app.after_request
def print_json_response(response):
    """Print every JSON response so it can be inspected in the server log."""
    if response.mimetype == 'application/json':
        print('HTTP {} {}'.format(
            response.status_code,
            request.path))
    return response

def safe_path_join(base, *paths):
    target_path = os.path.abspath(os.path.join(base, *paths))
    if not target_path.startswith(os.path.abspath(base)):
        raise ValueError("危險的路徑存取！")
    return target_path

def get_latest_db_path():
    replay_log = app.config.get("REPLAY_LOG")
    if replay_log:
        return replay_log

    if not os.path.exists(LOG_DIR): return None
    db_files = [f for f in os.listdir(LOG_DIR) if f.startswith("log_") and f.endswith(".db")]
    if not db_files: return None
    indices = []
    for f in db_files:
        match = re.search(r"log_(\d+)\.db", f)
        if match: indices.append((int(match.group(1)), f))
    if not indices: return None
    latest_file = max(indices, key=lambda x: x[0])[1]
    return os.path.join(LOG_DIR, latest_file)

def get_newest_file(paths):
    """Return the newest regular file from an iterable, or None."""
    files = [path for path in paths if os.path.isfile(path)]
    return max(files, key=os.path.getmtime_ns) if files else None

def send_current_image(path, simulation_fallback=False):
    if not path:
        return jsonify({"error": "No current image available"}), 404
    response = send_file(path, mimetype='image/jpeg', conditional=False)
    # Query parameter t is deliberately ignored.  These headers also prevent
    # browsers and proxies from reusing a prior frame.
    response.headers['Cache-Control'] = 'no-store, no-cache, must-revalidate, max-age=0'
    response.headers['Pragma'] = 'no-cache'
    response.headers['Expires'] = '0'
    if simulation_fallback:
        response.headers['X-Image-Source'] = 'simulation-seagrass-log'
    return response

def get_simulation_image(mask):
    """Resolve the image named by the current replay database row."""
    with REPLAY_ROW_LOCK:
        image_name = app.config.get('REPLAY_SEAGRASS_IMAGE_NAME')
        if not image_name:
            row = app.config.get('REPLAY_ROW')
            image_name = row.get('seagrass_image_name') if isinstance(row, dict) else None
    if not image_name or not isinstance(image_name, str):
        return None
    image_name = image_name.replace('\\', '/')
    directory, filename = os.path.split(image_name)
    if mask and not filename.endswith('_mask.jpg'):
        stem, extension = os.path.splitext(filename)
        filename = '{}_mask{}'.format(stem, extension)
    try:
        return safe_path_join(BASE_DIR, directory, filename)
    except ValueError:
        return None

def get_live_image(mask):
    images = glob.glob(os.path.join(BASE_DIR, '**', '*.jpg'), recursive=True)
    if mask:
        images = [path for path in images if os.path.basename(path).endswith('_mask.jpg')]
    else:
        images = [path for path in images if not os.path.basename(path).endswith('_mask.jpg')]
    return get_newest_file(images)

# ==========================================
# 2. 你原本的所有 Flask Routes (完全保留)
# ==========================================
@app.route('/api/log/latest', methods=['GET'])
def get_latest_log():
    try:
        if app.config.get('SIMULATION_MODE'):
            with REPLAY_ROW_LOCK:
                row = app.config.get('REPLAY_ROW')
            if row is None:
                return jsonify({
                    "message": "等待 log_manager 的第一筆回放資料",
                    "file": os.path.basename(app.config['REPLAY_LOG']),
                }), 503
            return jsonify({
                "status": "success",
                "database_file": os.path.basename(app.config['REPLAY_LOG']),
                "data": row,
            })

        db_path = get_latest_db_path()
        if not db_path: return jsonify({"error": "找不到任何 Log 資料庫檔案"}), 404
        conn = sqlite3.connect(db_path)
        conn.row_factory = sqlite3.Row
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM logs ORDER BY id DESC LIMIT 1")
        row = cursor.fetchone()
        conn.close()
        if row is None: return jsonify({"message": "資料庫目前尚無資料", "file": os.path.basename(db_path)}), 200
        return jsonify({"status": "success", "database_file": os.path.basename(db_path), "data": dict(row)})
    
    except Exception as e:
        return jsonify({"error": f"讀取資料庫失敗: {str(e)}"}), 500

@app.route('/api/structure', methods=['GET'])
def get_structure():
    try:
        if not os.path.exists(BASE_DIR): return jsonify({"error": "Base directory not found"}), 404
        records = []
        with os.scandir(BASE_DIR) as entries:
            for entry in entries:
                if entry.is_dir() and entry.name.startswith("seagrass_"):
                    records.append(entry.name)
        return jsonify({"records": sorted(records)})
    except Exception as e: return jsonify({"error": str(e)}), 500

@app.route('/api/structure/<record_dir>', methods=['GET'])
def get_images_in_record(record_dir):
    try:
        target_dir = safe_path_join(BASE_DIR, record_dir)
        if not os.path.exists(target_dir): return jsonify({"error": f"Directory {record_dir} not found"}), 404
        all_jpgs = glob.glob(os.path.join(target_dir, "*.jpg"))
        if not all_jpgs: return jsonify({"record": record_dir, "total_count": 0, "images": []})
        orig_jpgs = [f for f in all_jpgs if "_mask" not in f]
        if not orig_jpgs: return jsonify({"record": record_dir, "total_count": 0, "images": []})
        last_file_name = os.path.basename(max(orig_jpgs))
        try: max_index = int(last_file_name.split('.')[0])
        except ValueError: max_index = len(orig_jpgs)
        images = [f"{str(i).zfill(7)}.jpg" for i in range(1, max_index + 1)]
        return jsonify({"record": record_dir, "total_count": max_index, "images": images})
    except ValueError: abort(403)
    except Exception as e: return jsonify({"error": str(e)}), 500

@app.route('/api/image/<record_dir>/<filename>', methods=['GET'])
def get_image(record_dir, filename):
    try:
        target_dir = safe_path_join(BASE_DIR, record_dir)
        return send_from_directory(target_dir, filename)
    except ValueError: abort(403)
    except FileNotFoundError: abort(404)

@app.route('/api/image/latest', methods=['GET'])
def get_latest_image():
    # request.args.get('t') is intentionally ignored; callers use it only as
    # a cache-busting value.
    if app.config.get('SIMULATION_MODE'):
        return send_current_image(get_simulation_image(mask=False))
    return send_current_image(get_live_image(mask=False))

@app.route('/api/image/latestmask', methods=['GET'])
def get_latest_mask():
    # AI is intentionally disabled in simulation mode.  Use the mask from the
    # replay's corresponding seagrass frame instead of an MP4-derived image.
    if app.config.get('SIMULATION_MODE'):
        return send_current_image(get_simulation_image(mask=True), simulation_fallback=True)
    return send_current_image(get_live_image(mask=True))


# ==========================================
# 3. 🆕 定義 ROS 2 Node 與啟動邏輯
# ==========================================
class LogServerNode(Node):
    def __init__(self):
        super().__init__('log_server_node')
        simulation_mode = self.declare_parameter('simulation_mode', False).value
        replay_log = self.declare_parameter('replay_log', '').value

        if simulation_mode:
            if not replay_log:
                raise ValueError('simulation_mode requires the replay_log parameter')
            replay_log = os.path.abspath(os.path.expanduser(replay_log))
            if not os.path.isfile(replay_log):
                raise FileNotFoundError(
                    'Replay log does not exist: {}'.format(replay_log))
            app.config['REPLAY_LOG'] = replay_log
            app.config['SIMULATION_MODE'] = True
            app.config['REPLAY_ROW'] = None
            app.config['REPLAY_SEAGRASS_IMAGE_NAME'] = None
            replay_qos = QoSProfile(
                depth=1,
                reliability=ReliabilityPolicy.RELIABLE,
                durability=DurabilityPolicy.TRANSIENT_LOCAL)
            self.replay_row_subscriber = self.create_subscription(
                String, '/simulation/current_log_row',
                self.replay_row_callback, replay_qos)
            self.get_logger().info(
                'Simulation mode: HTTP server is reading {}.'
                .format(replay_log))

        self.get_logger().info("海草 Log 服務節點已啟動！")
        
        # 💡 未來擴充：你可以在這裡訂閱 ROS 2 Topic (例如 GPS 或相機點位)
        # self.subscription = self.create_subscription(Pose, 'robot_pose', self.pose_callback, 10)

        # 💡 在背景 Thread 啟動 Flask 伺服器，避免卡死 ROS 2
        self.flask_thread = threading.Thread(target=self.run_flask)
        self.flask_thread.daemon = True  # 當主程式關閉時，此 Thread 會自動結束
        self.flask_thread.start()

    def replay_row_callback(self, msg):
        try:
            row = json.loads(msg.data)
            if not isinstance(row, dict):
                raise ValueError('replay row must be a JSON object')
            with REPLAY_ROW_LOCK:
                app.config['REPLAY_ROW'] = row
                image_name = row.get('seagrass_image_name')
                if isinstance(image_name, str) and image_name.strip():
                    app.config['REPLAY_SEAGRASS_IMAGE_NAME'] = image_name
        except (TypeError, ValueError, json.JSONDecodeError) as error:
            self.get_logger().error(
                'Could not decode simulated replay row: {}'.format(error))

    def run_flask(self):
        self.get_logger().info("正在背景啟動 Flask Web 伺服器 (Port: 5000)...")
        # 這裡執行原本的 Flask app.run
        app.run(host='0.0.0.0', port=5000, debug=False, use_reloader=False)


def main(args=None):
    rclpy.init(args=args)
    node = LogServerNode()
    try:
        rclpy.spin(node)  # 讓 ROS 2 節點持續監聽與運行
    except KeyboardInterrupt:
        node.get_logger().info("正在關閉節點...")
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
