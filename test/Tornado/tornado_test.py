import tornado.ioloop
import tornado.web

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.write("Hello World")

class Application(tornado.web.Application):
    def __init__(self):
        handlers = [
                (r'/', MainHandler),
                ]
        tornado.web.Application.__init__(self, handlers)

if __name__ == "__main__":
    app = Application()
    app.listen(12309)
    print("Tornado Started in port 12309, http://localhost:12309")
    tornado.ioloop.IOLoop.current().start()
