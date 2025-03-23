const path = require('path');
const fs = require('fs');
const vscode = require('vscode');
const os = require('os');

const CodeUtil = {
  getAbsolutePath(context, relativePath) {
    return path.join(context.extensionPath, relativePath);
  },

  getWebViewContents(context, htmlPath) {
    try {
      const webViewPath = CodeUtil.getAbsolutePath(context, htmlPath);
      const dirPath = path.dirname(webViewPath);

      let htmlContent = fs.readFileSync(webViewPath, 'UTF-8');
      htmlContent = htmlContent.replace(/(<link.+?href="|<script.+?src="|<img.+?src=")(.+?)"/g, (text, s1, s2) => {
        return s1 + vscode.Uri.file(path.join(dirPath, s2)).with({ scheme: 'vscode-resource' }).toString() + '"';
      });

      return htmlContent;
    } catch (e) {
      console.log('CodeUtil::getWebViewContents---err: ' + e);
    }

    return null;
  },

  getLocalIP() {
    const interfaces = os.networkInterfaces();
    for (const devName of Object.keys(interfaces)) {
      if (devName.toLowerCase().indexOf('loopback') !== -1) {
        continue;
      }
      const iface = interfaces[devName];
      for (let i = 0; i < iface.length; i++) {
        const alias = iface[i];
        if (alias.family === 'IPv4' && alias.address !== '127.0.0.1' && !alias.internal) {
          return alias.address;
        }
      }
    }

    return '127.0.0.1';
  }
}

module.exports = CodeUtil;