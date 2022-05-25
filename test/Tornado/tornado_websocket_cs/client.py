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
            data = {'identity': 3,
                    'controlNotice': 10}

            yield self.ws.write_message(json.dumps(data))
            yield gen.sleep(1)

    def on_message(self, msg):
        print('message: {}'.format(msg))

    def keep_alive(self):
        if self.ws is None:
            self.connect()
        else:
            self.ws.write_message("keep alive")

if __name__ == '__main__':
    client = Client("ws://localhost:12340/periodicDataUpload?who=user", 5)
