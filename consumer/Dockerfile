FROM python:3.9
ADD . /tmp
WORKDIR /app

COPY . .
RUN pip3 install -r requirement.txt

CMD python -u consumer.py 
