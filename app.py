#responsável por iniciar o servidor e registrar as rotas.
from flask import Flask
from api.rfid import rfid_bp

app = Flask(__name__)

# Registra as rotas do RFID
app.register_blueprint(rfid_bp)

@app.route("/")
def home():
    return "Servidor do Sistema de Presença funcionando!"

if __name__ == "__main__":
    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )