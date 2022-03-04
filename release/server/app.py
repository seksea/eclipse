from flask import Flask, send_file
from waitress import serve
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

#download route for gdb
@app.route("/downloads/gdb")
def gdb():
    return send_file("gdb")

#download route for the loader
@app.route("/loader.sh")
def loader():
    return send_file("loader.sh")

#download route for eclipse
@app.route("/downloads/libeclipse.so")
def libeclipse():
    os.system("strip -s -v libeclipse.so")
    os.system("patchelf --set-soname libMangoHud.so libeclipse.so")
    return send_file("libeclipse.so")

serve(app, host="0.0.0.0", port=80)