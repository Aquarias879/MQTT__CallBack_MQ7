FROM python:3.10
ADD . /tmp
WORKDIR /app

COPY . .
RUN pip3 install -r requirement.txt

CMD python3 ls_data.py
