import os
import threading
import subprocess
import queue
import time
from flask import Flask, request, jsonify, send_from_directory


APP_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.abspath(os.path.join(APP_DIR, os.pardir))


class CLIProcess:
	def __init__(self, exe_path: str):
		self.exe_path = exe_path
		self.proc = None
		self.stdout_queue = queue.Queue()
		self.stdout_thread = None
		self.lock = threading.Lock()
		self.prompt = "> "

	def start(self):
		if self.proc is not None and self.proc.poll() is None:
			return
		self.proc = subprocess.Popen(
			[self.exe_path],
			stdin=subprocess.PIPE,
			stdout=subprocess.PIPE,
			stderr=subprocess.STDOUT,
			text=True,
			bufsize=1,
			cwd=ROOT_DIR,
		)
		self.stdout_thread = threading.Thread(target=self._pump_stdout, daemon=True)
		self.stdout_thread.start()

	def _pump_stdout(self):
		# Read line by line; prompt may lack newline; also read in small chunks
		buffer = ""
		while True:
			if self.proc is None:
				break
				
			ch = self.proc.stdout.read(1)
			if not ch:
				break
			buffer += ch
			# Emit on newline or when prompt marker appears
			if ch == "\n" or buffer.endswith(self.prompt):
				self.stdout_queue.put(buffer)
				buffer = ""

	def send_command(self, line: str, timeout_sec: float = 2.5) -> str:
		with self.lock:
			if self.proc is None or self.proc.poll() is not None:
				raise RuntimeError("CLI process is not running. Build and start the binary first.")
			# Drain existing output to return only fresh response
			try:
				while True:
					self.stdout_queue.get_nowait()
			except queue.Empty:
				pass
			# Write command
			assert self.proc.stdin is not None
			self.proc.stdin.write(line + "\n")
			self.proc.stdin.flush()
			# Collect until we see prompt chunk or timeout
			deadline = time.time() + timeout_sec
			collected = []
			while time.time() < deadline:
				try:
					chunk = self.stdout_queue.get(timeout=0.05)
					collected.append(chunk)
					if chunk.endswith(self.prompt):
						break
				except queue.Empty:
					continue
			return "".join(collected)


def find_binary() -> str:
	# Prefer prebuilt names
	candidates = [
		os.path.join(ROOT_DIR, "shm_cli"),
		os.path.join(ROOT_DIR, "shm_cli.exe"),
		os.path.join(ROOT_DIR, "a.out"),
	]
	for c in candidates:
		if os.path.isfile(c):
			return c
	# Fallback to building with g++ if available
	gxx = os.environ.get("CXX", "g++")
	cmd = [
		gxx,
		"-std=c++17",
		"-pthread",
		os.path.join(ROOT_DIR, "main.cpp"),
		os.path.join(ROOT_DIR, "SharedMemory.cpp"),
		os.path.join(ROOT_DIR, "FirstFitAllocator.cpp"),
		"-o",
		os.path.join(ROOT_DIR, "shm_cli"),
	]
	try:
		subprocess.check_call(cmd)
		bin_path = os.path.join(ROOT_DIR, "shm_cli")
		if os.name == "nt":
			bin_path += ".exe"
		return bin_path
	except Exception as exc:
		raise RuntimeError(f"Failed to locate or build CLI binary: {exc}")


app = Flask(__name__, static_folder=os.path.join(APP_DIR, "static"))
cli = None


@app.route("/")
def index():
	return send_from_directory(app.static_folder, "index.html")


@app.route("/static/<path:path>")
def static_files(path):
	return send_from_directory(app.static_folder, path)


@app.route("/api/start", methods=["POST"])
def api_start():
	global cli
	if cli is None:
		bin_path = find_binary()
		cli = CLIProcess(bin_path)
		cli.start()
		# Read initial banner
		time.sleep(0.2)
		return jsonify({"status": "started", "binary": bin_path})
	return jsonify({"status": "already_running"})


@app.route("/api/cmd", methods=["POST"])
def api_cmd():
	global cli
	if cli is None:
		return jsonify({"error": "CLI not started"}), 400
	body = request.get_json(force=True)
	cmd = body.get("cmd", "").strip()
	if not cmd:
		return jsonify({"error": "cmd required"}), 400
	try:
		output = cli.send_command(cmd)
		return jsonify({"ok": True, "output": output})
	except Exception as exc:
		return jsonify({"ok": False, "error": str(exc)}), 500


def create_app():
	return app


if __name__ == "__main__":
	port = int(os.environ.get("PORT", "5000"))
	app.run(host="0.0.0.0", port=port, debug=True)



