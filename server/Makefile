.PHONY: format dev server

PORT = 8000

venv: requirements.txt
	python3.14 -m venv venv
	. venv/bin/activate
	python3.14 -m pip install -r requirements.txt

server: venv
	gunicorn -w 4 -b 0.0.0.0:$(PORT) app:app

dev: venv
	python -m flask run

format: venv
	ruff format
