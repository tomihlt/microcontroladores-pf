
from flask import Flask, request, jsonify

app = Flask(__name__)

# Mock de usuarios
usuarios = [
	{"dni": "12345678", "nombre": "Juan", "apellido": "Pérez"},
	{"dni": "87654321", "nombre": "Ana", "apellido": "García"},
	{"dni": "11223344", "nombre": "Luis", "apellido": "Martínez"}
]

@app.route('/usuario', methods=['POST'])
def verificar_usuario():
	data = request.get_json()
	if not data or "dni" not in data:
		return jsonify({"error": "DNI no proporcionado"}), 400

	dni = data["dni"]
	usuario = next((u for u in usuarios if u["dni"] == dni), None)
	if usuario:
		return jsonify({"mensaje": "Usuario encontrado", "usuario": usuario}), 200
	else:
		return jsonify({"error": "Usuario no encontrado"}), 404

if __name__ == '__main__':
	app.run(host='0.0.0.0', port = 5000, debug=True)
