const WebSocket = require('ws');
const CodeUtil = require('../code-util/CodeUtil');
const { 
  LoggingDebugSession,
  StoppedEvent,
  BreakpointEvent,
  Breakpoint,
  InitializedEvent
} = require('@vscode/debugadapter')
const { EventEmitter } = require('events')

const kServerPort = 6666;
const kThreadID = 1;

// 接受远程调试
class BoyiaDebugServer {
    constructor() {
        this.server = new WebSocket.Server(
          {
            host: CodeUtil.getLocalIP(),
            port: kServerPort
          }
        );

        this.server.on('listening', socket => {
          console.log('Test WebSocket listening address=' + JSON.stringify(this.server.address()));
        });
      
        // 接受调试客户端连接
        this.server.on('connection', (socket, req) => {
          console.log('Test WebSocket connection url=' + req.url);
          socket.send('hello world');
      
          // 接受客户端发来的消息
          socket.on('message', event => {
            console.log('Test WebSocket message data=' + event.toString());
          });
        });  
    }

    getPort() {
      return kServerPort;
    }

    // 关闭服务
    close() {
      if (this.server) {
        this.server.close();
      }
    }

}

// BoyiaDebugSession与DebugAdapterServer进行通信，控制UI断点
class BoyiaDebugSession extends LoggingDebugSession {
  constructor() {
    super();
    this.runtime = new BoyiaRuntime();
  }

  initDebugEvents() {
    this.runtime.on('stopOnStep', () => {
			this.sendEvent(new StoppedEvent('step', kThreadID));
		});
  }

  /**
   * 重写LoggingDebugSession回调
   * DebugSession启动时，会执行到这个回调
   * @override
   * @param {*} response 
   * @param {*} args 
   */
  initializeRequest(response/*: DebugProtocol.InitializeResponse*/, args/*: DebugProtocol.InitializeRequestArguments*/) {
    this.sendResponse(response);

		// since this debug adapter can accept configuration requests like 'setBreakpoint' at any time,
		// we request them early by sending an 'initializeRequest' to the frontend.
		// The frontend will end the configuration sequence by calling 'configurationDone' request.
		this.sendEvent(new InitializedEvent());
  }

  /**
   * 重写LoggingDebugSession回调
   * 点击UI单步调试，会执行到这个回调
   * @override
   * @param {*} response 
   * @param {*} args 
   */
  nextRequest(response/*: DebugProtocol.NextResponse*/, args/*: DebugProtocol.NextArguments*/) {
		this.runtime.step();
		this.sendResponse(response);
	}

  /**
   * 点击VSCode UI上的Continue按钮，会执行到这个回调
   * @override
   * @param {*} response 
   * @param {*} args 
   */
  continueRequest(response/*: DebugProtocol.ContinueResponse*/, args/*: DebugProtocol.ContinueArguments*/) {
    // 通知UI
    this.sendResponse(response);
  }

  /**
   * 点击VSCodeUI上的断点，会调用到这个回调
   * @override
   * @param {*} response 
   * @param {*} args 
   */
  async setBreakPointsRequest(response/*: DebugProtocol.SetBreakpointsResponse*/, args/*: DebugProtocol.SetBreakpointsArguments*/) {
    const breakpointLines = args.lines || [];
    breakpointLines.forEach(breakpoint => {
      
    });
    this.sendResponse(response);
  }

  /**
   * VSCodeUI命中断点时，会请求调用堆栈
   * 这时需要去BoyiaRuntime中获取当前运行时堆栈信息
   * @override
   * @param {*} response 
   * @param {*} args 
   */
  stackTraceRequest(response/*: DebugProtocol.StackTraceResponse*/, args/*: DebugProtocol.StackTraceArguments*/) {
  }
}

// 启动Boyia虚拟机运行时抽象，与真实的Boyia虚拟机进行通信
class BoyiaRuntime extends EventEmitter {
  constructor() {
    super();
  }

  // 单步
  step() {
    this.sendEvent('stopOnStep');
  }

  // 发送事件
  sendEvent(event, args) {
    setTimeout(() => {
      this.emit(event, ...args);
    }, 0);
  }
}

module.exports = {
  BoyiaDebugServer,
  BoyiaDebugSession
};