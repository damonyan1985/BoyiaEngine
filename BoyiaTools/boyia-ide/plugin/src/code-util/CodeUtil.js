const path = require('path');
const fs = require('fs');
const vscode = require('vscode');

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
  }
}

module.exports = CodeUtil;