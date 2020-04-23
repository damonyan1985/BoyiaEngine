const CodeLogView = require('./CodeLogView');
const StringBuilder = require('../code-util/StringBuilder');

const CodeLogImpl = {
  init() {
    CodeLogImpl.logView = new CodeLogView();

    console.log = function (log) {
      CodeLogImpl.logView.logPrint(CodeLogImpl.formatLog(log));
    }
  },

  formatLog(log) {
    //const time = new Date().getTime();
    const builder = new StringBuilder();
    builder.append('[')
      .append(new Date().toLocaleString().replace(/:\d{1,2}$/, ' '))
      .append('] [log] [boyia] ')
      .append(log);
    return builder.toString();
  }
}

module.exports = CodeLogImpl;