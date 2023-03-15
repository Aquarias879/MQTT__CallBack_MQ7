FROM python:3.10
ADD . /tmp
WORKDIR /app

COPY  . .
RUN pip install -r requirement.txt

CMD python3 publisher.py
