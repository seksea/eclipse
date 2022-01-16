from flask import Flask, send_file

app = Flask(__name__)
app.config.update(TEMPLATES_AUTO_RELOAD=True)

@app.route("/downloads/libgamesdk.so")
def libgamesdk():
    return send_file("libgamesdk.so")

@app.route("/downloads/kubos")
def kubos():
    return send_file("kubos")

@app.route("/downloads/libeclipse.so")
def libeclipse():
    return send_file("libeclipse.so")

