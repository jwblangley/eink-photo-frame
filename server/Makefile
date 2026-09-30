.PHONY: format dev server

VENV = venv
BIN = $(VENV)/bin

VENV_DEV = venv-dev
BIN_DEV = $(VENV_DEV)/bin

PORT = 8000

$(VENV): requirements.txt
	python3.14 -m venv $(VENV)
	$(BIN)/pip install -r requirements.txt
	touch $(VENV)

$(VENV_DEV): requirements.txt requirements-dev.txt
	python3.14 -m venv $(VENV_DEV)
	$(BIN_DEV)/pip install -r requirements-dev.txt
	touch $(VENV_DEV)

server: $(VENV)
	$(BIN)/gunicorn -w 4 -b 0.0.0.0:$(PORT) app:app

dev: $(VENV_DEV)
	$(BIN_DEV)/python -m flask run --port $(PORT)

format: $(VENV_DEV)
	$(BIN_DEV)/ruff format
