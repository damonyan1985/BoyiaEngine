const vscode = require('vscode');
const Net = require('net');
const { BoyiaDebugSession } = require('./BoyiaDebugServer');

const kDebugSessionPort = 8888;

class BoyiaDebugAdapterServerDescriptorFactory {
  /**
   * 
   * @param { vscode.DebugSession } session 
   * @param { vscode.DebugAdapterExecutable } executable 
   * @returns { vscode.ProviderResult<vscode.DebugAdapterDescriptor> }
   */
  createDebugAdapterDescriptor(session, executable) {
    if (!this.server) {
      // start listening on a random port
      this.server = Net.createServer(socket => {
        const session = new BoyiaDebugSession();
        session.setRunAsServer(true);
        session.start(socket, socket);
      }).listen(kDebugSessionPort);
    }
    // Create a VS Code connect to debug server
    return new vscode.DebugAdapterServer(kDebugSessionPort);
  }

  dispose() {
    if (this.server) {
      this.server.close();
    }
  }   
}