FROM python:3.14

WORKDIR /app

COPY requirements.txt /app

RUN --mount=type=cache,target=/root/.cache/pip \
    --mount=type=bind,source=requirements.txt,target=requirements.txt \
    pip install -r requirements.txt

COPY app.py /app

ARG PORT=8000
ENV PORT=$PORT
EXPOSE $PORT

CMD gunicorn -w 4 -b 0.0.0.0:$PORT app:app
