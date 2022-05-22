import tornado.ioloop
import tornado.web
import tornado.websocket

class ProStatus():
    connector = {}

    def user_connect(self, user):
        if user not in self.connector:
            self.connector[user] = set()

    def user_remove(self, user):
        self.connector.remove(user)

    def trigger(self, message):
        for user in self.connector:
            user.write_message(message)

class ReceiveHandler(tornado.web.RequestHandler):
    def get(self):
        msg = self.get_argument('msg', '')
        ProStatus().trigger(msg)

class ConnectHandler(tornado.websocket.WebSocketHandler):
    def check_origin(self, origin):
        '''重写同源检查，解决跨域问题'''
        return True

    def open(self):
        print("clinet open!")
        self.write_message('welcome')

    def on_close(self):
        print("client closed!")

    def on_message(self, message):
        self.write_message('new message: ' + message)

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.write("Hello World")

class Application(tornado.web.Application):
    def __init__(self):
        handlers = [
                (r'/index', MainHandler),
                (r'/ws', ConnectHandler),
                (r'/receive', ReceiveHandler)
                ]
        tornado.web.Application.__init__(self, handlers)

if __name__ == '__main__':
    app = Application()
    app.listen(12309)
    tornado.ioloop.IOLoop.current().start()
