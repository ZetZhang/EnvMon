from tornado.ioloop import IOLoop, PeriodicCallback
from tornado import gen
from tornado.websocket import websocket_connect

import json

class Client(object):
    def __init__(self, url, timeout):
        self.count = 0
        self.url = url
        self.timeout = timeout
        self.ioloop = IOLoop.instance()
        self.ws = None
        self.connect()
        PeriodicCallback(self.keep_alive, 20000).start()
        self.ioloop.start()

    @gen.coroutine
    def connect(self):
        print("trying to connect")
        try:
            self.ws = yield websocket_connect(self.url, on_message_callback=self.on_message)
        except Exception as e:
            print("connection error")
        else:
            print("conncted")
            self.run()

    @gen.coroutine
    def run(self):
        while True:
            #  data = {'identity': 3, 'control': 1}
            data = {'identity': 5}

#  b''.join(map(lambda x:int.to_bytes(x,1,'little'),s))
            #  value = [70, 20, 109, 11, 110, 10, 67, 42, 70, 40, 28, 22, 30, 20]
            #  value = [98, 39, 70, 92, 120, 49, 52, 109, 92, 120, 48, 98, 110, 92]
            #  value = b''.join(map(lambda x:int.to_bytes(x, 1, 'little'), vlist))

            #  value = [20, 30, 22, 28, 40, 70, 42, 67, 10, 110, 11, 109, 70, 68]
            #  data = {'identity': 4,
            #          'threshold': value}
            yield self.ws.write_message(json.dumps(data))
            yield gen.sleep(100)

            #  go = [3, 2, 7, 3, 0]
            #  for i in go:
            #      data = {'identity': 3,
            #              'control': i}

            #      yield self.ws.write_message(json.dumps(data))
            #      yield gen.sleep(4)

    def on_message(self, msg):
        print('message: {}'.format(msg))

    def keep_alive(self):
        if self.ws is None:
            self.connect()
        else:
            self.ws.write_message("keep alive")

if __name__ == '__main__':
    client = Client("ws://192.168.199.102:12340/periodicDataUpload?who=users", 5)
