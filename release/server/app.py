from flask import Flask, send_file
import os

app = Flask(__name__)
app.config.update(TEMPLATES_AUTO_RELOAD=True)

# download route for gamesdk
@app.route("/downloads/libgamesdk.so")
def libgamesdk():
    return send_file("libgamesdk.so")

#download route for kubos injector
@app.route("/downloads/kubos")
def kubos():
    return send_file("kubos")

#download route for eclipse (requires vmprotecting with custom watermark based on IP)
@app.route("/downloads/libeclipse.so")
def libeclipse():
    try:
        os.remove("libeclipse.so")
    except:
        print("file already doesn't exist")
    os.system("wine VMProtect/VMProtect_Con.exe libeclipse_unprotected.so")
    return send_file("libeclipse.so")
