import tornado.ioloop
import tornado.web
import tornado.websocket

def run_test():
    print('connection...')
    conn = tornado.websocket.websocket_connect("ws://localhost:12306/ws_demo")
    while True:
        msg = conn.read_message()
        if msg is None: break
        print(msg)

if __name__ == '__main__':
    run_test()
    #  run_test_new()
